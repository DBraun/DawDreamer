#include "PlaybackWarpProcessor.h"
#ifdef BUILD_DAWDREAMER_RUBBERBAND

#include <rubberband/src/common/FFT.cpp>
#include <rubberband/src/common/Log.cpp>
#include <rubberband/src/common/mathmisc.cpp>
#include <rubberband/src/common/Profiler.cpp>
#include <rubberband/src/common/Resampler.cpp>
#include <rubberband/src/common/StretchCalculator.cpp>
#include <rubberband/src/common/sysutils.cpp>
#include <rubberband/src/common/Thread.cpp>
#include <rubberband/src/faster/AudioCurveCalculator.cpp>
#include <rubberband/src/faster/CompoundAudioCurve.cpp>
#include <rubberband/src/faster/HighFrequencyAudioCurve.cpp>
#include <rubberband/src/faster/PercussiveAudioCurve.cpp>
#include <rubberband/src/faster/R2Stretcher.cpp>
#include <rubberband/src/faster/SilentAudioCurve.cpp>
#include <rubberband/src/faster/StretcherChannelData.cpp>
#include <rubberband/src/faster/StretcherProcess.cpp>
#include <rubberband/src/finer/R3Stretcher.cpp>
#include <rubberband/src/RubberBandStretcher.cpp>

PlaybackWarpProcessor::PlaybackWarpProcessor(std::string newUniqueName,
                                             std::vector<std::vector<float>> inputData, double sr,
                                             double data_sr)
    : ProcessorBase{newUniqueName}
{
    createParameterLayout();
    m_numChannels = (int)inputData.size();
    setMainBusInputsAndOutputs(0, m_numChannels);
    const int numSamples = (int)inputData.at(0).size();

    myPlaybackData.setSize(m_numChannels, numSamples);
    for (int chan = 0; chan < m_numChannels; chan++)
    {
        myPlaybackData.copyFrom(chan, 0, inputData.at(chan).data(), numSamples);
    }

    if (data_sr)
    {
        myPlaybackDataSR = data_sr;
    }
    else
    {
        myPlaybackDataSR = sr;
    }

    m_sample_rate = sr;
    defaultRubberBandOptions();
    init();
    resetWarpMarkers(120.);
}

PlaybackWarpProcessor::PlaybackWarpProcessor(std::string newUniqueName, nb::ndarray<float> input,
                                             double sr, double data_sr)
    : ProcessorBase{newUniqueName}
{
    createParameterLayout();
    m_sample_rate = sr;
    setData(input, data_sr);
    defaultRubberBandOptions();
    init();
    resetWarpMarkers(120.);
}

void PlaybackWarpProcessor::init()
{
    setAutomationVal("transpose", 0.);
    setupRubberband();
    setClipPositionsDefault();
}

void PlaybackWarpProcessor::setClipPositionsDefault()
{
    std::vector<std::tuple<double, double, double>> positions;

    positions.push_back(
        std::tuple<double, double, double>(0.f, std::numeric_limits<double>::max(), 0.));

    setClipPositions(positions);
}

void PlaybackWarpProcessor::prepareToPlay(double, int) {}

void PlaybackWarpProcessor::automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples)
{
    double scale = std::pow(2., getAutomationVal("transpose", posInfo) / 12.);
    m_rbstretcher->setPitchScale(scale * myPlaybackDataSR / m_sample_rate);
}

nb::ndarray<nb::numpy, float> PlaybackWarpProcessor::getWarpMarkers()
{
    size_t numMarkers = m_clipInfo.warp_markers.size();
    size_t shape[2] = {numMarkers, 2};

    float* array_data = new float[numMarkers * 2];

    size_t i = 0;
    for (auto& warp_marker : m_clipInfo.warp_markers)
    {
        array_data[i * 2 + 0] = warp_marker.first;  // time in seconds in the audio
        array_data[i * 2 + 1] = warp_marker.second; // time in beats in the audio, relative to 1.1.1
        i++;
    }

    auto capsule = nb::capsule(array_data,
                               [](void* p) noexcept
                               {
                                   // Only delete if Python is still running
                                   if (!Py_IsInitialized() || _Py_IsFinalizing())
                                   {
                                       // Python is shutting down, let it handle cleanup
                                       return;
                                   }
                                   delete[] static_cast<float*>(p);
                               });

    return nb::ndarray<nb::numpy, float>(array_data, 2, shape, capsule);
}

void PlaybackWarpProcessor::resetWarpMarkers(double bpm)
{
    if (bpm <= 0)
    {
        throw std::runtime_error("When resetting warp markers, the BPM must be greater than zero.");
    }

    m_clipInfo.warp_markers.clear();

    m_clipInfo.warp_markers.push_back(std::make_pair(0, 0));
    double beats = 1. / 32.;
    double durSeconds = beats * (60. / bpm);
    m_clipInfo.warp_markers.push_back(std::make_pair(durSeconds, beats));

    m_clipInfo.end_marker = m_clipInfo.loop_end = m_clipInfo.hidden_loop_end =
        (bpm / 60.) * (myPlaybackData.getNumSamples() / myPlaybackDataSR);
}

void PlaybackWarpProcessor::setWarpMarkers(nb::ndarray<float> input)
{
    if (input.ndim() != 2)
    {
        throw std::runtime_error(
            "The warp markers must be two-dimensional and shaped (num_markers, "
            "2).");
        return;
    }

    const int numPairs = (int)input.shape(0);

    if (numPairs < 2)
    {
        throw std::runtime_error("The number of warp markers must be greater than one.");
        return;
    }

    if (input.shape(1) != 2)
    {
        throw std::runtime_error("The dimensions of the passed warp markers are incorrect.");
        return;
    }

    std::vector<std::pair<double, double>> warp_markers;

    double beat, new_beat;
    double pos, new_pos;
    beat = new_beat = pos = new_pos = std::numeric_limits<float>::lowest();

    float* input_ptr = (float*)input.data();

    for (int pair_i = 0; pair_i < numPairs; pair_i++)
    {
        new_pos = *input_ptr++;
        new_beat = *input_ptr++;

        if (new_beat <= beat || new_pos <= pos)
        {
            throw std::runtime_error(
                "The warp markers must be monotonically increasing. new_beat: " +
                std::to_string(new_beat) + " beat: " + std::to_string(beat) +
                " new_pos: " + std::to_string(new_pos) + " pos: " + std::to_string(pos));
        }

        pos = new_pos;
        beat = new_beat;

        warp_markers.push_back(std::make_pair(pos, beat));
    }

    m_clipInfo.warp_markers = warp_markers;
}

bool PlaybackWarpProcessor::setClipPositions(
    std::vector<std::tuple<double, double, double>> positions)
{
    // a position is a (clip start, clip end, clip offset)
    // clip start: The position in beats relative to the engine's timeline where
    // the clip starts clip end: The position in beats relative to the engine's
    // timeline where the clip ends clip offset: A clip's first sample is
    // determined by the "start marker" in the ASD file.
    //              This is an offset to that start marker.

    m_clips.clear();

    for (auto& position : positions)
    {
        Clip clip = Clip((double)std::get<0>(position), (double)std::get<1>(position),
                         (double)std::get<2>(position));
        m_clips.push_back(clip);
    }

    return true;
}

void PlaybackWarpProcessor::processBlock(juce::AudioSampleBuffer& buffer,
                                         juce::MidiBuffer& midiBuffer)
{
    auto posInfo = getPlayHead()->getPosition();
    buffer.clear();

    if ((m_clips.size() == 0) || (m_clipIndex >= m_clips.size()))
    {
        // There are no clips, or we've already passed the last clip.
        ProcessorBase::processBlock(buffer, midiBuffer);
        return;
    }

    double movingPPQ = *posInfo->getPpqPosition();

    double nextPPQ = *posInfo->getPpqPosition() +
                     (double(buffer.getNumSamples()) / m_sample_rate) * (*posInfo->getBpm()) / 60.;

    std::uint32_t numAvailable = 0;
    const std::uint32_t numSamplesNeeded = buffer.getNumSamples();

    std::uint32_t numWritten = 0;
    std::uint64_t numToRetrieve = 0;

    while (numWritten < numSamplesNeeded)
    {
        // In this loop, figure out just one sample at a time.
        // There are a lot of things to juggle including:
        // The rubberband stretcher: does it have samples available? what samples
        // should we tell it to process? The global clip position: are we inside a
        // region that should be producing any kind of audio at all or silence? The
        // local clip position: Do we have samples for the requested sample index,
        // or do we need to loop to another position, or fake zeros? The clip info:
        // is warping enabled, is looping enabled.

        numAvailable = m_rbstretcher->available();

        numToRetrieve = std::min(numAvailable, numSamplesNeeded - numWritten);
        if (m_currentClip.end_pos < std::numeric_limits<double>::max())
        {
            numToRetrieve =
                std::min(numToRetrieve,
                         (std::uint64_t)(std::ceil((m_currentClip.end_pos - movingPPQ) /
                                                   (*posInfo->getBpm()) * 60. * m_sample_rate)));
        }

        if (numToRetrieve > 0)
        {
            m_nonInterleavedBuffer.setSize(m_numChannels, (int)numToRetrieve);
            numToRetrieve = m_rbstretcher->retrieve(
                m_nonInterleavedBuffer.getArrayOfWritePointers(), numToRetrieve);

            for (int chan = 0; chan < m_numChannels; chan++)
            {
                auto chanPtr = m_nonInterleavedBuffer.getReadPointer(chan);
                buffer.copyFrom(chan, numWritten, chanPtr, (int)numToRetrieve);
            }

            numWritten += numToRetrieve;
            movingPPQ += double(numToRetrieve) * *posInfo->getBpm() / (m_sample_rate * 60.);
            continue;
        }

        while (movingPPQ >= m_currentClip.end_pos)
        {
            m_clipIndex += 1;
            if (m_clipIndex < m_clips.size())
            {
                m_currentClip = m_clips.at(m_clipIndex);
                setupRubberband();
                if (m_clipInfo.warp_on)
                {
                    sampleReadIndex = m_clipInfo.beat_to_sample(
                        m_clipInfo.start_marker + m_currentClip.start_marker_offset,
                        myPlaybackDataSR);
                }
                else
                {
                    sampleReadIndex = 0;
                }
            }
            else
            {
                ProcessorBase::processBlock(buffer, midiBuffer);
                return;
            }
        }

        if (nextPPQ < m_currentClip.start_pos || movingPPQ < m_currentClip.start_pos)
        {
            // write some zeros into the output
            for (int chan = 0; chan < m_numChannels; chan++)
            {
                buffer.setSample(chan, numWritten, 0.f);
            }
            numWritten += 1;
            movingPPQ += (*posInfo->getBpm()) / (m_sample_rate * 60.);
            continue;
        }

        double ppqPosition =
            movingPPQ - m_currentClip.start_pos + m_currentClip.start_marker_offset;

        bool past_end_marker_and_loop_off =
            ppqPosition > m_clipInfo.end_marker && !m_clipInfo.loop_on;
        if (past_end_marker_and_loop_off || movingPPQ > m_currentClip.end_pos)
        {
            m_clipIndex += 1;
            if (m_clipIndex < m_clips.size())
            {
                // Use the next clip position.
                m_currentClip = m_clips.at(m_clipIndex);
                setupRubberband();
                if (m_clipInfo.warp_on)
                {
                    sampleReadIndex = m_clipInfo.beat_to_sample(
                        m_clipInfo.start_marker + m_currentClip.start_marker_offset,
                        myPlaybackDataSR);
                }
                else
                {
                    sampleReadIndex = 0;
                }
                continue;
            }
            else
            {
                for (int chan = 0; chan < m_numChannels; chan++)
                {
                    buffer.setSample(chan, numWritten, 0.f);
                }
                numWritten += 1;
                movingPPQ += (*posInfo->getBpm()) / (m_sample_rate * 60.);
                continue;
            }
        }

        if (m_clipInfo.warp_on)
        {
            double instant_bpm;
            double _;
            // wrap-around the ppqPosition
            if (m_clipInfo.loop_on)
            {
                if (ppqPosition > m_clipInfo.loop_end - m_clipInfo.loop_start)
                {
                    auto loopSize = m_clipInfo.loop_end - m_clipInfo.loop_start;
                    ppqPosition -=
                        std::ceil((ppqPosition - m_clipInfo.loop_end) / loopSize) * loopSize;
                }

                int loop_end_sample =
                    m_clipInfo.beat_to_sample(m_clipInfo.loop_end, myPlaybackDataSR);
                if (sampleReadIndex > loop_end_sample)
                {
                    int loop_start_sample =
                        m_clipInfo.beat_to_sample(m_clipInfo.loop_start, myPlaybackDataSR);
                    sampleReadIndex = loop_start_sample;
                }
            }

            m_clipInfo.beat_to_seconds(ppqPosition, _, instant_bpm);
            m_rbstretcher->setTimeRatio((instant_bpm / (*posInfo->getBpm())) *
                                        (m_sample_rate / myPlaybackDataSR));
        }
        else
        {
            m_rbstretcher->setTimeRatio(m_time_ratio_if_warp_off *
                                        (m_sample_rate / myPlaybackDataSR));
        }

        m_nonInterleavedBuffer.setSize(m_numChannels, 1);

        // Can we read from the playback data?
        const int last_sample = myPlaybackData.getNumSamples() - 1;
        if (sampleReadIndex > -1 && sampleReadIndex <= last_sample)
        {
            for (int chan = 0; chan < m_numChannels; chan++)
            {
                m_nonInterleavedBuffer.copyFrom(chan, 0, myPlaybackData, chan, sampleReadIndex, 1);
            }
        }
        else
        {
            // We are asking for out of bounds samples, so pass zeros.
            m_nonInterleavedBuffer.clear();
        }

        m_rbstretcher->process(m_nonInterleavedBuffer.getArrayOfReadPointers(),
                               m_nonInterleavedBuffer.getNumSamples(), false);

        sampleReadIndex += 1;
    }

    ProcessorBase::processBlock(buffer, midiBuffer);
}

void PlaybackWarpProcessor::reset()
{
    setupRubberband();

    m_clipIndex = 0;
    sampleReadIndex = 0;

    if (m_clipIndex < m_clips.size())
    {
        m_currentClip = m_clips.at(0);
        if (m_clipInfo.warp_on)
        {
            sampleReadIndex = m_clipInfo.beat_to_sample(
                m_clipInfo.start_marker + m_currentClip.start_marker_offset, myPlaybackDataSR);
        }
        else
        {
            sampleReadIndex = 0;
        }
    }

    ProcessorBase::reset();
}

void PlaybackWarpProcessor::setData(nb::ndarray<float> input, double data_sr)
{
    float* input_ptr = (float*)input.data();

    m_numChannels = (int)input.shape(0);
    setMainBusInputsAndOutputs(0, m_numChannels);
    const int numSamples = (int)input.shape(1);

    myPlaybackData.setSize(m_numChannels, numSamples);

    // Get strides - nanobind returns ELEMENT strides, not byte strides
    size_t elem_stride_ch = input.stride(0);     // stride for channel dimension (in elements)
    size_t elem_stride_sample = input.stride(1); // stride for sample dimension (in elements)

    // Check if C-contiguous (row-major): channels x samples
    bool is_c_contiguous = (elem_stride_sample == 1 && elem_stride_ch == numSamples);

    if (is_c_contiguous)
    {
        // Fast path for C-contiguous arrays
        for (int chan = 0; chan < m_numChannels; chan++)
        {
            myPlaybackData.copyFrom(chan, 0, input_ptr, numSamples);
            input_ptr += numSamples;
        }
    }
    else
    {
        // General path using strides (handles F-contiguous and other layouts)
        for (int chan = 0; chan < m_numChannels; chan++)
        {
            float* chan_ptr = input_ptr + (chan * elem_stride_ch);
            float* dest = myPlaybackData.getWritePointer(chan);
            for (int samp = 0; samp < numSamples; samp++)
            {
                dest[samp] = chan_ptr[samp * elem_stride_sample];
            }
        }
    }

    if (data_sr)
    {
        myPlaybackDataSR = data_sr;
    }
    else
    {
        myPlaybackDataSR = m_sample_rate;
    }
}

bool PlaybackWarpProcessor::loadAbletonClipInfo(const char* filepath)
{
    return m_clipInfo.readWarpFile(filepath);
}

void PlaybackWarpProcessor::setRubberBandOptions(int options)
{
    using namespace RubberBand;

    // these settings are non-negotiable!
    options |= RubberBandStretcher::OptionProcessRealTime;
    options |= RubberBandStretcher::OptionStretchPrecise;
    options |= RubberBandStretcher::OptionThreadingNever;

    m_rubberbandConfig = options;
}

void PlaybackWarpProcessor::defaultRubberBandOptions()
{
    using namespace RubberBand;

    RubberBandStretcher::Options options = 0;

    // options |= RubberBandStretcher::OptionProcessOffline;
    options |= RubberBandStretcher::OptionProcessRealTime;

    // options |= RubberBandStretcher::OptionStretchElastic;
    options |= RubberBandStretcher::OptionStretchPrecise;

    options |= RubberBandStretcher::OptionTransientsCrisp;
    // options |= RubberBandStretcher::OptionTransientsMixed;
    // options |= RubberBandStretcher::OptionTransientsSmooth;

    options |= RubberBandStretcher::OptionDetectorCompound;
    // options |= RubberBandStretcher::OptionDetectorPercussive;
    // options |= RubberBandStretcher::OptionDetectorSoft;

    options |= RubberBandStretcher::OptionPhaseLaminar;
    // options |= RubberBandStretcher::OptionPhaseIndependent;

    // options |= RubberBandStretcher::OptionThreadingAuto;
    options |= RubberBandStretcher::OptionThreadingNever;
    // options |= RubberBandStretcher::OptionThreadingAlways;

    options |= RubberBandStretcher::OptionWindowStandard;
    // options |= RubberBandStretcher::OptionWindowShort;
    // options |= RubberBandStretcher::OptionWindowLong;

    options |= RubberBandStretcher::OptionSmoothingOff;
    // options |= RubberBandStretcher::OptionSmoothingOn;

    options |= RubberBandStretcher::OptionFormantShifted;
    // options |= RubberBandStretcher::OptionFormantPreserved;

    // options |= RubberBandStretcher::OptionPitchHighSpeed;
    options |= RubberBandStretcher::OptionPitchHighQuality; // NOT the default, so remember to pass
                                                            // this when doing set_options in Python
    // options |= RubberBandStretcher::OptionPitchHighConsistency;

    options |= RubberBandStretcher::OptionChannelsApart;
    // options |= RubberBandStretcher::OptionChannelsTogether;

    this->setRubberBandOptions(options);
}

void PlaybackWarpProcessor::setupRubberband()
{
    // Note that we call this instead of calling m_rbstretcher->reset() because
    // that method doesn't seem to work correctly.
    // It's better to just create a whole new stretcher object.

    m_rbstretcher = std::make_unique<RubberBand::RubberBandStretcher>(m_sample_rate, m_numChannels,
                                                                      m_rubberbandConfig, 1., 1.);
}

void PlaybackWarpProcessor::createParameterLayout()
{
    juce::AudioProcessorParameterGroup group;

    group.addChild(std::make_unique<AutomateParameterFloat>(
        "transpose", "transpose", NormalisableRange<float>(-96.f, 96.f), 0.f));

    this->setParameterTree(std::move(group));

    int i = 0;
    for (auto* parameter : this->getParameters())
    {
        // give it a valid single sample of automation.
        ProcessorBase::setAutomationValByIndex(i, parameter->getValue());
        i++;
    }
}

#endif

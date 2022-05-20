#include "PlaybackWarpProcessor.h"
#ifdef BUILD_DAWDREAMER_RUBBERBAND

PlaybackWarpProcessor::PlaybackWarpProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData, double sr, double data_sr) : ProcessorBase{ createParameterLayout, newUniqueName }
{
    m_numChannels = (int)inputData.size();
    setMainBusInputsAndOutputs(0, m_numChannels);
    const int numSamples = (int)inputData.at(0).size();

    myPlaybackData.setSize(m_numChannels, numSamples);
    for (int chan = 0; chan < m_numChannels; chan++) {
        myPlaybackData.copyFrom(chan, 0, inputData.at(chan).data(), numSamples);
    }

    if (data_sr) {
        myPlaybackDataSR = data_sr;
    }
    else {
        myPlaybackDataSR = sr;
    }

    m_sample_rate = sr;

    init();
}

PlaybackWarpProcessor::PlaybackWarpProcessor(std::string newUniqueName, py::array_t<float, py::array::c_style | py::array::forcecast> input, double sr, double data_sr) : ProcessorBase{ createParameterLayout, newUniqueName }
{
    m_sample_rate = sr;
    setData(input, data_sr);
    init();
}

void
PlaybackWarpProcessor::init() {
    setAutomationVal("transpose", 0.);
    myTranspose = myParameters.getRawParameterValue("transpose");
    setupRubberband();
    setClipPositionsDefault();
}


void
PlaybackWarpProcessor::setClipPositionsDefault() {

    std::vector<std::tuple<float, float, float>> positions;

    positions.push_back(std::tuple<float, float, float>(0.f, 65536.f, 0.f));

    setClipPositions(positions);
}

void
PlaybackWarpProcessor::prepareToPlay(double, int) {

}

void
PlaybackWarpProcessor::automateParameters() {

    AudioPlayHead::CurrentPositionInfo posInfo;
    getPlayHead()->getCurrentPosition(posInfo);

    *myTranspose = getAutomationVal("transpose", posInfo);
    double scale = std::pow(2., *myTranspose / 12.);
    m_rbstretcher->setPitchScale(scale * myPlaybackDataSR / m_sample_rate);
}


py::array_t<float>
PlaybackWarpProcessor::getWarpMarkers() {

    py::array_t<float, py::array::c_style> arr({ (int)m_clipInfo.warp_markers.size(), 2 });

    auto ra = arr.mutable_unchecked();

    int i = 0;
    for (auto& warp_marker : m_clipInfo.warp_markers) {
        ra(i, 0) = warp_marker.first; // time in seconds in the audio
        ra(i, 1) = warp_marker.second; // time in beats in the audio, relative to 1.1.1
        i++;
    }

    return arr;
}

void
PlaybackWarpProcessor::resetWarpMarkers(double bpm) {
    m_clipInfo.warp_markers.clear();

    m_clipInfo.warp_markers.push_back(std::make_pair(0, 0));
    double numSamples = 128;
    double beats = bpm / (60. * numSamples / myPlaybackDataSR);
    m_clipInfo.warp_markers.push_back(std::make_pair(numSamples, beats));
}

void
PlaybackWarpProcessor::setWarpMarkers(py::array_t<float, py::array::c_style | py::array::forcecast> input) {

    if (input.ndim() != 2) {
        throw std::runtime_error("The warp markers must be two-dimensional and shaped (num_markers, 2).");
        return;
    }

    const int numPairs = (int)input.shape(0);

    if (numPairs < 2) {
        throw std::runtime_error("The number of warp markers must be greater than one.");
        return;
    }

    if (input.shape(1) != 2) {
        throw std::runtime_error("The dimensions of the passed warp markers are incorrect.");
        return;
    }

    std::vector<std::pair<double, double>> warp_markers;

    double beat, new_beat;
    double pos, new_pos;
    beat = new_beat = pos = new_pos = -999999.;

    float* input_ptr = (float*)input.data();

    for (int pair_i = 0; pair_i < numPairs; pair_i++) {

        new_pos = *input_ptr++;
        new_beat = *input_ptr++;

        if (new_beat <= beat || new_pos <= pos) {
            throw std::runtime_error("The warp markers must be monotonically increasing. new_beat: " + std::to_string(new_beat) + " beat: " + std::to_string(beat) + " new_pos: " + std::to_string(new_pos) + " pos: " + std::to_string(pos));
        }

        pos = new_pos;
        beat = new_beat;

        warp_markers.push_back(std::make_pair(pos, beat));
    }

    m_clipInfo.warp_markers = warp_markers;
}


bool
PlaybackWarpProcessor::setClipPositions(std::vector<std::tuple<float, float, float>> positions) {

    // a position is a (clip start, clip end, clip offset)
    // clip start: The position in beats relative to the engine's timeline where the clip starts
    // clip end: The position in beats relative to the engine's timeline where the clip ends
    // clip offset: A clip's first sample is determined by the "start marker" in the ASD file.
    //              This is an offset to that start marker.

    m_clips.clear();

    for (auto& position : positions) {

        Clip clip = Clip((double)std::get<0>(position), (double)std::get<1>(position), (double)std::get<2>(position));
        m_clips.push_back(clip);
    }

    return true;
}

void
PlaybackWarpProcessor::processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer)
{
    AudioPlayHead::CurrentPositionInfo posInfo;
    getPlayHead()->getCurrentPosition(posInfo);

    automateParameters();

    if (m_clips.size() == 0) {
        ProcessorBase::processBlock(buffer, midiBuffer);
        return;
    }

    if (m_clipIndex >= m_clips.size()) {
        // we've already passed the last clip.
        ProcessorBase::processBlock(buffer, midiBuffer);
        return;
    }

    double movingPPQ = posInfo.ppqPosition;

    double nextPPQ = posInfo.ppqPosition + (double(buffer.getNumSamples()) / m_sample_rate) * posInfo.bpm / 60.;

    std::uint32_t numAvailable = 0;
    const std::uint32_t numSamplesNeeded = buffer.getNumSamples();

    std::uint32_t numWritten = 0;
    std::uint64_t numToRetrieve = 0;

    while (numWritten < numSamplesNeeded) {
        // In this loop, figure out just one sample at a time.
        // There are a lot of things to juggle including:
        // The rubberband stretcher: does it have samples available? what samples should we tell it to process?
        // The global clip position: are we inside a region that should be producing any kind of audio at all or silence?
        // The local clip position: Do we have samples for the requested sample index, or do we need to loop to another position, or fake zeros?
        // The clip info: is warping enabled, is looping enabled.

        numAvailable = m_rbstretcher->available();

        numToRetrieve = std::min(numAvailable, numSamplesNeeded - numWritten);
        numToRetrieve = std::min(numToRetrieve, (std::uint64_t)(std::ceil((m_currentClip.end_pos - movingPPQ) / (posInfo.bpm) * 60. * m_sample_rate)));

        if (numToRetrieve > 0) {
            m_nonInterleavedBuffer.setSize(m_numChannels, numToRetrieve);
            numToRetrieve = m_rbstretcher->retrieve(m_nonInterleavedBuffer.getArrayOfWritePointers(), numToRetrieve);

            for (int chan = 0; chan < m_numChannels; chan++) {
                auto chanPtr = m_nonInterleavedBuffer.getReadPointer(chan);
                buffer.copyFrom(chan, numWritten, chanPtr, numToRetrieve);
            }

            numWritten += numToRetrieve;
            movingPPQ += double(numToRetrieve) * posInfo.bpm / (m_sample_rate * 60.);
            continue;
        }

        while (movingPPQ >= m_currentClip.end_pos) {
            m_clipIndex += 1;
            if (m_clipIndex < m_clips.size()) {
                m_currentClip = m_clips.at(m_clipIndex);
                setupRubberband();
                if (m_clipInfo.warp_on) {
                    sampleReadIndex = m_clipInfo.beat_to_sample(m_clipInfo.start_marker + m_currentClip.start_marker_offset, myPlaybackDataSR);
                }
                else {
                    sampleReadIndex = 0;
                }
            }
            else {
                ProcessorBase::processBlock(buffer, midiBuffer);
                return;
            }
        }

        if (nextPPQ < m_currentClip.start_pos || movingPPQ < m_currentClip.start_pos) {
            // write some zeros into the output
            for (int chan = 0; chan < m_numChannels; chan++) {
                buffer.setSample(chan, numWritten, 0.f);
            }
            numWritten += 1;
            movingPPQ += posInfo.bpm / (m_sample_rate * 60.);
            continue;
        }

        double ppqPosition = movingPPQ - m_currentClip.start_pos + m_currentClip.start_marker_offset;

        bool past_end_marker_and_loop_off = ppqPosition > m_clipInfo.end_marker && !m_clipInfo.loop_on;
        if (past_end_marker_and_loop_off || movingPPQ > m_currentClip.end_pos) {
            m_clipIndex += 1;
            if (m_clipIndex < m_clips.size()) {
                // Use the next clip position.
                m_currentClip = m_clips.at(m_clipIndex);
                setupRubberband();
                if (m_clipInfo.warp_on) {
                    sampleReadIndex = m_clipInfo.beat_to_sample(m_clipInfo.start_marker + m_currentClip.start_marker_offset, myPlaybackDataSR);
                }
                else {
                    sampleReadIndex = 0;
                }
                continue;
            }
            else {
                for (int chan = 0; chan < m_numChannels; chan++) {
                    buffer.setSample(chan, numWritten, 0.f);
                }
                numWritten += 1;
                movingPPQ += posInfo.bpm / (m_sample_rate * 60.);
                continue;
            }
        }

        if (m_clipInfo.warp_on) {
            double instant_bpm;
            double _;
            m_clipInfo.beat_to_seconds(ppqPosition, _, instant_bpm);
            m_rbstretcher->setTimeRatio((instant_bpm / posInfo.bpm) * (m_sample_rate / myPlaybackDataSR));
        }
        else {
            m_rbstretcher->setTimeRatio(m_time_ratio_if_warp_off * (m_sample_rate / myPlaybackDataSR));
        }

        if (m_clipInfo.loop_on) {
            int loop_end_sample = m_clipInfo.beat_to_sample(m_clipInfo.loop_end, myPlaybackDataSR);
            if (sampleReadIndex > loop_end_sample) {
                int loop_start_sample = m_clipInfo.beat_to_sample(m_clipInfo.loop_start, myPlaybackDataSR);
                sampleReadIndex = loop_start_sample;
            }
        }

        m_nonInterleavedBuffer.setSize(m_numChannels, 1);

        // can we read from the playback data or are we out of bounds and we need to pass zeros to rubberband?
        const int last_sample = myPlaybackData.getNumSamples() - 1;
        if (sampleReadIndex > -1 && sampleReadIndex <= last_sample) {
            for (int chan = 0; chan < m_numChannels; chan++) {
                m_nonInterleavedBuffer.copyFrom(chan, 0, myPlaybackData, chan, sampleReadIndex, 1);
            }
        }
        else {
            // pass zeros because the requested clip loop parameters are asking for out of bounds samples.
            m_nonInterleavedBuffer.clear();
        }

        m_rbstretcher->process(m_nonInterleavedBuffer.getArrayOfReadPointers(), m_nonInterleavedBuffer.getNumSamples(), false);

        sampleReadIndex += 1;
    }

    ProcessorBase::processBlock(buffer, midiBuffer);
}

void
PlaybackWarpProcessor::reset() {

    setupRubberband();

    m_clipIndex = 0;
    sampleReadIndex = 0;

    if (m_clipIndex < m_clips.size()) {
        m_currentClip = m_clips.at(0);
        if (m_clipInfo.warp_on) {
            sampleReadIndex = m_clipInfo.beat_to_sample(m_clipInfo.start_marker + m_currentClip.start_marker_offset, myPlaybackDataSR);
        }
        else {
            sampleReadIndex = 0;
        }
    }
}

void
PlaybackWarpProcessor::setData(py::array_t<float, py::array::c_style | py::array::forcecast> input, double data_sr) {
    float* input_ptr = (float*)input.data();

    m_numChannels = (int)input.shape(0);
    setMainBusInputsAndOutputs(0, m_numChannels);
    const int numSamples = (int)input.shape(1);

    myPlaybackData.setSize(m_numChannels, numSamples);
    for (int chan = 0; chan < m_numChannels; chan++) {
        myPlaybackData.copyFrom(chan, 0, input_ptr, numSamples);
        input_ptr += numSamples;
    }

    if (data_sr) {
        myPlaybackDataSR = data_sr;
    }
    else {
        myPlaybackDataSR = m_sample_rate;
    }
}

bool
PlaybackWarpProcessor::loadAbletonClipInfo(const char* filepath) {
    return m_clipInfo.readWarpFile(filepath);
}

void
PlaybackWarpProcessor::setupRubberband() {
    // Note that we call this instead of calling m_rbstretcher->reset() because
    // that method doesn't seem to work correctly.
    // It's better to just create a whole new stretcher object.
    using namespace RubberBand;

    RubberBandStretcher::Options options = 0;

    //options |= RubberBandStretcher::OptionProcessOffline;
    options |= RubberBandStretcher::OptionProcessRealTime;  // NOT the default

    //options |= RubberBandStretcher::OptionStretchElastic;
    options |= RubberBandStretcher::OptionStretchPrecise;  // NOT the default

    //options |= RubberBandStretcher::OptionTransientsCrisp;
    //options |= RubberBandStretcher::OptionTransientsMixed;
    options |= RubberBandStretcher::OptionTransientsSmooth;  // NOT the default

    options |= RubberBandStretcher::OptionDetectorCompound;
    //options |= RubberBandStretcher::OptionDetectorPercussive;
    //options |= RubberBandStretcher::OptionDetectorSoft;

    options |= RubberBandStretcher::OptionPhaseLaminar;
    //options |= RubberBandStretcher::OptionPhaseIndependent;

    //options |= RubberBandStretcher::OptionThreadingAuto;
    options |= RubberBandStretcher::OptionThreadingNever;  // NOT the default
    //options |= RubberBandStretcher::OptionThreadingAlways;

    options |= RubberBandStretcher::OptionWindowStandard;
    //options |= RubberBandStretcher::OptionWindowShort;
    //options |= RubberBandStretcher::OptionWindowLong;

    options |= RubberBandStretcher::OptionSmoothingOff;
    //options |= RubberBandStretcher::OptionSmoothingOn;

    options |= RubberBandStretcher::OptionFormantShifted;
    //options |= RubberBandStretcher::OptionFormantPreserved;

    //options |= RubberBandStretcher::OptionPitchHighSpeed;
    options |= RubberBandStretcher::OptionPitchHighQuality;  // NOT the default
    //options |= RubberBandStretcher::OptionPitchHighConsistency;

    //options |= RubberBandStretcher::OptionChannelsApart;
    options |= RubberBandStretcher::OptionChannelsTogether;  // NOT the default

    m_rbstretcher = std::make_unique<RubberBand::RubberBandStretcher>(
        m_sample_rate,
        m_numChannels,
        options,
        1.,
        1.);
}

juce::AudioProcessorValueTreeState::ParameterLayout
PlaybackWarpProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;

    params.add(std::make_unique<AutomateParameterFloat>("transpose", "transpose", NormalisableRange<float>(-96.f, 96.f), 0.f));
    return params;
}

#endif
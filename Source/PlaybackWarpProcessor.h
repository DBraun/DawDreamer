#pragma once

#ifdef BUILD_DAWDREAMER_RUBBERBAND

#include "ProcessorBase.h"
#include "custom_pybind_wrappers.h"

#include "rubberband/RubberBandStretcher.h"
#include "AbletonClipInfo.h"

class PlaybackWarpProcessor : public ProcessorBase
{
public:
    PlaybackWarpProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData, double sr) : ProcessorBase{ createParameterLayout, newUniqueName }
    {
        const int numChannels = (int) inputData.size();
        const int numSamples = (int)inputData.at(0).size();
        
        // set to stereo
        myPlaybackData.setSize(channels, numSamples, false, false, false);
        for (int chan = 0; chan < channels; chan++) {
            myPlaybackData.copyFrom(chan, 0, inputData.at(std::min(chan, numChannels)).data(), numSamples);
        }

        init(sr);
    }

    PlaybackWarpProcessor(std::string newUniqueName, py::array_t<float, py::array::c_style | py::array::forcecast> input, double sr) : ProcessorBase{ createParameterLayout, newUniqueName }
    {
        setData(input);
        init(sr);
    }

private:
    void init(double sr) {
        m_sample_rate = sr;
        setAutomationVal("transpose", 0.);
        myTranspose = myParameters.getRawParameterValue("transpose");
        setupRubberband(sr);
        setClipPositionsDefault();
    }

    void setClipPositionsDefault() {

        std::vector<std::tuple<float, float, float>> positions;

        positions.push_back(std::tuple<float, float, float>(0.f, 65536.f, 0.f));

        setClipPositions(positions);
    }

public:
    void
    prepareToPlay(double, int) {
        m_clipIndex = 0;
        if (m_clipIndex < m_clips.size()) {
            m_currentClip = m_clips.at(0);
            m_rbstretcher->reset();
            if (m_clipInfo.warp_on) {
                sampleReadIndex = m_clipInfo.beat_to_sample(m_clipInfo.start_marker + m_currentClip.start_marker_offset, m_sample_rate);
            }
            else {
                sampleReadIndex = 0;
            }
        }

        reset();
    }

    void setTimeRatio(double ratio) {
        m_time_ratio_if_warp_off = ratio;
    }

    void automateParameters() {

        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        *myTranspose = getAutomationVal("transpose", posInfo.timeInSamples);
        double scale = std::pow(2., *myTranspose / 12.);
        m_rbstretcher->setPitchScale(scale);
    }

    void setTranspose(float newVal) { setAutomationVal("transpose", newVal); }
    float getTranspose() { return getAutomationVal("transpose", 0); }

    bool getWarpOn() { return m_clipInfo.warp_on; }
    void setWarpOn(bool warpOn) { m_clipInfo.warp_on = warpOn; }

    bool getLoopOn() { return m_clipInfo.loop_on; }
    void setLoopOn(bool loopOn) { m_clipInfo.loop_on = loopOn; }
    double getLoopStart() { return m_clipInfo.loop_start; }
    void setLoopStart(double loopStart) { m_clipInfo.loop_start = loopStart; }
    double getLoopEnd() { return m_clipInfo.loop_end; }
    void setLoopEnd(double loopEnd) { m_clipInfo.loop_end = loopEnd; }
    double getStartMarker() { return m_clipInfo.start_marker; }
    void setStartMarker(double startMarker) { m_clipInfo.start_marker = startMarker; }
    double getEndMarker() { return m_clipInfo.end_marker; }
    void setEndMarker(double endMarker) { m_clipInfo.end_marker = endMarker;}

private:
    class Clip {
    public:
        double start_pos = 0.;
        double end_pos = 4.;
        double start_marker_offset = 0.;
    };
public:

    bool setClipPositions(std::vector<std::tuple<float, float, float>> positions) {

        // a position is a (clip start, clip end, clip offset)
        // clip start: The position in beats relative to the engine's timeline where the clip starts
        // clip end: The position in beats relative to the engine's timeline where the clip ends
        // clip offset: A clip's first sample is determined by the "start marker" in the ASD file.
        //              This is an offset to that start marker.

        m_clips.clear();

        for (auto& position : positions) {

            Clip clip;
            clip.start_pos = (double)std::get<0>(position);
            clip.end_pos = (double)std::get<1>(position);
            clip.start_marker_offset = (double)std::get<2>(position);

            m_clips.push_back(clip);
        }

        return true;
    }
   
    void
    processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
    {
        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        automateParameters();

        if (m_clips.size() == 0) {
            return;
        }

        if (m_clipIndex >= m_clips.size()) {
            // we've already passed the last clip.
            return;
        }

        double movingPPQ = posInfo.ppqPosition;

        double nextPPQ = posInfo.ppqPosition + (double(buffer.getNumSamples())/ m_sample_rate) * posInfo.bpm / 60.;
        
        int numAvailable = 0;
        const int numSamplesNeeded = buffer.getNumSamples();

        int numWritten = 0;

        while (numWritten < numSamplesNeeded) {

            numAvailable = m_rbstretcher->available();

            int numToRetrieve = std::min(numAvailable, numSamplesNeeded - numWritten);
            numToRetrieve = std::min(numToRetrieve,int(std::ceil( (m_currentClip.end_pos-movingPPQ)/(posInfo.bpm)*60.*m_sample_rate)));

            if (numToRetrieve > 0) {
                m_nonInterleavedBuffer.setSize(channels, numToRetrieve, false, true);
                m_rbstretcher->retrieve(m_nonInterleavedBuffer.getArrayOfWritePointers(), numToRetrieve);

                for (int chan = 0; chan < channels; chan++) {
                    auto chanPtr = m_nonInterleavedBuffer.getReadPointer(chan);
                    buffer.copyFrom(chan, numWritten, chanPtr, numToRetrieve);
                }

                numWritten += numToRetrieve;
                movingPPQ += (double)(numToRetrieve)*posInfo.bpm / (m_sample_rate * 60.);
                continue;
            }

            while (movingPPQ >= m_currentClip.end_pos) {
                m_clipIndex += 1;
                if (m_clipIndex < m_clips.size()) {
                    m_currentClip = m_clips.at(m_clipIndex);
                    m_rbstretcher->reset();
                    if (m_clipInfo.warp_on) {
                        sampleReadIndex = m_clipInfo.beat_to_sample(m_clipInfo.start_marker + m_currentClip.start_marker_offset, m_sample_rate);
                    }
                    else {
                        sampleReadIndex = 0;
                    }
                }
                else {
                    return;
                }
            }

            if (nextPPQ < m_currentClip.start_pos || movingPPQ < m_currentClip.start_pos) {
                // write some zeros into the output
                for (int chan = 0; chan < channels; chan++) {
                    buffer.setSample(chan, numWritten, 0.f);
                }
                numWritten += 1;
                movingPPQ += posInfo.bpm / (m_sample_rate * 60.);

                continue;
            }

            double ppqPosition = movingPPQ - m_currentClip.start_pos;

            bool past_end_marker_and_loop_off = ppqPosition > m_clipInfo.end_marker && !m_clipInfo.loop_on;
            if (past_end_marker_and_loop_off || movingPPQ > m_currentClip.end_pos) {
                m_clipIndex += 1;
                if (m_clipIndex < m_clips.size()) {
                    // Use the next clip position.
                    m_currentClip = m_clips.at(m_clipIndex);
                    m_rbstretcher->reset();
                    if (m_clipInfo.warp_on) {
                        sampleReadIndex = m_clipInfo.beat_to_sample(m_clipInfo.start_marker, m_sample_rate);
                    }
                    else {
                        sampleReadIndex = 0;
                    }
                    continue;
                }
                else {
                    for (int chan = 0; chan < channels; chan++) {
                        buffer.setSample(chan, numWritten, 0.f);
                    }
                    numWritten += 1;
                    movingPPQ += posInfo.bpm / (m_sample_rate * 60.);

                    continue;
                }
            }

            int loop_start_sample, loop_end_sample, end_marker_sample;
            if (m_clipInfo.warp_on) {
                loop_start_sample = m_clipInfo.beat_to_sample(m_clipInfo.loop_start, m_sample_rate);
                loop_end_sample = m_clipInfo.beat_to_sample(m_clipInfo.loop_end, m_sample_rate);
                end_marker_sample = m_clipInfo.beat_to_sample(m_clipInfo.end_marker, m_sample_rate);
                
                const int last_sample = myPlaybackData.getNumSamples()-1;
                loop_start_sample = std::min(loop_start_sample, last_sample);
                loop_end_sample = std::min(loop_end_sample, last_sample);
                end_marker_sample = std::min(end_marker_sample, last_sample);
                
            }
            else {
                loop_start_sample = 0;
                loop_end_sample = myPlaybackData.getNumSamples() - 1;
                end_marker_sample = myPlaybackData.getNumSamples() - 1;
            }

            if (m_clipInfo.warp_on) {
                // todo: if the playback data sample rate is different than the engine's sr
                // then that would affect the call to setTimeRatio.

                double instant_bpm = -1.;
                double _;
                m_clipInfo.beat_to_seconds(ppqPosition, _, instant_bpm);
                m_rbstretcher->setTimeRatio(instant_bpm / posInfo.bpm);
            }
            else {
                m_rbstretcher->setTimeRatio(m_time_ratio_if_warp_off);
            }

            int count = 0;
            if (m_clipInfo.loop_on) {
                count = std::min(1, loop_end_sample - sampleReadIndex);
            }
            else {
                count = std::min(1, end_marker_sample - sampleReadIndex);
            }

            if (count <= 0) {
                if (m_clipInfo.loop_on) {
                    sampleReadIndex = loop_start_sample;
                    count = 1;
                }
                else {
                    continue;
                }
            }

            m_nonInterleavedBuffer.setSize(channels, count, false, true);
            
            for (int chan=0; chan < channels; chan++) {
                m_nonInterleavedBuffer.copyFrom(chan, 0, myPlaybackData, chan, sampleReadIndex, count);
            }
            
            m_rbstretcher->process(m_nonInterleavedBuffer.getArrayOfReadPointers(), m_nonInterleavedBuffer.getNumSamples(), false);
            
            sampleReadIndex += count;
        }
    }

    void
    reset() {
    }

    const juce::String getName() const { return "PlaybackWarpProcessor"; }

    void setData(py::array_t<float, py::array::c_style | py::array::forcecast> input) {
        float* input_ptr = (float*)input.data();
        
        const int numChannels = (int) input.shape(0);
        const int numSamples = (int) input.shape(1);

        myPlaybackData.setSize(numChannels, numSamples, false, false, false);
        for (int chan = 0; chan < channels; chan++) {
            myPlaybackData.copyFrom(chan, 0, input_ptr, numSamples);
            input_ptr += numSamples;
        }
    }

    bool loadAbletonClipInfo(const char* filepath) {
        return m_clipInfo.readWarpFile(filepath);;
    }

private:

    juce::AudioSampleBuffer myPlaybackData;

    std::unique_ptr<RubberBand::RubberBandStretcher> m_rbstretcher;

    const int channels = 2;

    juce::AudioSampleBuffer m_nonInterleavedBuffer;
    int sampleReadIndex = 0;

    AbletonClipInfo m_clipInfo;
    double m_sample_rate;

    double m_time_ratio_if_warp_off = 1.;
    std::atomic<float>* myTranspose;

    std::vector<Clip> m_clips;
    int m_clipIndex = 0;
    Clip m_currentClip;

    void setupRubberband(float sr) {
        using namespace RubberBand;

        RubberBandStretcher::Options options = 0;
        options |= RubberBandStretcher::OptionProcessRealTime;
        options |= RubberBandStretcher::OptionStretchPrecise;
        //options |= RubberBandStretcher::OptionPhaseIndependent;
        //options |= RubberBandStretcher::OptionWindowLong;
        //options |= RubberBandStretcher::OptionWindowShort;
        //options |= RubberBandStretcher::OptionSmoothingOn;
        //options |= RubberBandStretcher::OptionFormantPreserved;
        options |= RubberBandStretcher::OptionPitchHighQuality;
        //options |= RubberBandStretcher::OptionChannelsTogether;

        // Pick one of these:
        options |= RubberBandStretcher::OptionThreadingAuto;
        //options |= RubberBandStretcher::OptionThreadingNever;
        //options |= RubberBandStretcher::OptionThreadingAlways;

        // Pick one of these:
        options |= RubberBandStretcher::OptionTransientsSmooth;
        //options |= RubberBandStretcher::OptionTransientsMixed;
        //options |= RubberBandStretcher::OptionTransientsCrisp;

        // Pick one of these:
        options |= RubberBandStretcher::OptionDetectorCompound;
        //options |= RubberBandStretcher::OptionDetectorPercussive;
        //options |= RubberBandStretcher::OptionDetectorSoft;

        m_rbstretcher = std::make_unique<RubberBand::RubberBandStretcher>(
            sr,
            2,
            options,
            1.,
            1.);
    }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;

        params.add(std::make_unique<AutomateParameterFloat>("transpose", "transpose", NormalisableRange<float>(-96.f, 96.f), 0.f));
        return params;
    }
};

#endif

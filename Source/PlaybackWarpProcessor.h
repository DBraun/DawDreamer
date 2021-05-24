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
        myPlaybackData.setSize(inputData.size(), inputData.at(0).size(), false, false, false);
        for (int chan = 0; chan < channels; chan++) {
            myPlaybackData.copyFrom(chan, 0, inputData.at(0).data(), inputData.at(0).size());
        }

        m_sample_rate = sr;
        setAutomationVal("transpose", 0.);
        myTranspose = myParameters.getRawParameterValue("transpose");
        setupRubberband(sr);
    }

    PlaybackWarpProcessor(std::string newUniqueName, py::array_t<float, py::array::c_style | py::array::forcecast> input, double sr) : ProcessorBase{ createParameterLayout, newUniqueName }
    {
        setData(input);

        m_sample_rate = sr;
        setAutomationVal("transpose", 0.);
        myTranspose = myParameters.getRawParameterValue("transpose");
        setupRubberband(sr);
    }

    void
    prepareToPlay(double, int) {
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

    double getClipStart() { return m_clipStartPos; }
    void setClipStart(double clipStartPos) { m_clipStartPos = clipStartPos; }
    double getClipEnd() { return m_clipEndPos; }
    void setClipEnd(double clipEndPos) { m_clipEndPos = clipEndPos; }
    
    void
    processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
    {
        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        automateParameters();

        double nextPPQ = posInfo.ppqPosition + (buffer.getNumSamples() / m_sample_rate) * posInfo.bpm / 60.;

        if (nextPPQ < m_clipStartPos) {
            buffer.clear();
            return;
        }

        int deadspace = 0;
        if (m_clipStartPos >= posInfo.ppqPosition && m_clipStartPos < nextPPQ) {
            // We will play from the playback buffer for the first time.

            // deadspace is the number of zeros we'll insert before using the playback buffer.
            deadspace = (m_clipStartPos  - posInfo.ppqPosition) / posInfo.bpm * 60. * m_sample_rate;

            if (m_clipInfo.warp_on) {
                sampleReadIndex = m_clipInfo.beat_to_sample(m_clipInfo.start_marker, m_sample_rate);
            }
            else {
                sampleReadIndex = 0;
            }
        }
        else {
            deadspace = 0;
        }

        double ppqPosition = posInfo.ppqPosition - m_clipStartPos;

        bool past_end_marker_and_loop_off = ppqPosition > m_clipInfo.end_marker && !m_clipInfo.loop_on;
        if (past_end_marker_and_loop_off) {
            // write zeros
            buffer.clear();
            return;
        }

        int loop_start_sample, loop_end_sample, end_marker_sample;
        if (m_clipInfo.warp_on) {
            loop_start_sample = m_clipInfo.beat_to_sample(m_clipInfo.loop_start, m_sample_rate);
            loop_end_sample = m_clipInfo.beat_to_sample(m_clipInfo.loop_end, m_sample_rate);
            end_marker_sample = m_clipInfo.beat_to_sample(m_clipInfo.end_marker, m_sample_rate);
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
        
        int numAvailable = m_rbstretcher->available();
        int numSamplesNeeded = std::min(buffer.getNumSamples(), (int)((m_clipEndPos - posInfo.ppqPosition)/posInfo.bpm*60.*m_sample_rate));

        while (numAvailable < numSamplesNeeded) {

            int count = 0;
            if (m_clipInfo.loop_on) {
                count = std::min(buffer.getNumSamples(), loop_end_sample - sampleReadIndex);
            }
            else {
                count = std::min(buffer.getNumSamples(), end_marker_sample - sampleReadIndex);
            }

            if (count <= 0) {
                if (m_clipInfo.loop_on) {
                    sampleReadIndex = loop_start_sample;
                    continue;
                }
                else {
                    break;
                }
            }

            bool isFinal = (sampleReadIndex + count - deadspace >= end_marker_sample) && !m_clipInfo.loop_on;

            // m_nonInterleavedBuffer will be set to have "count" number of samples,
            // but we will only start writing after the deadspace-th sample.
            m_nonInterleavedBuffer.setSize(channels, count, false, true);
            m_nonInterleavedBuffer.clear();
            
            for (int chan=0; chan < channels; chan++) {
                auto readPtr = myPlaybackData.getReadPointer(chan);
                readPtr += sampleReadIndex;
                m_nonInterleavedBuffer.copyFrom(chan, deadspace, readPtr, count - deadspace);
            }
            
            m_rbstretcher->process(m_nonInterleavedBuffer.getArrayOfReadPointers(), m_nonInterleavedBuffer.getNumSamples(), isFinal);
            numAvailable = m_rbstretcher->available();

            sampleReadIndex += count - deadspace;
            deadspace = 0;  // NB: this is an important line. Next time through the while loop, deadspace should be zero.
        }

        int numToRetrieve = std::min(numAvailable, numSamplesNeeded);
        if (numToRetrieve > 0) {
            m_nonInterleavedBuffer.setSize(channels, numToRetrieve, false, true);
            m_rbstretcher->retrieve(m_nonInterleavedBuffer.getArrayOfWritePointers(), numToRetrieve);

            for (int chan = 0; chan < channels; chan++) {
                auto chanPtr = m_nonInterleavedBuffer.getReadPointer(chan);
                buffer.copyFrom(chan, 0, chanPtr, numToRetrieve);
            }
        }
    }

    void
    reset() {
    }

    const juce::String getName() const { return "PlaybackWarpProcessor"; }

    void setData(py::array_t<float, py::array::c_style | py::array::forcecast> input) {
        float* input_ptr = (float*)input.data();

        myPlaybackData.setSize(input.shape(0), input.shape(1), false, false, false);
        for (int chan = 0; chan < channels; chan++) {
            myPlaybackData.copyFrom(chan, 0, input_ptr, input.shape(1));
            input_ptr += input.shape(1);
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

    double m_clipStartPos = 0.;
    double m_clipEndPos = 99999.;

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
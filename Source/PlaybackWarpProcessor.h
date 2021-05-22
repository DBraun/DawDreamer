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

        if (m_warpOn) {
            sampleReadIndex = m_clipInfo.beat_to_sample(m_clipInfo.start_marker, m_sample_rate);
        }
        else {
            sampleReadIndex = 0;
        }
    }

    void setTimeRatio(double ratio) {
        m_time_ratio_if_warp_off = ratio;
    }

    void automateParameters() {

        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        *myTranspose = getAutomationVal("transpose", posInfo.timeInSamples);
        float scale = std::pow(2., *myTranspose / 12.);
        m_rbstretcher->setPitchScale(scale);
    }

    void setTranspose(float newVal) { setAutomationVal("transpose", newVal); }
    float getTranspose() { return getAutomationVal("transpose", 0); }

    void disableWarp() { m_warpOn = false; }

    void
    processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
    {
        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        automateParameters();

        bool past_end_marker_and_loop_off = posInfo.ppqPosition > m_clipInfo.end_marker && !m_clipInfo.loop_on;
        if (past_end_marker_and_loop_off) {
            // write zeros
            buffer.applyGain(0.);
            return;
        }

        int loop_start_sample, loop_end_sample, end_marker_sample;
        if (m_warpOn) {
            loop_start_sample = m_clipInfo.beat_to_sample(m_clipInfo.loop_start, m_sample_rate);
            loop_end_sample = m_clipInfo.beat_to_sample(m_clipInfo.loop_end, m_sample_rate);
            end_marker_sample = m_clipInfo.beat_to_sample(m_clipInfo.end_marker, m_sample_rate);
        }
        else {
            loop_start_sample = 0;
            loop_end_sample = myPlaybackData.getNumSamples()-1;
            end_marker_sample = myPlaybackData.getNumSamples() - 1;
        }

        if (m_warpOn) {
            // todo: consider differences in sample rate here

            double instant_bpm = -1.;
            double _;
            m_clipInfo.beat_to_seconds(posInfo.ppqPosition, _, instant_bpm);
            m_rbstretcher->setTimeRatio(instant_bpm / posInfo.bpm);
        }
        else {
            m_rbstretcher->setTimeRatio(m_time_ratio_if_warp_off);
        }

        double ppqPosition = posInfo.ppqPosition;
        
        int numAvailable = m_rbstretcher->available();
        while (numAvailable < buffer.getNumSamples()) {

            int count = 0;
            if (m_clipInfo.loop_on) {
                count = std::min(buffer.getNumSamples(), loop_end_sample - sampleReadIndex);
            }
            else {
                count = std::min(buffer.getNumSamples(), end_marker_sample - sampleReadIndex);
            }

            if (count == 0) {
                if (m_clipInfo.loop_on) {
                    sampleReadIndex = loop_start_sample;
                    continue;
                }
                else {
                    break;
                }
            }

            bool isFinal = (sampleReadIndex + count >= end_marker_sample) && !m_clipInfo.loop_on;

            m_nonInterleavedBuffer.setSize(channels, count);
            
            for (int chan=0; chan < channels; chan++) {
                auto readPtr = myPlaybackData.getReadPointer(chan);
                readPtr += sampleReadIndex;
                m_nonInterleavedBuffer.copyFrom(chan, 0, readPtr, count);
            }
            
            m_rbstretcher->process(m_nonInterleavedBuffer.getArrayOfReadPointers(), count, isFinal);
            numAvailable = m_rbstretcher->available();

            sampleReadIndex += count;
        }

        int numToRetrieve = std::min(numAvailable, buffer.getNumSamples());
        if (numToRetrieve > 0) {
            m_nonInterleavedBuffer.setSize(channels, numToRetrieve);
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
        m_warpOn = m_clipInfo.readWarpFile(filepath);
        return m_warpOn;
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

    bool m_warpOn = false;

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
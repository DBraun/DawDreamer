#pragma once

#include "ProcessorBase.h"
#include "custom_pybind_wrappers.h"

#include "rubberband/RubberBandStretcher.h"

class PlaybackWarpProcessor : public ProcessorBase
{
public:
    PlaybackWarpProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData, double sr) : ProcessorBase{ newUniqueName }
    {
        myPlaybackData.setSize(inputData.size(), inputData.at(0).size(), false, false, false);
        for (int chan = 0; chan < channels; chan++) {
            myPlaybackData.copyFrom(chan, 0, inputData.at(0).data(), inputData.at(0).size());
        }
        setupRubberband(sr);
    }

    PlaybackWarpProcessor(std::string newUniqueName, py::array_t<float, py::array::c_style | py::array::forcecast> input, double sr) : ProcessorBase{ newUniqueName }
    {
        setData(input);
        setupRubberband(sr);
    }

    void
    prepareToPlay(double sr, int blocksize) {
        
        reset();
        //m_rbstretcher->setTimeRatio(1.2);
        //m_rbstretcher->setPitchScale(1.2);
    }
    
    void setTranspose(double transpose) {

        float scale = std::pow(2., transpose / 12.);

        m_rbstretcher->setPitchScale(scale);
    }

    void setTimeRatio(double ratio) {
        m_rbstretcher->setTimeRatio(ratio);
    }

    void
    processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
    {
        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        int numAvailable = m_rbstretcher->available();
        while (numAvailable < buffer.getNumSamples()) {
            int count = std::min(buffer.getNumSamples(), myPlaybackData.getNumSamples() - sampleReadIndex);
            if (count == 0) {
                break;
            }

            bool isFinal = sampleReadIndex + count >= myPlaybackData.getNumSamples();

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
        sampleReadIndex = 0;
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

private:

    juce::AudioSampleBuffer myPlaybackData;

    std::unique_ptr<RubberBand::RubberBandStretcher> m_rbstretcher;

    const int channels = 2;

    juce::AudioSampleBuffer m_nonInterleavedBuffer;
    int sampleReadIndex = 0;

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

};

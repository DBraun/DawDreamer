#pragma once

#include "ProcessorBase.h"
#include "custom_pybind_wrappers.h"

class PlaybackProcessor : public ProcessorBase
{
public:
    PlaybackProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData) : ProcessorBase{ newUniqueName }
    {
        int numChannels = inputData.size();
        int numSamples = inputData.at(0).size();
        myPlaybackData.setSize(numChannels, numSamples);
        for (int chan = 0; chan < numChannels; chan++) {
            myPlaybackData.copyFrom(chan, 0, inputData.at(chan).data(), numSamples);
        }
        
        setMainBusInputsAndOutputs(0, numChannels);
    }

    PlaybackProcessor(std::string newUniqueName, py::array_t<float, py::array::c_style | py::array::forcecast> input) : ProcessorBase{ newUniqueName }
    {
        setData(input);
    }

    void
    prepareToPlay(double, int) {}

    void
    processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer)
    {
        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        buffer.applyGain(0.);

        int numSamples = std::min(buffer.getNumSamples(), myPlaybackData.getNumSamples() - (int)posInfo.timeInSamples);
        int numChannels = buffer.getNumChannels();
        for (int chan = 0; chan < numChannels; chan++) {
            auto srcPtr = myPlaybackData.getReadPointer(chan);
            srcPtr += posInfo.timeInSamples;
            buffer.copyFrom(chan, 0, srcPtr, numSamples);
        }

        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    void
    reset() {}

    const juce::String getName() const { return "PlaybackProcessor"; }

    void setData(py::array_t<float, py::array::c_style | py::array::forcecast> input) {
        float* input_ptr = (float*)input.data();

        myPlaybackData.setSize(input.shape(0), input.shape(1));
        
        int numChannels = input.shape(0);

        for (int y = 0; y < input.shape(1); y++) {
            for (int x = 0; x < input.shape(0); x++) {
                myPlaybackData.setSample(x, y, input_ptr[x * input.shape(1) + y]);
            }
        }
        setMainBusInputsAndOutputs(0, numChannels);
    }

private:

    juce::AudioSampleBuffer myPlaybackData;
};

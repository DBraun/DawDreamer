#pragma once

#include "ProcessorBase.h"
#include "custom_pybind_wrappers.h"

class PlaybackProcessor : public ProcessorBase
{
public:
    PlaybackProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData) : ProcessorBase{ newUniqueName }
    {
        myPlaybackData = inputData;
    }

    PlaybackProcessor(std::string newUniqueName, py::array input) : ProcessorBase{ newUniqueName }
    {
        setData(input);
    }

    void
    prepareToPlay(double, int)
    {
    }

    void
    processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
    {
        size_t myLength;
        if (myPlaybackData.size()) {
            myLength = myPlaybackData[0].size();
        }
        else {
            myLength = 0;
        }

        // todo:  buffer.getWritePointer would probably be faster.
        for (int i = 0; i < buffer.getNumSamples() && myPlaybackIndex < myLength; i++)
        {
            for (int chan = 0; chan < buffer.getNumChannels(); chan++) {
                buffer.setSample(chan, i, myPlaybackData[chan][myPlaybackIndex]);
            }
            myPlaybackIndex++;
        }
    }

    void
    reset()
    {
        myPlaybackIndex = 0;
    }

    const juce::String getName() const { return "PlaybackProcessor"; }

    void setData(py::array_t<float, py::array::c_style | py::array::forcecast> input) {
        float* input_ptr = (float*)input.data();

        myPlaybackData = std::vector<std::vector<float>>(input.shape(0), std::vector<float>(input.shape(1)));

        for (int y = 0; y < input.shape(1); y++) {
            for (int x = 0; x < input.shape(0); x++) {
                myPlaybackData[x][y] = *(input_ptr++);
            }
        }
    }

private:

    std::vector<std::vector<float>> myPlaybackData;
    uint32_t myPlaybackIndex = 0;

};
#pragma once

#include "ProcessorBase.h"
#include "custom_pybind_wrappers.h"

#include "rubberband/RubberBandStretcher.h"

class PlaybackWarpProcessor : public ProcessorBase
{
public:
    PlaybackWarpProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData) : ProcessorBase{ newUniqueName }
    {
        myPlaybackData = inputData;
    }

    PlaybackWarpProcessor(std::string newUniqueName, py::array_t<float, py::array::c_style | py::array::forcecast> input) : ProcessorBase{ newUniqueName }
    {
        setData(input);
    }

    void
    prepareToPlay(double, int) {}

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

        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        int i = 0;

        // todo:  buffer.getWritePointer would probably be faster.
        for (i = 0; i < buffer.getNumSamples() && posInfo.timeInSamples < myLength; i++)
        {
            for (int chan = 0; chan < buffer.getNumChannels(); chan++) {
                buffer.setSample(chan, i, myPlaybackData[chan][posInfo.timeInSamples+i]);
            }
        }
        for (; i < buffer.getNumSamples(); i++) {
            for (int chan = 0; chan < buffer.getNumChannels(); chan++) {
                buffer.setSample(chan, i, 0.);
            }
        }
    }

    void
    reset() {}

    const juce::String getName() const { return "PlaybackProcessor"; }

    void setData(py::array_t<float, py::array::c_style | py::array::forcecast> input) {
        float* input_ptr = (float*)input.data();

        myPlaybackData = std::vector<std::vector<float>>(input.shape(0), std::vector<float>(input.shape(1)));

        for (int y = 0; y < input.shape(1); y++) {
            for (int x = 0; x < input.shape(0); x++) {
                myPlaybackData[x][y] = input_ptr[x * input.shape(1) + y];
            }
        }
    }

private:

    std::vector<std::vector<float>> myPlaybackData;

};

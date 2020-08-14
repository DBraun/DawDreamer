#pragma once

#include "ProcessorBase.h"

class PlaybackProcessor : public ProcessorBase
{
public:
    PlaybackProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData) : ProcessorBase{ newUniqueName }
    {
        myPlaybackData = inputData;
    }

    void
    prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        myPlaybackIndex = 0;
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
        for (size_t i = 0; i < buffer.getNumSamples() && myPlaybackIndex < myLength; i++)
        {
            for (size_t chan = 0; chan < buffer.getNumChannels(); chan++) {
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

private:

    std::vector<std::vector<float>> myPlaybackData;
    uint32_t myPlaybackIndex = 0;

};
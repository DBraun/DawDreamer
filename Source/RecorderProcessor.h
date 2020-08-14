#pragma once

#include "ProcessorBase.h"

//==============================================================================
class RecorderProcessor : public ProcessorBase
{
public:
    RecorderProcessor(std::string newUniqueName) : ProcessorBase{ newUniqueName }
    {
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock)
    {
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
    {
        myWriteIndex = 0;
        const auto readptrs = buffer.getArrayOfReadPointers();
        const int numberChannels = buffer.getNumChannels();

        int i = 0;

        for (i = 0; i < buffer.getNumSamples(); ++i)
        {
            for (int chan = 0; chan < numberChannels; chan++) {
                // Write the sample to our recorded history for the right channel.
                myRecordedSamples[chan][myWriteIndex] = readptrs[chan][i];
            }
            myWriteIndex++;
        }

        // Write zeros for the rest
        for (; i < myLength; ++i)
        {
            for (int chan = 0; chan < numberChannels; chan++) {
                // Write the sample to our recorded history for the right channel.
                myRecordedSamples[chan][myWriteIndex] = 0;
            }
            myWriteIndex++;
        }

    }

    void reset()
    {
    }

    const juce::String getName() const { return "RecorderProcessor"; }

    void reserveSamples(size_t numChannels, size_t numSamples) {
        myRecordedSamples.clear();
        myRecordedSamples = std::vector<std::vector<float>>(numChannels, std::vector<float>(numSamples));
        myLength = numSamples;
    }

    void clearSamples() {
        myRecordedSamples.clear();
    }

public:
    std::vector<std::vector<float>> getRecordedSamples() {
        return myRecordedSamples;
    }

private:
    int myWriteIndex = 0;
    size_t myLength = 0;
    std::vector<std::vector<float>> myRecordedSamples;
};
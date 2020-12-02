#pragma once

#include "ProcessorBase.h"

//==============================================================================
class RecorderProcessor : public ProcessorBase
{
public:
    RecorderProcessor(std::string newUniqueName) : ProcessorBase{ newUniqueName }
    {

    }

    void prepareToPlay(double, int)
    {
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
    {
        const auto readptrs = buffer.getArrayOfReadPointers();
        const int numberChannels = buffer.getNumChannels();

        int i = 0;

        for (i = 0; i < buffer.getNumSamples(); ++i)
        {
            for (int chan = 0; chan < numberChannels; chan++) {
                // Write the sample to the engine's history for the correct channel.
                (*myEngineBuffer)[chan][myWriteIndex] = readptrs[chan][i];
            }
            myWriteIndex++;
        }
    }

    void reset()
    {
        myWriteIndex = 0;
    }

    const juce::String getName() const { return "RecorderProcessor"; }

    void setRecorderBuffer(std::vector<std::vector<float>>* engineBuffer) {
        myEngineBuffer = engineBuffer;
    }

private:
    int myWriteIndex = 0;
    std::vector<std::vector<float>>* myEngineBuffer = nullptr;
};
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
        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        const int numberChannels = buffer.getNumChannels();

        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            for (int chan = 0; chan < numberChannels; chan++) {
                // Write the sample to the engine's history for the correct channel.                
                (*myEngineBuffer)[chan][posInfo.timeInSamples+i] = buffer.getSample(chan, i);
            }
        }
    }

    void reset(){}

    const juce::String getName() const { return "RecorderProcessor"; }

    void setRecorderBuffer(std::vector<std::vector<float>>* engineBuffer) {
        myEngineBuffer = engineBuffer;
    }

private:
    std::vector<std::vector<float>>* myEngineBuffer = nullptr;
};
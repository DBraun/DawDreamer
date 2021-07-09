#pragma once

#include "ProcessorBase.h"

//==============================================================================
class RecorderProcessor : public ProcessorBase
{
public:
    RecorderProcessor(std::string newUniqueName) : ProcessorBase{ newUniqueName }
    {
        m_recordEnable = true;
    }

    void prepareToPlay(double, int)
    {
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer)
    {
        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    void reset(){}

    const juce::String getName() const { return "RecorderProcessor"; }
};
#pragma once

#include "ProcessorBase.h"

class OscillatorProcessor : public ProcessorBase
{
public:

    OscillatorProcessor(std::string newUniqueName, float freq = 440.0f) : ProcessorBase( newUniqueName )
    {
        myFreq = freq;
        myOscillator.setFrequency(freq);
        myOscillator.initialise([](float x) { return std::sin(x); });
    }

    OscillatorProcessor():ProcessorBase( "osc1" )
    {
        myFreq = 440.f;
        myOscillator.setFrequency(myFreq);
        myOscillator.initialise([](float x) { return std::sin(x); });
    }

    void
    prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock) };
        myOscillator.prepare(spec);
    }

    void
    processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myOscillator.process(context);
    }

    void
    reset()
    {
        myOscillator.reset();
    }

    const juce::String getName() const { return "OscillatorProcessor"; }

    float myFreq;

private:
    juce::dsp::Oscillator<float> myOscillator;
};
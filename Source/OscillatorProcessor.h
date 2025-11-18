#pragma once

#include "custom_nanobind_wrappers.h"
#include "ProcessorBase.h"

class OscillatorProcessor : public ProcessorBase
{
  public:
    OscillatorProcessor(std::string newUniqueName, float freq = 440.0f)
        : ProcessorBase(newUniqueName)
    {
        myFreq = freq;
        myOscillator.setFrequency(freq);
        myOscillator.initialise([](float x) { return std::sin(x); });
        setMainBusInputsAndOutputs(0, 2);
    }

    OscillatorProcessor() : ProcessorBase("osc1")
    {
        myFreq = 440.f;
        myOscillator.setFrequency(myFreq);
        myOscillator.initialise([](float x) { return std::sin(x); });
        setMainBusInputsAndOutputs(0, 2);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock)};
        myOscillator.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myOscillator.process(context);
        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    void reset() override
    {
        myOscillator.reset();
        ProcessorBase::reset();
    }

    const juce::String getName() const override { return "OscillatorProcessor"; }

    nb::dict getPickleState()
    {
        nb::dict state;
        state["unique_name"] = getUniqueName();
        state["frequency"] = myFreq;
        return state;
    }

    void setPickleState(nb::dict state)
    {
        std::string name = nb::cast<std::string>(state["unique_name"]);
        float freq = nb::cast<float>(state["frequency"]);
        new (this) OscillatorProcessor(name, freq);
    }

    float myFreq;

  private:
    juce::dsp::Oscillator<float> myOscillator;
};

#pragma once

#include "ProcessorBase.h"

class DelayProcessor : public ProcessorBase
{
public:
    DelayProcessor(std::string newUniqueName, std::string rule, float delaySize, float wet) :
        ProcessorBase(createParameterLayout, newUniqueName), myRule{ rule }{

        setDelay(delaySize);
        setWet(wet);

        myDelaySize = myParameters.getRawParameterValue("delay");
        myWetLevel = myParameters.getRawParameterValue("wet_level");
        setMainBusInputsAndOutputs(2, 2);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        mySampleRate = sampleRate;
        
        AudioPlayHead::PositionInfo posInfo;
        automateParameters(posInfo, 1); // do this to give a valid state to the filter.

        initDelay();
        
        const int numChannels = 2;
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), static_cast<juce::uint32> (numChannels) };
        myDelay.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override {
        //auto posInfo = getPlayHead()->getPosition();

        delayBuffer.makeCopyOf(buffer);
        juce::dsp::AudioBlock<float> block(delayBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myDelay.process(context);

        buffer.applyGain(1.f - *myWetLevel);

        for (int chan = 0; chan < buffer.getNumChannels(); chan++)
        {
            buffer.addFrom(chan, 0, delayBuffer, chan, 0, buffer.getNumSamples(), *myWetLevel);
        }
        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    void automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) override {
        *myWetLevel = getAutomationVal("wet_level", posInfo);
        *myDelaySize = getAutomationVal("delay", posInfo);
        updateParameters();
    }

    void reset() override {
        myDelay.reset();
        ProcessorBase::reset();
    };

    const juce::String getName() const override { return "DelayProcessor"; };

    void setDelay(float newDelaySize) { setAutomationVal("delay", newDelaySize); }
    float getDelay() { return getAutomationAtZero("delay"); }

    void setWet(float newWet) { setAutomationVal("wet_level", newWet); }
    float getWet() { return getAutomationAtZero("wet_level"); }


private:

    double mySampleRate = 44100.;

    std::atomic<float>* myDelaySize; // delay in milliseconds

    // todo: very inconvenient that Linear is a struct and not an enum. It makes changing the type later difficult.
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> myDelay;

    std::atomic<float>* myWetLevel; // 0 means use all of original signal and none of the "wet" delayed signal. 1. is the opposite.
    std::string myRule; // todo: ugly because storing an enum would probably be better.

    juce::AudioSampleBuffer delayBuffer;

    void updateParameters() {
        // convert milliseconds to samples
        myDelay.setDelay((*myDelaySize)*.001f*(float)mySampleRate);
    }

    void initDelay() {
        // todo: NB: DelayLine seems to need to be initialized with the maximum delay size you'll ever ask for before re-initializing.
        // todo: What's a more flexible solution so that through parameter automation we might be able to increase the delay over time?

        int delayInSamples = int(*myDelaySize * .001 * mySampleRate);
        if (!myRule.compare("linear"))
        {
            myDelay = juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>(delayInSamples);
        }
        else {
            // todo: eventually support more times but for now keep defaulting to Linear
            myDelay = juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>(delayInSamples);
        }
    }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;

        params.add(std::make_unique<AutomateParameterFloat>("wet_level", "wet_level", NormalisableRange<float>(0.f, 1.f), .1f));
        params.add(std::make_unique<AutomateParameterFloat>("delay", "delay", NormalisableRange<float>(0.f, 44100.f), 10.f)); // todo: proper max value
        return params;
    }

};

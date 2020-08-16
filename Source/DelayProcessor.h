#pragma once

#include "ProcessorBase.h"

class DelayProcessor : public ProcessorBase
{
public:
    DelayProcessor(std::string newUniqueName, std::string rule, float delaySize, float wet) :
        ProcessorBase(newUniqueName), myRule{ rule }, myDelaySize{ delaySize }, myWet{ wet } {
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        mySampleRate = sampleRate;

        initDelay();
        
        const int numChannels = 2;
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), numChannels };
        myDelay.prepare(spec);

        updateParameters();
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) {
        delayBuffer.makeCopyOf(buffer);
        juce::dsp::AudioBlock<float> block(delayBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myDelay.process(context);

        buffer.applyGain(1.f - myWet);

        for (size_t chan = 0; chan < buffer.getNumChannels(); chan++)
        {
            buffer.addFrom(chan, 0, delayBuffer, chan, 0, buffer.getNumSamples(), myWet);
        }
    }

    void reset() {
        myDelay.reset();
    };

    const juce::String getName() { return "DelayProcessor"; };

    void setDelay(float newDelaySize) { myDelaySize = newDelaySize; updateParameters(); }
    float getDelay() { return myDelaySize; }

    void setWet(float newWet) { myWet = newWet; updateParameters(); }
    float getWet() { return myWet; }


private:

    double mySampleRate = 44100.;

    float myDelaySize; // delay in milliseconds

    // todo: very inconvenient that Linear is a struct and not an enum. It makes changing the type later difficult.
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> myDelay;

    float myWet = .1; // 0 means use all of original signal and none of the "wet" delayed signal. 1. is the opposite.
    std::string myRule; // todo: ugly because storing an enum would probably be better.

    juce::AudioSampleBuffer delayBuffer;

    void updateParameters() {
        // convert milliseconds to samples
        myDelay.setDelay(myDelaySize*.001*mySampleRate);
    }

    void initDelay() {
        // todo: NB: DelayLine seems to need to be initialized with the maximum delay size you'll ever ask for before re-initializing.
        // todo: What's a more flexible solution so that through parameter automation we might be able to increase the delay over time?

        int delayInSamples = int(myDelaySize * .001 * mySampleRate);
        if (!myRule.compare("linear"))
        {
            myDelay = juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>(delayInSamples);
        }
        else {
            // todo: eventually support more times but for now keep defaulting to Linear
            myDelay = juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>(delayInSamples);
        }
    }
};
#pragma once

#include "ProcessorBase.h"

class ReverbProcessor : public ProcessorBase
{
public:
    ReverbProcessor(std::string newUniqueName, float roomSize=0.5f, float damping=0.5f, float wetLevel=0.33f, float dryLevel=0.4f, float width=1.0f) :
        ProcessorBase(newUniqueName),
        myRoomSize{ roomSize }, myDamping{ damping }, myWetLevel{ wetLevel }, myDryLevel{ dryLevel }, myWidth{ width } {

        juce::dsp::Reverb::Parameters params;

        params.damping = myDamping;
        params.dryLevel = myDryLevel;
        params.roomSize = myRoomSize;
        params.wetLevel = myWetLevel;
        params.width = myWidth;

        myReverb.setParameters(params);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock) };
        myReverb.prepare(spec);

        juce::dsp::Reverb::Parameters params;
        params.damping = myDamping;
        params.dryLevel = myDryLevel;
        params.roomSize = myRoomSize;
        params.wetLevel = myWetLevel;
        params.width = myWidth;

        myReverb.setParameters(params);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myReverb.process(context);
    }

    void reset() {
        myReverb.reset();
    };

    const juce::String getName() { return "ReverbProcessor"; };

private:
    juce::dsp::Reverb myReverb;
    float myRoomSize;
    float myDamping;
    float myWetLevel;
    float myDryLevel;
    float myWidth;
};
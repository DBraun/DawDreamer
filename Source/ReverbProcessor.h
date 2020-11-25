#pragma once

#include "ProcessorBase.h"

class ReverbProcessor : public ProcessorBase
{
public:
    ReverbProcessor(std::string newUniqueName, float roomSize=0.5f, float damping=0.5f, float wetLevel=0.33f, float dryLevel=0.4f, float width=1.0f) :
        ProcessorBase(createParameterLayout, newUniqueName) {

        setRoomSize(roomSize);
        setDamping(damping);
        setDryLevel(dryLevel);
        setWetLevel(wetLevel);
        setWidth(width);

        myRoomSize = myParameters.getRawParameterValue("room_size");
        myDamping = myParameters.getRawParameterValue("damping");
        myDryLevel = myParameters.getRawParameterValue("dry_level");
        myWetLevel = myParameters.getRawParameterValue("wet_level");
        myWidth = myParameters.getRawParameterValue("width");

    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        automateParameters(myPlayheadIndex); // do this to give a valid state to the filter.
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock) };
        myReverb.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) {

        automateParameters(myPlayheadIndex);

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myReverb.process(context);

        myPlayheadIndex += buffer.getNumSamples();
    }

    void automateParameters(size_t index) {

        *myRoomSize = ((AutomateParameterFloat*)myParameters.getParameter("room_size"))->sample(index);
        *myDamping = ((AutomateParameterFloat*)myParameters.getParameter("damping"))->sample(index);
        *myDryLevel = ((AutomateParameterFloat*)myParameters.getParameter("dry_level"))->sample(index);
        *myWetLevel = ((AutomateParameterFloat*)myParameters.getParameter("wet_level"))->sample(index);
        *myWidth = ((AutomateParameterFloat*)myParameters.getParameter("width"))->sample(index);

        updateParameters();
    }

    void reset() {
        myReverb.reset();
        myPlayheadIndex = 0;
    };

    const juce::String getName() { return "ReverbProcessor"; };

    void setRoomSize(float roomSize) { setParameter("room_size", roomSize); }
    float getRoomSize() { return *myRoomSize; }

    void setDamping(float damping) { setParameter("damping", damping); }
    float getDamping() { return *myDamping; }

    void setWetLevel(float wetLevel) { setParameter("wet_level", wetLevel); }
    float getWetLevel() { return *myWetLevel; }

    void setDryLevel(float dryLevel) { setParameter("dry_level", dryLevel); }
    float getDryLevel() { return *myDryLevel; }

    void setWidth(float width) { setParameter("width", width); }
    float getWidth() { return *myWidth; }


private:
    juce::dsp::Reverb myReverb;
    std::atomic<float>* myRoomSize;
    std::atomic<float>* myDamping;
    std::atomic<float>* myWetLevel;
    std::atomic<float>* myDryLevel;
    std::atomic<float>* myWidth;

    void updateParameters() {
        juce::dsp::Reverb::Parameters params;
        params.damping = *myDamping;
        params.dryLevel = *myDryLevel;
        params.roomSize = *myRoomSize;
        params.wetLevel = *myWetLevel;
        params.width = *myWidth;
        myReverb.setParameters(params);
    }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;

        params.add(std::make_unique<AutomateParameterFloat>("room_size", "room_size", NormalisableRange<float>(0.f, 1.f), 0.f));
        params.add(std::make_unique<AutomateParameterFloat>("damping", "damping", NormalisableRange<float>(0.f, 1.f), 0.f));
        params.add(std::make_unique<AutomateParameterFloat>("wet_level", "wet_level", NormalisableRange<float>(0.f, 1.f), 0.f));
        params.add(std::make_unique<AutomateParameterFloat>("dry_level", "dry_level", NormalisableRange<float>(0.f, 1.f), 0.f));
        params.add(std::make_unique<AutomateParameterFloat>("width", "width", NormalisableRange<float>(0.f, 1.f), 0.f));
        return params;
    }

};
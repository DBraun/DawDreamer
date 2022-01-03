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
        setMainBusInputsAndOutputs(2, 2);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        automateParameters(); // do this to give a valid state to the filter.
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock) };
        myReverb.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) {

        automateParameters();

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myReverb.process(context);
        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    void automateParameters() {

        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        *myRoomSize = getAutomationVal("room_size", posInfo.timeInSamples);
        *myDamping = getAutomationVal("damping", posInfo.timeInSamples);
        *myDryLevel = getAutomationVal("dry_level", posInfo.timeInSamples);
        *myWetLevel = getAutomationVal("wet_level", posInfo.timeInSamples);
        *myWidth = getAutomationVal("width", posInfo.timeInSamples);

        updateParameters();
    }

    void reset() {
        myReverb.reset();
    };

    const juce::String getName() { return "ReverbProcessor"; };

    void setRoomSize(float roomSize) { setAutomationVal("room_size", roomSize); }
    float getRoomSize() { return getAutomationVal("room_size", 0); }

    void setDamping(float damping) { setAutomationVal("damping", damping); }
    float getDamping() { return getAutomationVal("damping", 0); }

    void setWetLevel(float wetLevel) { setAutomationVal("wet_level", wetLevel); }
    float getWetLevel() { return getAutomationVal("wet_level", 0); }

    void setDryLevel(float dryLevel) { setAutomationVal("dry_level", dryLevel); }
    float getDryLevel() { return getAutomationVal("dry_level", 0);  }

    void setWidth(float width) { setAutomationVal("width", width); }
    float getWidth() { return getAutomationVal("width", 0);  }


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

#pragma once

#include "custom_nanobind_wrappers.h"
#include "ProcessorBase.h"

class ReverbProcessor : public ProcessorBase
{
  public:
    ReverbProcessor(std::string newUniqueName, float roomSize = 0.5f, float damping = 0.5f,
                    float wetLevel = 0.33f, float dryLevel = 0.4f, float width = 1.0f)
        : ProcessorBase(newUniqueName)
    {
        createParameterLayout();

        setMainBusInputsAndOutputs(2, 2);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        AudioPlayHead::PositionInfo posInfo;
        posInfo.setTimeInSeconds(0.);
        posInfo.setTimeInSamples(0.);
        automateParameters(posInfo,
                           1); // do this to give a valid state to the filter.
        juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock)};
        myReverb.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override
    {
        auto posInfo = getPlayHead()->getPosition();

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myReverb.process(context);
        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    void automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) override
    {
        juce::dsp::Reverb::Parameters params;
        params.damping = getAutomationVal("damping", posInfo);
        params.dryLevel = getAutomationVal("dry_level", posInfo);
        params.roomSize = getAutomationVal("room_size", posInfo);
        params.wetLevel = getAutomationVal("wet_level", posInfo);
        params.width = getAutomationVal("width", posInfo);
        myReverb.setParameters(params);
    }

    void reset() override
    {
        myReverb.reset();
        ProcessorBase::reset();
    };

    const juce::String getName() const override { return "ReverbProcessor"; };

    void setRoomSize(float roomSize) { setAutomationVal("room_size", roomSize); }
    float getRoomSize() { return getAutomationAtZero("room_size"); }

    void setDamping(float damping) { setAutomationVal("damping", damping); }
    float getDamping() { return getAutomationAtZero("damping"); }

    void setWetLevel(float wetLevel) { setAutomationVal("wet_level", wetLevel); }
    float getWetLevel() { return getAutomationAtZero("wet_level"); }

    void setDryLevel(float dryLevel) { setAutomationVal("dry_level", dryLevel); }
    float getDryLevel() { return getAutomationAtZero("dry_level"); }

    void setWidth(float width) { setAutomationVal("width", width); }
    float getWidth() { return getAutomationAtZero("width"); }

    nb::dict getPickleState()
    {
        nb::dict state;
        state["unique_name"] = getUniqueName();
        state["room_size"] = getRoomSize();
        state["damping"] = getDamping();
        state["wet_level"] = getWetLevel();
        state["dry_level"] = getDryLevel();
        state["width"] = getWidth();
        return state;
    }

    void setPickleState(nb::dict state)
    {
        std::string name = nb::cast<std::string>(state["unique_name"]);
        float roomSize = nb::cast<float>(state["room_size"]);
        float damping = nb::cast<float>(state["damping"]);
        float wetLevel = nb::cast<float>(state["wet_level"]);
        float dryLevel = nb::cast<float>(state["dry_level"]);
        float width = nb::cast<float>(state["width"]);
        new (this) ReverbProcessor(name, roomSize, damping, wetLevel, dryLevel, width);
    }

    void createParameterLayout()
    {
        juce::AudioProcessorParameterGroup group;

        group.addChild(std::make_unique<AutomateParameterFloat>(
            "room_size", "room_size", NormalisableRange<float>(0.f, 1.f), 0.f));
        group.addChild(std::make_unique<AutomateParameterFloat>(
            "damping", "damping", NormalisableRange<float>(0.f, 1.f), 0.f));
        group.addChild(std::make_unique<AutomateParameterFloat>(
            "wet_level", "wet_level", NormalisableRange<float>(0.f, 1.f), 0.f));
        group.addChild(std::make_unique<AutomateParameterFloat>(
            "dry_level", "dry_level", NormalisableRange<float>(0.f, 1.f), 0.f));
        group.addChild(std::make_unique<AutomateParameterFloat>(
            "width", "width", NormalisableRange<float>(0.f, 1.f), 0.f));

        this->setParameterTree(std::move(group));

        int i = 0;
        for (auto* parameter : this->getParameters())
        {
            // give it a valid single sample of automation.
            ProcessorBase::setAutomationValByIndex(i, parameter->getValue());
            i++;
        }
    }

  private:
    juce::dsp::Reverb myReverb;
};

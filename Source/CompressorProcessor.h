#pragma once

#include "custom_nanobind_wrappers.h"
#include "ProcessorBase.h"

class CompressorProcessor : public ProcessorBase
{
  public:
    CompressorProcessor(std::string newUniqueName, float threshold, float ratio, float attack,
                        float release)
        : ProcessorBase(newUniqueName)
    {
        createParameterLayout();
        setThreshold(threshold);
        setRatio(ratio);
        setAttack(attack);
        setRelease(release);
        setMainBusInputsAndOutputs(2, 2);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
        myCompressor.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override
    {
        // auto posInfo = getPlayHead()->getPosition();

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myCompressor.process(context);
        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    void automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) override
    {
        myCompressor.setThreshold(getAutomationVal("threshold", posInfo));
        myCompressor.setRatio(getAutomationVal("ratio", posInfo));
        myCompressor.setAttack(getAutomationVal("attack", posInfo));
        myCompressor.setRelease(getAutomationVal("release", posInfo));
    }

    void reset() override
    {
        myCompressor.reset();
        ProcessorBase::reset();
    };

    const juce::String getName() const override { return "CompressorProcessor"; };

    void setThreshold(float threshold) { setAutomationVal("threshold", threshold); }
    float getThreshold() { return getAutomationAtZero("threshold"); }

    void setRatio(float ratio) { setAutomationVal("ratio", ratio); }
    float getRatio() { return getAutomationAtZero("ratio"); }

    void setAttack(float attack) { setAutomationVal("attack", attack); }
    float getAttack() { return getAutomationAtZero("attack"); }

    void setRelease(float release) { setAutomationVal("release", release); }
    float getRelease() { return getAutomationAtZero("release"); }

    nb::dict getPickleState()
    {
        nb::dict state;
        state["unique_name"] = getUniqueName();
        state["threshold"] = getThreshold();
        state["ratio"] = getRatio();
        state["attack"] = getAttack();
        state["release"] = getRelease();
        return state;
    }

    void setPickleState(nb::dict state)
    {
        std::string name = nb::cast<std::string>(state["unique_name"]);
        float threshold = nb::cast<float>(state["threshold"]);
        float ratio = nb::cast<float>(state["ratio"]);
        float attack = nb::cast<float>(state["attack"]);
        float release = nb::cast<float>(state["release"]);
        new (this) CompressorProcessor(name, threshold, ratio, attack, release);
    }

  private:
    juce::dsp::Compressor<float> myCompressor;

  public:
    void createParameterLayout()
    {
        juce::AudioProcessorParameterGroup group;

        group.addChild(std::make_unique<AutomateParameterFloat>(
            "threshold", "threshold", NormalisableRange<float>(-400.f, 200.f), 0.f));
        group.addChild(std::make_unique<AutomateParameterFloat>(
            "ratio", "ratio", NormalisableRange<float>(1.f, std::numeric_limits<float>::max()),
            0.f));
        group.addChild(std::make_unique<AutomateParameterFloat>(
            "attack", "attack", NormalisableRange<float>(0.f, 10000.f), 0.f));
        group.addChild(std::make_unique<AutomateParameterFloat>(
            "release", "release", NormalisableRange<float>(0.f, 10000.f), 0.f));

        this->setParameterTree(std::move(group));

        int i = 0;
        for (auto* parameter : this->getParameters())
        {
            // give it a valid single sample of automation.
            ProcessorBase::setAutomationValByIndex(i, parameter->getValue());
            i++;
        }
    }
};

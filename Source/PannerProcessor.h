#pragma once

#include "custom_nanobind_wrappers.h"
#include "ProcessorBase.h"

class PannerProcessor : public ProcessorBase
{
  public:
    PannerProcessor(std::string newUniqueName, std::string rule, float panVal)
        : ProcessorBase(newUniqueName)
    {
        createParameterLayout();
        myRule = stringToRule(rule);

        setMainBusInputsAndOutputs(2, 2);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        const int numChannels = 2;
        AudioPlayHead::PositionInfo posInfo;
        posInfo.setTimeInSamples(0.);
        posInfo.setTimeInSeconds(0.);
        automateParameters(posInfo,
                           1); // do this to give a valid state to the filter.
        juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock),
                                    static_cast<juce::uint32>(numChannels)};
        myPanner.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override
    {
        //        auto posInfo = getPlayHead()->getPosition();

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myPanner.process(context);
        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    void automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) override
    {
        myPanner.setRule(myRule);
        myPanner.setPan(getAutomationVal("pan", posInfo));
    }

    void reset() override
    {
        myPanner.reset();
        ProcessorBase::reset();
    };

    const juce::String getName() const override { return "PannerProcessor"; };

    void setPan(float newPanVal) { setAutomationVal("pan", newPanVal); }
    float getPan() const { return getAutomationAtZero("pan"); }

    void setRule(std::string newRule)
    {
        myRule = stringToRule(newRule);
        myPanner.setRule(myRule);
    }
    std::string getRule() const { return ruleToString(myRule); }

    nb::dict getPickleState()
    {
        nb::dict state;
        state["unique_name"] = getUniqueName();
        state["rule"] = getRule();
        state["pan"] = getPan();
        return state;
    }

    void setPickleState(nb::dict state)
    {
        std::string name = nb::cast<std::string>(state["unique_name"]);
        std::string rule = nb::cast<std::string>(state["rule"]);
        float pan = nb::cast<float>(state["pan"]);
        new (this) PannerProcessor(name, rule, pan);
    }

    void createParameterLayout()
    {
        juce::AudioProcessorParameterGroup group;

        group.addChild(std::make_unique<AutomateParameterFloat>(
            "pan", "pan", NormalisableRange<float>(-1.f, 1.f), 0.f));

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
    juce::dsp::Panner<float> myPanner;
    juce::dsp::PannerRule myRule;

    std::string ruleToString(juce::dsp::PannerRule rule) const
    {
        switch (rule)
        {
        case juce::dsp::PannerRule::linear:
            return "linear";
            break;
        case juce::dsp::PannerRule::balanced:
            return "balanced";
            break;
        case juce::dsp::PannerRule::sin3dB:
            return "sin3dB";
            break;
        case juce::dsp::PannerRule::sin4p5dB:
            return "sin4p5dB";
            break;
        case juce::dsp::PannerRule::sin6dB:
            return "sin6dB";
            break;
        case juce::dsp::PannerRule::squareRoot3dB:
            return "squareRoot3dB";
            break;
        case juce::dsp::PannerRule::squareRoot4p5dB:
            return "squareRoot4p5dB";
            break;
        default:
            return "invalid";
            break;
        }
    }

    juce::dsp::PannerRule stringToRule(std::string s)
    {
        if (!s.compare("linear"))
        {
            return juce::dsp::PannerRule::linear;
        }
        else if (!s.compare("balanced"))
        {
            return juce::dsp::PannerRule::balanced;
        }
        else if (!s.compare("sin3dB"))
        {
            return juce::dsp::PannerRule::sin3dB;
        }
        else if (!s.compare("sin4p5dB"))
        {
            return juce::dsp::PannerRule::sin4p5dB;
        }
        else if (!s.compare("sin6dB"))
        {
            return juce::dsp::PannerRule::sin6dB;
        }
        else if (!s.compare("squareRoot3dB"))
        {
            return juce::dsp::PannerRule::squareRoot3dB;
        }
        else if (!s.compare("squareRoot4p5dB"))
        {
            return juce::dsp::PannerRule::squareRoot4p5dB;
        }
        else
        {
            // todo: throw error
            return juce::dsp::PannerRule::balanced;
        }
    }
};

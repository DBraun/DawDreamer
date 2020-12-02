#pragma once

#include "ProcessorBase.h"

class PannerProcessor : public ProcessorBase
{
public:
    PannerProcessor(std::string newUniqueName, std::string rule, float panVal) :
        ProcessorBase(createParameterLayout, newUniqueName) {
        myRule = stringToRule(rule);

        setPan(panVal);

        myPan = myParameters.getRawParameterValue("pan");

    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        const int numChannels = 2;
        automateParameters(myPlayheadIndex); // do this to give a valid state to the filter.
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), static_cast<juce::uint32> (numChannels) };
        myPanner.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) {

        automateParameters(myPlayheadIndex);

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myPanner.process(context);

        myPlayheadIndex += buffer.getNumSamples();
    }

    void automateParameters(size_t index) {

        *myPan = getAutomationVal("pan", index);
        updateParameters();
    }

    void reset() {
        myPanner.reset();
        myPlayheadIndex = 0;
    };

    const juce::String getName() { return "PannerProcessor"; };

    void setPan(float newPanVal) { setAutomationVal("pan", newPanVal); }
    float getPan() { return getAutomationVal("pan", 0);
    }

    void setRule(std::string newRule) {
        myRule = stringToRule(newRule);
        updateParameters();
    }
    std::string getRule() {
        return ruleToString(myRule);
    }

private:
    juce::dsp::Panner<float> myPanner;
    juce::dsp::PannerRule myRule;
    std::atomic<float>* myPan;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;

        params.add(std::make_unique<AutomateParameterFloat>("pan", "pan", NormalisableRange<float>(-1.f, 1.f), 0.f));
        return params;
    }

    void updateParameters() {
        myPanner.setRule(myRule);
        myPanner.setPan(*myPan);
    }

    std::string ruleToString(juce::dsp::PannerRule rule) {
        switch (rule) {
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

    juce::dsp::PannerRule stringToRule(std::string s) {
        if (!s.compare("linear")) {
            return juce::dsp::PannerRule::linear;
        }
        else if (!s.compare("balanced")) {
            return juce::dsp::PannerRule::balanced;
        }
        else if (!s.compare("sin3dB")) {
            return juce::dsp::PannerRule::sin3dB;
        }
        else if (!s.compare("sin4p5dB")) {
            return juce::dsp::PannerRule::sin4p5dB;
        }
        else if (!s.compare("sin6dB")) {
            return juce::dsp::PannerRule::sin6dB;
        }
        else if (!s.compare("squareRoot3dB")) {
            return juce::dsp::PannerRule::squareRoot3dB;
        }
        else if (!s.compare("squareRoot4p5dB")) {
            return juce::dsp::PannerRule::squareRoot4p5dB;
        }
        else {
            // todo: throw error
            return juce::dsp::PannerRule::balanced;
        }
    }
};
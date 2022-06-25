#pragma once

#include "ProcessorBase.h"

class CompressorProcessor : public ProcessorBase
{
public:
    CompressorProcessor(std::string newUniqueName, float threshold, float ratio, float attack, float release) :
        ProcessorBase(createParameterLayout, newUniqueName) {

        setThreshold(threshold);
        setRatio(ratio);
        setAttack(attack);
        setRelease(release);

        myThreshold = myParameters.getRawParameterValue("threshold");
        myRatio = myParameters.getRawParameterValue("ratio");
        myAttack = myParameters.getRawParameterValue("attack");
        myRelease = myParameters.getRawParameterValue("release");

        updateParameters();
        setMainBusInputsAndOutputs(2, 2);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        myCompressor.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) {

        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);
        
        automateParameters(buffer.getNumSamples());
        recordAutomation(posInfo, buffer.getNumSamples());

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myCompressor.process(context);
        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    void automateParameters(int numSamples) {

        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        *myThreshold = getAutomationVal("threshold", posInfo);
        *myRatio = getAutomationVal("ratio", posInfo);
        *myAttack = getAutomationVal("attack", posInfo);
        *myRelease = getAutomationVal("release", posInfo);
        updateParameters();
    }

    void reset() {
        myCompressor.reset();
        ProcessorBase::reset();
    };

    const juce::String getName() { return "CompressorProcessor"; };

    void setThreshold(float threshold) { setAutomationVal("threshold", threshold); }
    float getThreshold() { AudioPlayHead::CurrentPositionInfo posInfo; return getAutomationVal("threshold", posInfo); }

    void setRatio(float ratio) { setAutomationVal("ratio", ratio); }
    float getRatio() { AudioPlayHead::CurrentPositionInfo posInfo; return getAutomationVal("ratio", posInfo); }

    void setAttack(float attack) { setAutomationVal("attack", attack); }
    float getAttack() { AudioPlayHead::CurrentPositionInfo posInfo; return getAutomationVal("attack", posInfo); }

    void setRelease(float release) { setAutomationVal("release", release); }
    float getRelease() { AudioPlayHead::CurrentPositionInfo posInfo; return getAutomationVal("release", posInfo); }


private:
    juce::dsp::Compressor<float> myCompressor;
    std::atomic<float>* myThreshold;
    std::atomic<float>* myRatio;
    std::atomic<float>* myAttack;
    std::atomic<float>* myRelease;

    void updateParameters() {
        myCompressor.setThreshold(*myThreshold);
        myCompressor.setRatio(*myRatio);
        myCompressor.setAttack(*myAttack);
        myCompressor.setRelease(*myRelease);
    }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;

        params.add(std::make_unique<AutomateParameterFloat>("threshold", "threshold", NormalisableRange<float>(-400.f, 200.f), 0.f));
        params.add(std::make_unique<AutomateParameterFloat>("ratio", "ratio", NormalisableRange<float>(1.f, std::numeric_limits<float>::max()), 0.f));
        params.add(std::make_unique<AutomateParameterFloat>("attack", "attack", NormalisableRange<float>(0.f, 10000.f), 0.f));
        params.add(std::make_unique<AutomateParameterFloat>("release", "release", NormalisableRange<float>(0.f, 10000.f), 0.f));
        return params;
    }

};

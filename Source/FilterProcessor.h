#pragma once

#include "ProcessorBase.h"

enum class FILTER_FilterFormat
{
    Invalid = -1,
    LOW_PASS,
    BAND_PASS,
    HIGH_PASS,
    LOW_SHELF,
    HIGH_SHELF,
    NOTCH
};

class FilterProcessor : public ProcessorBase
{
public:
    FilterProcessor(std::string newUniqueName, std::string mode, float freq, float q, float gain);

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override;
    
    void automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) override;

    void reset() override;

    const juce::String getName() const override { return "FilterProcessor"; }

    void setMode(std::string mode);
    std::string getMode();

    void setFrequency(float freq);
    float getFrequency();

    void setQ(float q);
    float getQ();

    void setGain(float gain);
    float getGain();

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> myFilter;
    FILTER_FilterFormat myMode;

    double mySampleRate;
    int mySamplesPerBlock;

    std::string modeToString(FILTER_FilterFormat mode);
    FILTER_FilterFormat stringToMode(std::string s);

public:
    void createParameterLayout() {
      juce::AudioProcessorParameterGroup group;

      group.addChild(std::make_unique<AutomateParameterFloat>(
          "freq", "freq", NormalisableRange<float>(0.f, 22050.f), 1000.f));
      group.addChild(std::make_unique<AutomateParameterFloat>(
          "q", "q", NormalisableRange<float>(0.01f, 10.f), 0.707107f));
      group.addChild(std::make_unique<AutomateParameterFloat>(
          "gain", "gain", NormalisableRange<float>(-100.f, 30.f), 1.f));

      this->setParameterTree(std::move(group));

	  this->updateHostDisplay();

      int i = 0;
      for (auto* parameter : this->getParameters()) {
        // give it a valid single sample of automation.
        ProcessorBase::setAutomationValByIndex(i, parameter->getValue());
        i++;
      }
    }

};

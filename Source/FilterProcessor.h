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

    void prepareToPlay(double sampleRate, int samplesPerBlock);

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&);

    void reset();

    const juce::String getName();

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
    std::atomic<float>* myFreq;
    std::atomic<float>* myQ;
    std::atomic<float>* myGain;

    double mySampleRate;
    int mySamplesPerBlock;

    std::string modeToString(FILTER_FilterFormat mode);
    FILTER_FilterFormat stringToMode(std::string s);

    void automateParameters(int numSamples);

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;

        params.add(std::make_unique<AutomateParameterFloat>("freq", "freq", NormalisableRange<float>(0.f, 22050.f), 1000.f));
        params.add(std::make_unique<AutomateParameterFloat>("q", "q", NormalisableRange<float>(0.01f, 10.f), 0.707107f));
        params.add(std::make_unique<AutomateParameterFloat>("gain", "gain", NormalisableRange<float>(-100.f, 30.f), 1.f));
        return params;
    }
};

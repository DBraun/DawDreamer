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
    FilterProcessor(std::string newUniqueName, FILTER_FilterFormat mode, float freq, float q, float gain);

    void prepareToPlay(double sampleRate, int samplesPerBlock);

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&);

    void reset();

    const juce::String getName();

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> myFilter;
    FILTER_FilterFormat myMode;
    float myFreq;
    float myQ;
    float myGain;
};
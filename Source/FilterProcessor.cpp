#include "FilterProcessor.h"

FilterProcessor::FilterProcessor(std::string newUniqueName, FILTER_FilterFormat mode, float freq = 1000.f, float q=1.f, float gain=1.f)
    : ProcessorBase{ newUniqueName }, myMode{ mode }, myFreq{ freq }, myQ{ q }, myGain{ gain }
{

}

void
FilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const int numChannels = 2; // todo: make configurable.

    switch (myMode)
    {
    case FILTER_FilterFormat::Invalid:
        return; // todo: throw error
        break;
    case FILTER_FilterFormat::LOW_PASS:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, myFreq, myQ);
        break;
    case FILTER_FilterFormat::BAND_PASS:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, myFreq, myQ);
        break;
    case FILTER_FilterFormat::HIGH_PASS:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, myFreq, myQ);
        break;
    case FILTER_FilterFormat::LOW_SHELF:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, myFreq, myQ, myGain);
        break;
    case FILTER_FilterFormat::HIGH_SHELF:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, myFreq, myQ, myGain);
        break;
    case FILTER_FilterFormat::NOTCH:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate, myFreq, myQ);
        break;
    default:
        return; // todo: throw error
        break;
    }

    juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), numChannels };
    myFilter.prepare(spec);
}

void
FilterProcessor::processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    myFilter.process(context);
}

void
FilterProcessor::reset()
{
    myFilter.reset();
}

const juce::String
FilterProcessor::getName() { return "FilterProcessor"; }
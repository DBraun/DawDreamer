#include "FilterProcessor.h"

FilterProcessor::FilterProcessor(std::string newUniqueName, std::string mode, float freq = 1000.f, float q=0.707107f, float gain=1.f)

    : ProcessorBase{ newUniqueName }, myFreq{ freq }, myQ{ q }, myGain{ gain }
{
    setMode(mode);
}

void
FilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    mySampleRate = sampleRate;
    mySamplesPerBlock = samplesPerBlock;
    FilterProcessor::updateParameters();
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

std::string
FilterProcessor::modeToString(FILTER_FilterFormat mode) {
    switch (mode)
    {
    case FILTER_FilterFormat::Invalid:
        return "invalid"; // todo: throw error
        break;
    case FILTER_FilterFormat::LOW_PASS:
        return "low";
        break;
    case FILTER_FilterFormat::BAND_PASS:
        return "band";
        break;
    case FILTER_FilterFormat::HIGH_PASS:
        return "high";
        break;
    case FILTER_FilterFormat::LOW_SHELF:
        return "low_shelf";
        break;
    case FILTER_FilterFormat::HIGH_SHELF:
        return "high_shelf";
        break;
    case FILTER_FilterFormat::NOTCH:
        return "notch";
        break;
    default:
        return "invalid";
        break;
    }
}

FILTER_FilterFormat
FilterProcessor::stringToMode(std::string s) {
    if (!s.compare("low")) {
        return FILTER_FilterFormat::LOW_PASS;
    }
    else if (!s.compare("high")) {
        return FILTER_FilterFormat::HIGH_PASS;
    }
    else if (!s.compare("band")) {
        return FILTER_FilterFormat::BAND_PASS;
    }
    else if (!s.compare("low_shelf")) {
        return FILTER_FilterFormat::LOW_SHELF;
    }
    else if (!s.compare("high_shelf")) {
        return FILTER_FilterFormat::HIGH_SHELF;
    }
    else if (!s.compare("notch")) {
        return FILTER_FilterFormat::NOTCH;
    }

    return FILTER_FilterFormat::Invalid;
}

void FilterProcessor::updateParameters() {
    const int numChannels = 2; // todo: make configurable.

    switch (myMode)
    {
    case FILTER_FilterFormat::Invalid:
        return; // todo: throw error
        break;
    case FILTER_FilterFormat::LOW_PASS:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(mySampleRate, myFreq, myQ);
        break;
    case FILTER_FilterFormat::BAND_PASS:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(mySampleRate, myFreq, myQ);
        break;
    case FILTER_FilterFormat::HIGH_PASS:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(mySampleRate, myFreq, myQ);
        break;
    case FILTER_FilterFormat::LOW_SHELF:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(mySampleRate, myFreq, myQ, myGain);
        break;
    case FILTER_FilterFormat::HIGH_SHELF:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(mySampleRate, myFreq, myQ, myGain);
        break;
    case FILTER_FilterFormat::NOTCH:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeNotch(mySampleRate, myFreq, myQ);
        break;
    default:
        return; // todo: throw error
        break;
    }

    juce::dsp::ProcessSpec spec{ mySampleRate, static_cast<juce::uint32> (mySamplesPerBlock), numChannels };
    myFilter.prepare(spec);
}

const juce::String
FilterProcessor::getName() { return "FilterProcessor"; }

void
FilterProcessor::setMode(std::string mode) {
    myMode = stringToMode(mode);
    if (myMode == FILTER_FilterFormat::Invalid) {
        // todo: better error handling.
        std::cout << "Warning: Unrecognized filter mode: " << mode << std::endl;
    };
    updateParameters();
}

std::string
FilterProcessor::getMode() {
    return modeToString(myMode);
}

void
FilterProcessor::setFrequency(float freq) { myFreq = freq; updateParameters(); }
float
FilterProcessor::getFrequency() { return myFreq; }

void
FilterProcessor::setQ(float q) { myQ = q; updateParameters(); }
float
FilterProcessor::getQ() { return myQ; }

void
FilterProcessor::setGain(float gain) { myGain = gain; updateParameters(); }
float
FilterProcessor::getGain() { return myGain; }
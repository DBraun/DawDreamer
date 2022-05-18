#include "FilterProcessor.h"

FilterProcessor::FilterProcessor(std::string newUniqueName, std::string mode, float freq = 1000.f, float q=0.707107f, float gain=1.f)

    : ProcessorBase{ createParameterLayout, newUniqueName }
{
    setFrequency(freq);
    setQ(q);
    setGain(gain);

    myFreq = myParameters.getRawParameterValue("freq");
    myQ = myParameters.getRawParameterValue("q");
    myGain = myParameters.getRawParameterValue("gain");

    setMode(mode);
    setMainBusInputsAndOutputs(2, 2);
}

const juce::String FilterProcessor::getName() { return "FilterProcessor"; }

void
FilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    mySampleRate = sampleRate;
    mySamplesPerBlock = samplesPerBlock;

    automateParameters();  // this gives the filters an initial state.

    int numChannels = 2;
    juce::dsp::ProcessSpec spec{ mySampleRate, static_cast<juce::uint32> (mySamplesPerBlock), static_cast<juce::uint32> (numChannels) };
    myFilter.prepare(spec);  // todo: need to do this?
}

void
FilterProcessor::processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer)
{
    automateParameters();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    myFilter.process(context);
    ProcessorBase::processBlock(buffer, midiBuffer);
}


void FilterProcessor::automateParameters() {

    AudioPlayHead::CurrentPositionInfo posInfo;
    getPlayHead()->getCurrentPosition(posInfo);

    *myFreq = getAutomationVal("freq", posInfo);
    *myQ = getAutomationVal("q", posInfo);
    *myGain = getAutomationVal("gain", posInfo);
    
    switch (myMode)
    {
    case FILTER_FilterFormat::Invalid:
        return; // todo: throw error
        break;
    case FILTER_FilterFormat::LOW_PASS:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(mySampleRate, *myFreq, *myQ);
        break;
    case FILTER_FilterFormat::BAND_PASS:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(mySampleRate, *myFreq, *myQ);
        break;
    case FILTER_FilterFormat::HIGH_PASS:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(mySampleRate, *myFreq, *myQ);
        break;
    case FILTER_FilterFormat::LOW_SHELF:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(mySampleRate, *myFreq, *myQ, *myGain);
        break;
    case FILTER_FilterFormat::HIGH_SHELF:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(mySampleRate, *myFreq, *myQ, *myGain);
        break;
    case FILTER_FilterFormat::NOTCH:
        *myFilter.state = *juce::dsp::IIR::Coefficients<float>::makeNotch(mySampleRate, *myFreq, *myQ);
        break;
    default:
        return; // todo: throw error
        break;
    }

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

void
FilterProcessor::setMode(std::string mode) {
    myMode = stringToMode(mode);
    if (myMode == FILTER_FilterFormat::Invalid) {
        // todo: better error handling.
        std::cout << "Warning: Unrecognized filter mode: " << mode << std::endl;
    };
}

std::string
FilterProcessor::getMode() {
    return modeToString(myMode);
}

void
FilterProcessor::setFrequency(float freq) { setAutomationVal("freq", freq);}
float
FilterProcessor::getFrequency() { AudioPlayHead::CurrentPositionInfo posInfo; return getAutomationVal("freq", posInfo); }

void
FilterProcessor::setQ(float q) { setAutomationVal("q", q);}
float
FilterProcessor::getQ() { AudioPlayHead::CurrentPositionInfo posInfo; return getAutomationVal("q", posInfo); }

void
FilterProcessor::setGain(float gain) { setAutomationVal("gain", gain);}
float
FilterProcessor::getGain() { AudioPlayHead::CurrentPositionInfo posInfo; return getAutomationVal("gain", posInfo);}

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "custom_pybind_wrappers.h"
#include "CustomParameters.h"


class ProcessorBase : public juce::AudioProcessor
{
public:
    //==============================================================================
    ProcessorBase(std::function< juce::AudioProcessorValueTreeState::ParameterLayout()> createParameterFunc, std::string newUniqueName = "") : myUniqueName{ newUniqueName }, myParameters(*this, nullptr, "PARAMETERS", createParameterFunc()) {
        this->setNonRealtime(true);
    }

    ProcessorBase(std::string newUniqueName = "") : myUniqueName{ newUniqueName }, myParameters(*this, nullptr, newUniqueName.c_str(), createEmptyParameterLayout()) {
        this->setNonRealtime(true);
    }

    //==============================================================================
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioSampleBuffer&, juce::MidiBuffer&) override {}

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

    //==============================================================================
    const juce::String getName() const override { return {}; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }

    //==============================================================================
    int getNumPrograms() override { return 0; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock&);
    void setStateInformation(const void*, int);

    bool setAutomation(std::string parameterName, py::array input);

    bool setAutomationVal(std::string parameterName, float val);

    float getAutomationVal(std::string parameterName, int index);

    std::vector<float> getAutomation(std::string parameterName);
    py::array_t<float> getAutomationNumpy(std::string parameterName);

    //==============================================================================
    std::string getUniqueName() { return myUniqueName; }

    void automateParameters(size_t) {};

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorBase)
    std::string myUniqueName;

protected:

    AudioProcessorValueTreeState myParameters;
    size_t myPlayheadIndex = 0;

    juce::AudioProcessorValueTreeState::ParameterLayout createEmptyParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;
        return params;
    }

};
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

    bool setAutomation(std::string parameterName, py::array input) {

        try
        {
            auto parameter = (AutomateParameterFloat*)myParameters.getParameter(parameterName);  // todo: why do we have to cast to AutomateParameterFloat instead of AutomateParameter
            parameter->setAutomation(input);
        }
        catch (const std::exception& e)
        {
            std::cout << "Failed to set automation: " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    bool setParameter(std::string parameterName, float val) {
        try
        {
            auto parameter = (AutomateParameterFloat*)myParameters.getParameter(parameterName);  // todo: why do we have to cast to AutomateParameterFloat instead of AutomateParameter
            parameter->setAutomation(val);
        }
        catch (const std::exception& e)
        {
            std::cout << "Failed to set parameter: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    std::vector<float> getAutomation(std::string parameterName, int maxSamples) {
        auto parameter = (AutomateParameter*)myParameters.getParameter(parameterName);

        return parameter->getAutomation();
    }

    //==============================================================================
    std::string getUniqueName() { return myUniqueName; }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorBase)
    std::string myUniqueName;

    juce::AudioProcessorValueTreeState::ParameterLayout createEmptyParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;
        return params;
    }

protected:

    AudioProcessorValueTreeState myParameters;
    size_t myPlayheadIndex = 0;
};
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "custom_pybind_wrappers.h"
#include "CustomParameters.h"


class ProcessorBase : public juce::AudioProcessor
{
public:
    //==============================================================================
    ProcessorBase(std::function< juce::AudioProcessorValueTreeState::ParameterLayout()> createParameterFunc, std::string newUniqueName = "") : myParameters( *this, nullptr, "PARAMETERS", createParameterFunc()) {
        myUniqueName = newUniqueName;
        this->setNonRealtime(true);
    }

    ProcessorBase(std::string newUniqueName = "") : myParameters(*this, nullptr, newUniqueName.c_str(), createEmptyParameterLayout()) {
        myUniqueName = newUniqueName;
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
        float* input_ptr = (float*)input.data();
        std::cout << "size: " << input.shape(0) << std::endl;
        std::vector<float> automation = std::vector<float>(input.shape(0), 0.f);
        std::cout << "setAutomation debug 1 " << std::endl;

        memcpy(automation.data(), input_ptr, sizeof(float) * input.shape(0));

        try
        {
            auto parameter = (AutomateParameter*)myParameters.getParameter(parameterName);
            std::cout << "setAutomation debug 2 " << std::endl;
            //parameter->setAutomation(automation);
            std::vector<float> data = { 0.f, 0.f, 0.f };
            parameter->setAutomation(data);
            std::cout << "setAutomation debug 3 " << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cout << "Failed to set automation: " << e.what() << std::endl;
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
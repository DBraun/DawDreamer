#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "custom_pybind_wrappers.h"
#include "CustomParameters.h"


class ProcessorBase : public juce::AudioProcessor
{
public:
    //==============================================================================
    ProcessorBase(std::function< juce::AudioProcessorValueTreeState::ParameterLayout()> createParameterFunc, std::string newUniqueName = "") : 
        myUniqueName{ newUniqueName }, myParameters(*this, nullptr, "PARAMETERS", createParameterFunc()) {
        this->setNonRealtime(true);
    }

    ProcessorBase(std::string newUniqueName = "") : myUniqueName{ newUniqueName }, myParameters(*this, nullptr, newUniqueName.c_str(), createEmptyParameterLayout()) {
        this->setNonRealtime(true);
    }

    //==============================================================================
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}

    // All subclasses of ProcessorBase should implement processBlock and call ProcessorBase::processBlock at the end
    // of their implementation. The ProcessorBase implementation takes care of recording.
    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) {
    
        if (!m_recordEnable) {
            return;
        }
        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        const int numberChannels = buffer.getNumChannels();

        for (int chan = 0; chan < numberChannels; chan++) {
            // Write the sample to the engine's history for the correct channel.       
            int numSamplesToCopy = std::min(buffer.getNumSamples(), (int) myRecordBuffer.getNumSamples() -(int)posInfo.timeInSamples);
            // assume numSamplesToCopy > 0
            myRecordBuffer.copyFrom(chan, posInfo.timeInSamples, buffer.getReadPointer(chan), numSamplesToCopy);
        }
    }

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

    void automateParameters() {};

    void setRecordEnable(bool recordEnable) { m_recordEnable = recordEnable; }
    bool getRecordEnable() { return m_recordEnable; }

    py::array_t<float>
    getAudioFrames()
    {
        size_t num_channels = myRecordBuffer.getNumChannels();
        size_t num_samples = myRecordBuffer.getNumSamples();

        py::array_t<float, py::array::c_style> arr({ (int)num_channels, (int)num_samples });

        auto ra = arr.mutable_unchecked();

        for (size_t i = 0; i < num_channels; i++)
        {
            for (size_t j = 0; j < num_samples; j++)
            {
                ra(i, j) = myRecordBuffer.getSample(i, j);
            }
        }

        return arr;
    }

    void setRecorderLength(int numSamples) {
        if (m_recordEnable) {
            myRecordBuffer.setSize(2, numSamples);
        }
        else {
            myRecordBuffer.setSize(2, 0);
        }
    }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorBase)
    std::string myUniqueName;
    juce::AudioSampleBuffer myRecordBuffer;
 
protected:

    AudioProcessorValueTreeState myParameters;

    juce::AudioProcessorValueTreeState::ParameterLayout createEmptyParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;
        return params;
    }
    bool m_recordEnable = false;
};
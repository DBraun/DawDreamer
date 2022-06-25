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
    void reset();

    // All subclasses of ProcessorBase should implement processBlock and call ProcessorBase::processBlock at the end
    // of their implementation. The ProcessorBase implementation takes care of recording.
    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&);

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

    //==============================================================================
    const juce::String getName() const override { return {}; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }

    //==============================================================================
    virtual bool canApplyBusesLayout(const juce::AudioProcessor::BusesLayout& layout) {
        return AudioProcessor::canApplyBusesLayout(layout);
    }

    virtual bool setBusesLayout(const BusesLayout& arr) {
        return AudioProcessor::setBusesLayout(arr);
    }

    //==============================================================================
    int getNumPrograms() override { return 0; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock&);
    void setStateInformation(const void*, int);

    bool setAutomation(std::string parameterName, py::array input, std::uint32_t ppqn);

    virtual bool setAutomationVal(std::string parameterName, float val);

    float getAutomationVal(std::string parameterName, juce::AudioPlayHead::CurrentPositionInfo& posInfo);

    std::vector<float> getAutomation(std::string parameterName);
    py::array_t<float> getAutomationNumpy(std::string parameterName);
    py::dict getAutomationAll();

    //==============================================================================
    std::string getUniqueName() { return myUniqueName; }

    void automateParameters(int numParameters) {};
    
    void setRecordEnable(bool recordEnable) { m_recordEnable = recordEnable; }
    bool getRecordEnable() { return m_recordEnable; }
    
    void setRecordAutomationEnable(bool recordAutomation) { m_recordAutomation = recordAutomation; }
    bool getRecordAutomationEnable() { return m_recordAutomation; }
    
    py::array_t<float> bufferToPyArray(juce::AudioSampleBuffer& buffer);

    py::array_t<float> getAudioFrames();
    
    void setRecorderLength(int numSamples);

    int
    getTotalNumOutputChannels() {
        return AudioProcessor::getTotalNumOutputChannels();
    }
    
    int
    getTotalNumInputChannels() {
        return AudioProcessor::getTotalNumInputChannels();
    }

    virtual void numChannelsChanged();

    bool isConnectedInGraph() { return m_isConnectedInGraph;}
    void setConnectedInGraph(bool isConnected) {
        m_isConnectedInGraph = isConnected;
    }
    
    bool setMainBusInputsAndOutputs(int inputs, int outputs) {
        BusesLayout busesLayout = makeBusesLayout(inputs, outputs);

        if (this->canApplyBusesLayout(busesLayout)) {
            return this->setBusesLayout(busesLayout);
        }
        else {
            throw std::invalid_argument(this->getUniqueName() + " CANNOT ApplyBusesLayout inputs: " + std::to_string(inputs) + " outputs: " + std::to_string(outputs));
        }
    }

    bool canApplyBusInputsAndOutputs(int inputs, int outputs) {
        BusesLayout busesLayout = makeBusesLayout(inputs, outputs);
        return this->canApplyBusesLayout(busesLayout);
    }

    // todo: this is not a good thing to hard-code.
    // Ableton saves MIDI at a PPQN of 96, which is somewhat low.
    // To be easily compatible with higher resolution PPQN MIDI files that
    // are loaded with `load_midi`, we use an internal high rate PPQN of 960.
    // It's easy to "upsample" to this higher resolution, as we do in `load_midi` and `add_midi_note`.
    // Another bad design of the code is that FaustProcessor, PluginProcessor, and SamplerProcessor
    // have a lot of common code related to MIDIBuffers.
    // There's one set of variables for keeping track of everything in absolute time (when
    // `isBeats` is False), and another for relative-to-tempo timing (when note start
    // times and durations are measured in "quarter notes" (QN)).
    const static std::uint32_t PPQN = 3840;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorBase)
    std::string myUniqueName;
    juce::AudioSampleBuffer myRecordBuffer;
    bool m_isConnectedInGraph = false;
 
protected:

    AudioProcessorValueTreeState myParameters;

    juce::AudioProcessorValueTreeState::ParameterLayout createEmptyParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;
        return params;
    }
    bool m_recordEnable = false;
    bool m_recordAutomation = false;
    
    int m_expectedRecordNumSamples = 0;
    std::map<std::string,juce::AudioSampleBuffer> m_recordedAutomationDict;

    BusesLayout makeBusesLayout(int inputs, int outputs) {
        BusesLayout busesLayout;
        const AudioChannelSet inputChannelSet = AudioChannelSet::discreteChannels(inputs);
        const AudioChannelSet outputChannelSet = AudioChannelSet::discreteChannels(outputs);
        busesLayout.inputBuses.add(inputChannelSet);
        busesLayout.outputBuses.add(outputChannelSet);
        return busesLayout;
    }
    
    void recordAutomation(juce::AudioPlayHead::CurrentPositionInfo& posInfo, int numSamples);
};

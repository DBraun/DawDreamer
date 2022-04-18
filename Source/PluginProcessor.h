#pragma once

#include "ProcessorBase.h"
#include "custom_pybind_wrappers.h"

typedef std::vector<std::pair<int, float>> PluginPatch;

class PluginProcessor : public ProcessorBase
{
public:
    PluginProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock, std::string path);
    ~PluginProcessor();

    bool canApplyBusesLayout(const juce::AudioProcessor::BusesLayout& layout);

    bool canApplyBusInputsAndOutputs(int inputs, int outputs) {
        BusesLayout busesLayout = this->makeBusesLayout(inputs, outputs);
        return this->canApplyBusesLayout(busesLayout);
    }

    bool setBusesLayout(const BusesLayout& arr);

    bool setMainBusInputsAndOutputs(int inputs, int outputs) {
        BusesLayout busesLayout = this->makeBusesLayout(inputs, outputs);

        if (this->canApplyBusesLayout(busesLayout)) {
            return this->setBusesLayout(busesLayout);
        }
        else {
            throw std::invalid_argument(this->getUniqueName() + " CANNOT ApplyBusesLayout inputs: " + std::to_string(inputs) + " outputs: " + std::to_string(outputs));
            return false;
        }
    }

    void numChannelsChanged() {
        ProcessorBase::numChannelsChanged();
        if (myPlugin.get()) {
            myPlugin->setPlayConfigDetails(this->getTotalNumInputChannels(), this->getTotalNumOutputChannels(), this->getSampleRate(), this->getBlockSize());
        }
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock);

    bool supportsDoublePrecisionProcessing() { return myPlugin ? myPlugin->supportsDoublePrecisionProcessing() : false; }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer);

    bool acceptsMidi() const { return true; }
    bool producesMidi() const { return true; }

    void reset();

    bool loadPreset(const std::string& path);
    bool loadVST3Preset(const std::string& path);

    void createParameterLayout();  // NB: this is different from other processors because it's called after a VST is loaded.

    void setPatch(const PluginPatch patch);

    std::string getParameterAsText(const int parameter);
    void setParameter(int paramIndex, float newValue);
    const PluginPatch getPatch();
    const size_t getPluginParameterSize();

    const juce::String getName() const { return "PluginProcessor"; }

    bool loadMidi(const std::string& path, bool allEvents);

    void clearMidi();

    int getNumMidiEvents();

    bool addMidiNote(const uint8  midiNote,
        const uint8  midiVelocity,
        const double noteStart,
        const double noteLength);

    void setPlayHead(AudioPlayHead* newPlayHead);

    void openEditor();

    void loadStateInformation(std::string filepath) {

        MemoryBlock state;
        File file = File(filepath);
        file.loadFileAsData(state);
        
        myPlugin->setStateInformation((const char*)state.getData(), (int)state.getSize());

        for (int i = 0; i < myPlugin->AudioProcessor::getNumParameters(); i++) {
            std::string paramID = std::to_string(i);
            ProcessorBase::setAutomationVal(paramID, myPlugin->getParameter(i));
        }
    }

    void saveStateInformation(std::string filepath) {
        if (!myPlugin) {
            throw std::runtime_error("Please load the plugin first");
        }
        MemoryBlock state;
        myPlugin->getStateInformation(state);

        juce::File file(filepath);
        juce::FileOutputStream fos(file);

        fos.write(state.getData(), state.getSize());
    }

private:

    bool loadPlugin(double sampleRate, int samplesPerBlock);
    bool isLoaded = false;

    std::string myPluginPath;
    double mySampleRate;

    MidiBuffer myMidiBuffer;
    MidiBuffer myRenderMidiBuffer;
    MidiMessage myMidiMessage;
    int myMidiMessagePosition = -1;
    MidiBuffer::Iterator* myMidiIterator = nullptr;
    bool myIsMessageBetween = false;
    bool myMidiEventsDoRemain = false;

    void automateParameters();

protected:

    std::unique_ptr<juce::AudioPluginInstance, std::default_delete<juce::AudioPluginInstance>> myPlugin = nullptr;
};


//==========================================================================
class PluginProcessorWrapper : public PluginProcessor
{
public:

    PluginProcessorWrapper(std::string newUniqueName, double sampleRate, int samplesPerBlock, std::string path);

    void wrapperSetPatch(py::list listOfTuples);

    py::list wrapperGetPatch();

    float wrapperGetParameter(int parameterIndex);

    std::string wrapperGetParameterName(int parameter);

    bool wrapperSetParameter(int parameter, float value);

    bool wrapperSetAutomation(int parameterIndex, py::array input);

    int wrapperGetPluginParameterSize();

    py::list getPluginParametersDescription();

};

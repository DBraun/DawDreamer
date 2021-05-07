#pragma once

#include "ProcessorBase.h"
#include "custom_pybind_wrappers.h"

typedef std::vector<std::pair<int, float>> PluginPatch;

class PluginProcessor : public ProcessorBase
{
public:
    PluginProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock, std::string path);
    ~PluginProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock);

    bool supportsDoublePrecisionProcessing() { return myPlugin ? myPlugin->supportsDoublePrecisionProcessing() : false; }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer);

    bool acceptsMidi() const { return true; }
    bool producesMidi() const { return true; }

    void reset();

    bool loadPreset(const std::string& path);

    void createParameterLayout();  // NB: this is different from other processors because it's called after a VST is loaded.

    void setPatch(const PluginPatch patch);

    std::string getParameterAsText(const int parameter);
    void setParameter(int paramIndex, float newValue);
    const PluginPatch getPatch();
    const size_t getPluginParameterSize();

    const juce::String getName() const { return "PluginProcessor"; }

    bool loadMidi(const std::string& path);

    void clearMidi();

    int getNumMidiEvents();

    bool addMidiNote(const uint8  midiNote,
        const uint8  midiVelocity,
        const double noteStart,
        const double noteLength);

    void setPlayHead(AudioPlayHead* newPlayHead);

private:

    bool loadPlugin(double sampleRate, int samplesPerBlock);

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

    std::unique_ptr<juce::AudioPluginInstance, std::default_delete<juce::AudioPluginInstance>> myPlugin;
    // For an explanation of myCopyBuffer, read PluginProcessor::processBlock
    juce::AudioBuffer<float>* myCopyBuffer = nullptr;
    int myCopyBufferNumChans = 2;

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

    void wrapperSetParameter(int parameter, float value);

    int wrapperGetPluginParameterSize();

    py::list getPluginParametersDescription();

};
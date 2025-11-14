#pragma once

#include "custom_nanobind_wrappers.h"
#include "ProcessorBase.h"

typedef std::vector<std::pair<int, float>> PluginPatch;

// Alias for variant that can hold either float or string
using ValueType = std::variant<float, std::string>;

class PluginProcessor : public ProcessorBase
{
  public:
    PluginProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock,
                    std::string path);
    ~PluginProcessor();

    bool canApplyBusesLayout(const juce::AudioProcessor::BusesLayout& layout) override;

    bool setBusesLayout(const BusesLayout& arr) override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override;

    void automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) override;

    bool acceptsMidi() const override { return myPlugin.get() && myPlugin->acceptsMidi(); }
    bool producesMidi() const override { return myPlugin.get() && myPlugin->producesMidi(); }
    double getTailLengthSeconds() const override;
    int getLatencySamples();

    void reset() override;

    bool loadPreset(const std::string& path);
    bool loadVST3Preset(const std::string& path);

    void createParameterLayout();

    void setPatch(const PluginPatch patch);

    std::string getParameterAsText(const int parameter);
    const PluginPatch getPatch();
    const size_t getPluginParameterSize();
    std::map<std::pair<float, float>, ValueType>
    getParameterValueRange(const int parameterIndex, int search_steps, bool convert);

    const juce::String getName() const override { return "PluginProcessor"; }

    bool loadMidi(const std::string& path, bool clearPrevious, bool isBeats, bool allEvents);

    void clearMidi();

    int getNumMidiEvents();

    bool addMidiNote(const uint8 midiNote, const uint8 midiVelocity, const double noteStart,
                     const double noteLength, bool isBeats);

    void setPlayHead(AudioPlayHead* newPlayHead) override;

    void openEditor();

    void loadStateInformation(std::string filepath);

    void saveStateInformation(std::string filepath);

    void saveMIDI(std::string& savePath);

  private:
    bool loadPlugin(double sampleRate, int samplesPerBlock);

    std::string myPluginPath;
    double mySampleRate;

    MidiBuffer myMidiBufferQN;
    MidiBuffer myMidiBufferSec;

    MidiBuffer myRenderMidiBuffer;
    MidiMessageSequence myRecordedMidiSequence; // for fetching by user later.

    MidiMessage myMidiMessageQN;
    MidiMessage myMidiMessageSec;

    int myMidiMessagePositionQN = -1;
    int myMidiMessagePositionSec = -1;

    MidiBuffer::Iterator* myMidiIteratorQN = nullptr;
    MidiBuffer::Iterator* myMidiIteratorSec = nullptr;

    bool myIsMessageBetweenQN = false;
    bool myIsMessageBetweenSec = false;

    bool myMidiEventsDoRemainQN = false;
    bool myMidiEventsDoRemainSec = false;

  protected:
    std::unique_ptr<juce::AudioPluginInstance, std::default_delete<juce::AudioPluginInstance>>
        myPlugin = nullptr;
};

//==========================================================================
class PluginProcessorWrapper : public PluginProcessor
{
  public:
    PluginProcessorWrapper(std::string newUniqueName, double sampleRate, int samplesPerBlock,
                           std::string path);

    void wrapperSetPatch(nb::list listOfTuples);

    nb::list wrapperGetPatch();

    std::string wrapperGetParameterName(const int& parameter);

    bool wrapperSetParameter(const int& parameterIndex, const float& value);

    int wrapperGetPluginParameterSize();

    nb::list getPluginParametersDescription();
};

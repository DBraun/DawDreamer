#pragma once

#include "custom_nanobind_wrappers.h"
#include "MidiSerialization.h"
#include "PickleVersion.h"
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

    nb::dict getPickleState()
    {
        nb::dict state;
        state["pickle_version"] = DawDreamerPickle::getVersion();
        state["unique_name"] = getUniqueName();
        state["plugin_path"] = myPluginPath;
        state["sample_rate"] = mySampleRate;

        // Get plugin state as binary blob
        if (myPlugin)
        {
            juce::MemoryBlock stateData;
            myPlugin->getStateInformation(stateData);

            // Convert MemoryBlock to bytes
            nb::bytes plugin_state((const char*)stateData.getData(), stateData.getSize());
            state["plugin_state"] = plugin_state;
        }
        else
        {
            state["plugin_state"] = nb::bytes("", 0);
        }

        // Serialize MIDI buffers using shared helper
        state["midi_qn"] = MidiSerialization::serializeMidiBuffer(myMidiBufferQN);
        state["midi_sec"] = MidiSerialization::serializeMidiBuffer(myMidiBufferSec);

        return state;
    }

    void setPickleState(nb::dict state)
    {
        // Check pickle version
        if (state.contains("pickle_version"))
        {
            int version = nb::cast<int>(state["pickle_version"]);
            if (!DawDreamerPickle::isCompatibleVersion(version))
            {
                throw std::runtime_error(DawDreamerPickle::getVersionErrorMessage(version));
            }
        }

        std::string name = nb::cast<std::string>(state["unique_name"]);
        std::string plugin_path = nb::cast<std::string>(state["plugin_path"]);
        double sr = nb::cast<double>(state["sample_rate"]);

        // Reconstruct using placement new
        new (this) PluginProcessor(name, sr, 512, plugin_path);

        // Restore plugin state
        if (state.contains("plugin_state") && myPlugin)
        {
            nb::bytes plugin_state_bytes = nb::cast<nb::bytes>(state["plugin_state"]);
            if (plugin_state_bytes.size() > 0)
            {
                myPlugin->setStateInformation(plugin_state_bytes.c_str(),
                                              (int)plugin_state_bytes.size());

                // Update automation values from plugin parameters
                int i = 0;
                for (auto* parameter : myPlugin->getParameters())
                {
                    ProcessorBase::setAutomationValByIndex(i, parameter->getValue());
                    i++;
                }
            }
        }

        // Restore MIDI buffers using shared helper
        if (state.contains("midi_qn"))
        {
            nb::bytes midi_qn_data = nb::cast<nb::bytes>(state["midi_qn"]);
            MidiSerialization::deserializeMidiBuffer(myMidiBufferQN, midi_qn_data);
        }

        if (state.contains("midi_sec"))
        {
            nb::bytes midi_sec_data = nb::cast<nb::bytes>(state["midi_sec"]);
            MidiSerialization::deserializeMidiBuffer(myMidiBufferSec, midi_sec_data);
        }
    }

    // Restore plugin state without placement new (for RenderEngine restoration)
    void restorePluginState(nb::dict state)
    {
        if (state.contains("plugin_state") && myPlugin)
        {
            nb::bytes plugin_state_bytes = nb::cast<nb::bytes>(state["plugin_state"]);
            if (plugin_state_bytes.size() > 0)
            {
                myPlugin->setStateInformation(plugin_state_bytes.c_str(),
                                              (int)plugin_state_bytes.size());

                // Update automation values from plugin parameters
                int i = 0;
                for (auto* parameter : myPlugin->getParameters())
                {
                    ProcessorBase::setAutomationValByIndex(i, parameter->getValue());
                    i++;
                }
            }
        }

        // Restore MIDI buffers using shared helper
        if (state.contains("midi_qn"))
        {
            nb::bytes midi_qn_data = nb::cast<nb::bytes>(state["midi_qn"]);
            MidiSerialization::deserializeMidiBuffer(myMidiBufferQN, midi_qn_data);
        }

        if (state.contains("midi_sec"))
        {
            nb::bytes midi_sec_data = nb::cast<nb::bytes>(state["midi_sec"]);
            MidiSerialization::deserializeMidiBuffer(myMidiBufferSec, midi_sec_data);
        }
    }

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

    // Expose pickle methods from base class
    nb::dict getPickleState() { return PluginProcessor::getPickleState(); }
    void setPickleState(nb::dict state) { PluginProcessor::setPickleState(state); }
};

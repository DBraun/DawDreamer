#pragma once

#include "ProcessorBase.h"

#ifdef BUILD_DAWDREAMER_FAUST

#include "faust/dsp/poly-llvm-dsp.h"
#include "faust/dsp/poly-interpreter-dsp.h"
#include "generator/libfaust.h"
#include "faust/gui/APIUI.h"
#include "faust/gui/MidiUI.h"
#include "faust/gui/SoundUI.h"

#include "faust/midi/rt-midi.h"
#include "TMutex.h"

#include <iostream>
#include <map>


class MySoundUI : public SoundUI {

public:

    virtual void addSoundfile(const char* label, const char* filename, Soundfile** sf_zone) {
        // Parse the possible list
        std::string saved_url_real = std::string(label);
        if (fSoundfileMap.find(saved_url_real) == fSoundfileMap.end()) {
            // If failure, use 'defaultsound'
            std::cerr << "addSoundfile : soundfile for " << label << " cannot be created !" << std::endl;
            *sf_zone = defaultsound;
            return;
        }

        // Get the soundfile
        *sf_zone = fSoundfileMap[saved_url_real];
    }

    virtual void addSoundfileFromBuffers(const char* label, std::vector<AudioSampleBuffer> buffers, int sample_rate)
    {
        // Parse the possible list
        std::string saved_url_real = std::string(label);
        if (fSoundfileMap.find(saved_url_real) == fSoundfileMap.end()) {

            int total_length = 0;
            int numChannels = 1;  // start with at least 1 channel. This may increase due to code below.

            for (auto& buffer : buffers) {
                total_length += buffer.getNumSamples();
                numChannels = std::max(numChannels, buffer.getNumChannels());
            }

            total_length += (MAX_SOUNDFILE_PARTS - buffers.size()) * BUFFER_SIZE;

            Soundfile* soundfile = new Soundfile(numChannels, total_length, MAX_CHAN, buffers.size(), false);

            // Manually fill in the soundfile:
            // The following code is a modification of SoundfileReader::createSoundfile and SoundfileReader::readFile

            int offset = 0;

            int i = 0;
            for (auto& buffer : buffers) {

                int numSamples = buffer.getNumSamples();

                soundfile->fLength[i] = numSamples;
                soundfile->fSR[i] = sample_rate;
                soundfile->fOffset[i] = offset;

                void* tmpBuffers = alloca(soundfile->fChannels * sizeof(float*));
                soundfile->getBuffersOffsetReal<float>(tmpBuffers, offset);

                for (int chan = 0; chan < buffer.getNumChannels(); chan++) {
                    for (int sample = 0; sample < numSamples; sample++) {
                        // todo: don't assume float
                        // todo: use memcpy or similar to be faster
                        static_cast<float**>(soundfile->fBuffers)[chan][offset + sample] = buffer.getSample(chan, sample);
                    }
                }

                offset += soundfile->fLength[i];
                i++;
            }

            // Complete with empty parts
            for (int i = buffers.size(); i < MAX_SOUNDFILE_PARTS; i++) {
                soundfile->emptyFile(i, offset);
            }

            // Share the same buffers for all other channels so that we have max_chan channels available
            soundfile->shareBuffers(numChannels, MAX_CHAN);

            fSoundfileMap[saved_url_real] = soundfile;
        }
    }
};


class FaustProcessor : public ProcessorBase
{
public:
    FaustProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock);
    ~FaustProcessor();

    bool canApplyBusesLayout(const juce::AudioProcessor::BusesLayout& layout);
    void prepareToPlay(double sampleRate, int samplesPerBlock);

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer);

    bool acceptsMidi() const { return false; }
    bool producesMidi() const { return false; }

    void reset();

    void createParameterLayout();  // NB: this is different from other processors because it's called after a Faust DSP file is compiled.

    const juce::String getName() const { return "FaustProcessor"; }

    bool setAutomation(std::string parameterName, py::array input, std::uint32_t ppqn);

    // faust stuff
    void clear();
    bool compile();
    bool setDSPString(const std::string& code);
    bool setDSPFile(const std::string& path);
    bool setParamWithIndex(const int index, float p);
    float getParamWithIndex(const int index);
    float getParamWithPath(const std::string& n);
    std::string code();
    bool isCompiled() { return m_isCompiled; };

    py::list getPluginParametersDescription();

    void setNumVoices(int numVoices);
    int getNumVoices();

    void setGroupVoices(bool groupVoices);
    int getGroupVoices();

    void setAutoImport(const std::string& s) { m_autoImport = s; }
    std::string getAutoImport() { return m_autoImport; }

    bool loadMidi(const std::string& path, bool clearPrevious, bool isBeats, bool allEvents);

    void clearMidi();

    int getNumMidiEvents();

    bool addMidiNote(const uint8  midiNote,
        const uint8  midiVelocity,
        const double noteStart,
        const double noteLength,
        bool isBeats);

    void setSoundfiles(py::dict);

    double getReleaseLength();

    void setReleaseLength(double sec);

    void setFaustLibrariesPath(std::string faustLibrariesPath) {
        m_faustLibrariesPath = faustLibrariesPath;
    }

    std::string getFaustLibrariesPath() {
        return m_faustLibrariesPath;
    }

    std::map<std::string, std::vector<juce::AudioSampleBuffer>> m_SoundfileMap;
    
    void saveMIDI(std::string& savePath);

private:

    double mySampleRate;

    void automateParameters(int numSamples);

    std::string getPathToFaustLibraries();

protected:

    llvm_dsp_factory* m_factory = nullptr;
    dsp* m_dsp = nullptr;
    APIUI* m_ui = nullptr;

    llvm_dsp_poly_factory* m_poly_factory = nullptr;
    dsp_poly* m_dsp_poly = nullptr;
    MySoundUI* m_soundUI = nullptr;

    rt_midi m_midi_handler;

    int m_numInputChannels = 0;
    int m_numOutputChannels = 0;

    double m_releaseLengthSec = 0.5;

    std::string m_autoImport;
    std::string m_code;
    std::string m_faustLibrariesPath = "";

    int m_nvoices = 0;
    bool m_groupVoices = true;
    bool m_isCompiled = false;

    MidiBuffer myMidiBufferQN;
    MidiBuffer myMidiBufferSec;
    
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

    juce::AudioSampleBuffer oneSampleInBuffer;
    juce::AudioSampleBuffer oneSampleOutBuffer;

    std::map<int, int> m_map_juceIndex_to_faustIndex;
    std::map<int, std::string> m_map_juceIndex_to_parAddress;

    TMutex guiUpdateMutex;
};

#endif

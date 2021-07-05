#pragma once

#include "ProcessorBase.h"

#ifdef BUILD_DAWDREAMER_FAUST

#include "faust/dsp/poly-llvm-dsp.h"
#include "faust/dsp/poly-interpreter-dsp.h"
#include "generator/libfaust.h"
#include "faust/gui/APIUI.h"
#include "faust/gui/MidiUI.h"

#include "faust/midi/rt-midi.h"

#include <iostream>
#include <map>

class FaustProcessor : public ProcessorBase
{
public:
    FaustProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock, std::string path);
    ~FaustProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock);

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer);

    bool acceptsMidi() const { return false; }
    bool producesMidi() const { return false; }

    void reset();

    void createParameterLayout();  // NB: this is different from other processors because it's called after a Faust DSP file is compiled.

    const juce::String getName() const { return "FaustProcessor"; }

    // faust stuff
    void clear();
    bool compileFromString(const std::string& code);
    bool compileFromFile(const std::string& path);
    bool setParamWithIndex(const int index, float p);
    float getParamWithIndex(const int index);
    float getParamWithPath(const std::string& n);
    std::string code();
    bool isCompiled() { return m_isCompiled; };

    py::list getPluginParametersDescription();

    bool setNumVoices(int numVoices);
    int getNumVoices(int numVoices);

    bool loadMidi(const std::string& path);

    void clearMidi();

    int getNumMidiEvents();

    bool addMidiNote(const uint8  midiNote,
        const uint8  midiVelocity,
        const double noteStart,
        const double noteLength);

private:

    double mySampleRate;

    void automateParameters();

protected:

    llvm_dsp_factory* m_factory;
    dsp* m_dsp;
    MidiUI* m_midi_ui;
    APIUI* m_ui;

    llvm_dsp_poly_factory* m_poly_factory;
    dsp_poly* m_dsp_poly;

    rt_midi m_midi_handler;

    int m_numInputChannels = 0;
    int m_numOutputChannels = 0;

    std::string m_autoImport;
    std::string m_code;

    int m_nvoices = 0;    
    bool m_isCompiled = false;

    MidiBuffer myMidiBuffer;
    MidiMessage myMidiMessage;
    int myMidiMessagePosition = -1;
    MidiBuffer::Iterator* myMidiIterator = nullptr;
    bool myIsMessageBetween = false;
    bool myMidiEventsDoRemain = false;

    AudioSampleBuffer oneSampleInBuffer;
    AudioSampleBuffer oneSampleOutBuffer;

    std::map<int, int> m_map_juceIndex_to_faustIndex;
    std::map<int, std::string> m_map_juceIndex_to_parAddress;
};

#endif

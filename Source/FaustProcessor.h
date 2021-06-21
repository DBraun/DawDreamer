#pragma once

#include "ProcessorBase.h"

#ifdef BUILD_DAWDREAMER_FAUST

#include "faust/dsp/poly-llvm-dsp.h"
#include "faust/dsp/poly-interpreter-dsp.h"
#include "generator/libfaust.h"
#include "faust/gui/APIUI.h"
#include "faust/gui/MidiUI.h"

#include <iostream>

#ifdef WIN32
__declspec(selectany) std::list<GUI*> GUI::fGuiList;
__declspec(selectany) ztimedmap GUI::gTimedZoneMap;
#endif


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
    float setParamWithPath(const std::string& n, float p);
    float getParamWithPath(const std::string& n);
    float setParamWithIndex(const int index, float p);
    float getParamWithIndex(const int index);
    std::string code();

    py::list getPluginParametersDescription();

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

    int m_numInputChannels = 0;
    int m_numOutputChannels = 0;

    std::string m_autoImport;
    std::string m_code;

    int m_nvoices = 0;
    bool m_polyphony_enable = false;
    bool m_midi_enable = false;

};

#endif

#pragma once

#include "ProcessorBase.h"

#ifdef BUILD_DAWDREAMER_FAUST

#include "faust/dsp/poly-llvm-dsp.h"
#include "faust/dsp/poly-interpreter-dsp.h"
#include "generator/libfaust.h"

#include "faust/gui/meta.h"
#include "faust/gui/APIUI.h"
#include "faust/gui/FUI.h"
#include "faust/gui/MidiUI.h"
#include "faust/gui/UI.h"
#include "faust/gui/GUI.h"
#include "faust/gui/PathBuilder.h"

#include <map>
#include <iostream>

#include <regex>
//std::list<GUI*> GUI::fGuiList;
//ztimedmap GUI::gTimedZoneMap;
__declspec(selectany) std::list<GUI*> GUI::fGuiList;
__declspec(selectany) ztimedmap GUI::gTimedZoneMap;

//-----------------------------------------------------------------------------
// name: class FaustCHOPUI
// desc: Faust CHOP UI -> map of complete hierarchical path and zones
//-----------------------------------------------------------------------------
class FaustCHOPUI : public APIUI
{

public:

    void
    dumpParams() {
        // iterator
        std::map<std::string, int>::iterator iter = fPathMap.begin();
        // go
        for (; iter != fPathMap.end(); iter++)
        {
            // print
            std::cerr << iter->first << " : " << (iter->second) << std::endl;
        }
    }
};


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
    bool eval(const std::string& code);
    bool compileFromFile(const std::string& path);
    bool compile();
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
    FaustCHOPUI* m_ui;

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
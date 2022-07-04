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

#include "faust/dsp/libfaust-signal.h"
#include "faust/misc.h"
#include "faust/export.h"

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

            Soundfile* soundfile = new Soundfile(numChannels, total_length, MAX_CHAN, (int) buffers.size(), false);

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
            for (auto i = (int) buffers.size(); i < MAX_SOUNDFILE_PARTS; i++) {
                soundfile->emptyFile(i, offset);
            }

            // Share the same buffers for all other channels so that we have max_chan channels available
            soundfile->shareBuffers(numChannels, MAX_CHAN);

            fSoundfileMap[saved_url_real] = soundfile;
        }
    }
};

struct SigWrapper {
    CTree *ptr;
    SigWrapper(CTree *ptr) : ptr{ptr} {}
    SigWrapper(float val) : ptr{sigReal(val)} {}  // todo: this ignores createLibContext();
    SigWrapper(int val) : ptr{sigInt(val)} {}  // todo: this ignores createLibContext();
    operator CTree *() { return ptr; }
};


class FaustProcessor : public ProcessorBase
{
public:
    FaustProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock);
    ~FaustProcessor();

    bool canApplyBusesLayout(const juce::AudioProcessor::BusesLayout& layout) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    
    int
    getTotalNumOutputChannels() override {
        if (!m_isCompiled) {
            this->compile();
        }
        return ProcessorBase::getTotalNumOutputChannels();
    }
    
    int
    getTotalNumInputChannels() override {
        if (!m_isCompiled) {
            this->compile();
        }
        return ProcessorBase::getTotalNumInputChannels();
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override;

    bool acceptsMidi() const override { return false; } // todo: faust should be able to play MIDI.
    bool producesMidi() const override { return false; }

    void reset() override;

    void createParameterLayout();  // NB: this is different from other processors because it's called after a Faust DSP file is compiled.

    const juce::String getName() const override { return "FaustProcessor"; }

    void automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) override;
    bool setAutomation(std::string parameterName, py::array input, std::uint32_t ppqn) override;

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
    
    // public libfaust signal API
public:
    SigWrapper getSigInt(int val) { return SigWrapper(sigInt(val)); }
    
    SigWrapper getSigReal(double val) { return SigWrapper(sigReal(val)); }

    SigWrapper getSigInput(int index) { return SigWrapper(sigInput(index)); }
    
    SigWrapper getSigDelay(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigDelay(sig1, sig2)); }
    
    SigWrapper getSigIntCast(SigWrapper &sig1) { return SigWrapper(sigIntCast(sig1)); }
    SigWrapper getSigFloatCast(SigWrapper &sig1) { return SigWrapper(sigFloatCast(sig1)); }

    // todo: sigReadOnlyTable
    // todo: sigWriteReadTable
    // todo: sigSoundfileRate
    // todo: sigSoundfileBuffer
    
    std::vector<SigWrapper> getSigWaveform(std::vector<float> vals) {
        tvec waveform;
        for (auto& val : vals) {
            waveform.push_back(sigReal(val));
        }
        auto mySigWaveform = sigWaveform(waveform);

        auto result = std::vector<SigWrapper>{
            SigWrapper(sigInt((int)waveform.size())),
            SigWrapper(mySigWaveform)
        };
        
        return result;
    }
    
    std::vector<SigWrapper> getSigSoundfile(std::string &name, SigWrapper& rdx, SigWrapper& chan, SigWrapper &part) {
        // Soundfile definition
        Signal sf = sigSoundfile(name);
        Signal partInt = sigIntCast(part);
        // Wrapped index to avoid reading outside the buffer
        Signal wridx = sigIntCast(sigMax(sigInt(0),
                                         sigMin(sigIntCast(rdx),
                                                sigSub(sigSoundfileLength(sf, partInt),
                                                       sigInt(1)))));

        auto result = std::vector<SigWrapper>{
            SigWrapper(sigSoundfileLength(sf, partInt)),
            SigWrapper(sigSoundfileRate(sf, partInt)),
            SigWrapper(sigSoundfileBuffer(sf, sigIntCast(chan), partInt, wridx))
        };
        
        return result;
    }
    
    SigWrapper getSigSelect2(SigWrapper& selector, SigWrapper &sig1, SigWrapper &sig2) {
        return SigWrapper(sigSelect2(selector, sig1, sig2));
    }
    
    SigWrapper getSigSelect3(SigWrapper& selector, SigWrapper &sig1, SigWrapper &sig2, SigWrapper &sig3) {
        return SigWrapper(sigSelect3(selector, sig1, sig2, sig3));
    }
    
    SigWrapper getSigFConst(SType type, const std::string& name, const std::string& file) {
        return SigWrapper(sigFConst(type, name, file));
    }

    SigWrapper getSigFVar(SType type, const std::string& name, const std::string& file) {
        return SigWrapper(sigFVar(type, name, file));
    }

    SigWrapper getSigBinOp(SOperator op, SigWrapper &sig1, SigWrapper &sig2) {
        return SigWrapper(sigBinOp(op, sig1, sig2));
    }

    SigWrapper getSigAdd(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigAdd(sig1, sig2)); }
    SigWrapper getSigSub(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigSub(sig1, sig2)); }
    SigWrapper getSigMul(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigMul(sig1, sig2)); }
    SigWrapper getSigDiv(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigDiv(sig1, sig2)); }
    SigWrapper getSigRem(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigRem(sig1, sig2)); }
    
    SigWrapper getSigLeftShift(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigLeftShift(sig1, sig2)); }
    SigWrapper getSigLRightShift(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigLRightShift(sig1, sig2)); }
    SigWrapper getSigARightShift(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigARightShift(sig1, sig2)); }
        
    SigWrapper getSigGT(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigGT(sig1, sig2)); }
    SigWrapper getSigLT(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigLT(sig1, sig2)); }
    SigWrapper getSigGE(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigGE(sig1, sig2)); }
    SigWrapper getSigLE(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigLE(sig1, sig2)); }
    SigWrapper getSigEQ(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigEQ(sig1, sig2)); }
    SigWrapper getSigNE(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigNE(sig1, sig2)); }

    SigWrapper getSigAND(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigAND(sig1, sig2)); }
    SigWrapper getSigOR(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigOR(sig1, sig2)); }
    SigWrapper getSigXOR(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigXOR(sig1, sig2)); }
        
    SigWrapper getSigAbs(SigWrapper &sig1) { return SigWrapper(sigAbs(sig1)); }
    SigWrapper getSigAcos(SigWrapper &sig1) { return SigWrapper(sigAcos(sig1)); }
    SigWrapper getSigTan(SigWrapper &sig1) { return SigWrapper(sigTan(sig1)); }
    SigWrapper getSigSqrt(SigWrapper &sig1) { return SigWrapper(sigSqrt(sig1)); }
    SigWrapper getSigSin(SigWrapper &sig1) { return SigWrapper(sigSin(sig1)); }
    SigWrapper getSigRint(SigWrapper &sig1) { return SigWrapper(sigRint(sig1)); }
    SigWrapper getSigLog(SigWrapper &sig1) { return SigWrapper(sigLog(sig1)); }
    SigWrapper getSigLog10(SigWrapper &sig1) { return SigWrapper(sigLog10(sig1)); }
    SigWrapper getSigFloor(SigWrapper &sig1) { return SigWrapper(sigFloor(sig1)); }
    SigWrapper getSigExp(SigWrapper &sig1) { return SigWrapper(sigExp(sig1)); }
    SigWrapper getSigExp10(SigWrapper &sig1) { return SigWrapper(sigExp10(sig1)); }
    SigWrapper getSigCos(SigWrapper &sig1) { return SigWrapper(sigCos(sig1)); }
    SigWrapper getSigCeil(SigWrapper &sig1) { return SigWrapper(sigCeil(sig1)); }
    SigWrapper getSigAtan(SigWrapper &sig1) { return SigWrapper(sigAtan(sig1)); }
    SigWrapper getSigAsin(SigWrapper &sig1) { return SigWrapper(sigAsin(sig1)); }
    
    SigWrapper getSigRemainder(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigRemainder(sig1, sig2)); }
    SigWrapper getSigPow(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigPow(sig1, sig2)); }
    SigWrapper getSigMin(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigMin(sig1, sig2)); }
    SigWrapper getSigMax(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigMax(sig1, sig2)); }
    SigWrapper getSigFmod(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigFmod(sig1, sig2)); }
    SigWrapper getSigAtan2(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigAtan2(sig1, sig2)); }

    SigWrapper getSigSelf() { return SigWrapper(sigSelf()); }
    
    SigWrapper getSigRecursion(SigWrapper &sig1) { return SigWrapper(sigRecursion(sig1)); }

    SigWrapper getSigButton(std::string &label) { return SigWrapper(sigButton(label)); }
    
    SigWrapper getSigCheckbox(std::string &label) { return SigWrapper(sigCheckbox(label)); }

    SigWrapper getSigVSlider(std::string &label, SigWrapper &sigInit, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sigStep)
        { return SigWrapper(sigVSlider(label, sigInit, sigMin, sigMax, sigStep)); }
    
    SigWrapper getSigHSlider(std::string &label, SigWrapper &sigInit, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sigStep)
        { return SigWrapper(sigHSlider(label, sigInit, sigMin, sigMax, sigStep)); }
    
    SigWrapper getSigNumEntry(std::string &label, SigWrapper &sigInit, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sigStep)
        { return SigWrapper(sigNumEntry(label, sigInit, sigMin, sigMax, sigStep)); }
    
    SigWrapper getSigVBargraph(std::string &label, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sig)
        { return SigWrapper(sigVBargraph(label, sigMin, sigMax, sig)); }
    
    SigWrapper getSigHBargraph(std::string &label, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sig)
        { return SigWrapper(sigHBargraph(label, sigMin, sigMax, sig)); }
    
    SigWrapper getSigAttach(SigWrapper &s1, SigWrapper &s2)
        { return SigWrapper(sigAttach(s1, s2)); }
    
    SigWrapper getSigSampleRate() { return SigWrapper(sigMin(sigReal(192000.0),
                                                             sigMax(sigReal(1.0),
                                                                    sigFConst(SType::kSInt, "fSamplingFreq", "<dummy.h>")))); }
    SigWrapper getSigBufferSize() { return SigWrapper(sigFVar(SType::kSInt, "count", "<math.h>")); }
    
    
    void compileSignals(const std::string& name,
                        std::vector<SigWrapper> &wrappers,
                        std::optional<std::vector<std::string>> in_argv)
    {
        int argc = 0;
        const char** argv = new const char* [256];
        std::string error_msg;
        
        if (in_argv.has_value()) {
            for (auto v : *in_argv) {
                argv[argc++] = v.c_str();
            }
        }
    
        tvec signals;
        for (auto wrapper : wrappers) {
            signals.push_back(wrapper);
        }
        
        dsp_factory_base* factory = createCPPDSPFactoryFromSignals(name,
                                                                   signals,
                                                                   argc,
                                                                   argv,
                                                                   error_msg);
        if (factory) {
            // Print the C++ class
            factory->write(&std::cout);
            delete(factory);
        } else {
            throw std::runtime_error("FaustProcessor: " + error_msg);
        }
    }

};


inline void create_bindings_for_faust_signal(py::module &m) {
    
    using arg = py::arg;
    using kw_only = py::kw_only;
    
    py::return_value_policy returnPolicy = py::return_value_policy::reference;
    
    // todo: for consistency, lookup these descriptions from source.cpp
    auto add_midi_description = "Add a single MIDI note whose note and velocity are integers between 0 and 127. By default, when `beats` is False, the start time and duration are measured in seconds, otherwise beats.";
    auto load_midi_description = "Load MIDI from a file. If `all_events` is True, then all events (not just Note On/Off) will be loaded. By default, when `beats` is False, notes will be converted to absolute times and will not be affected by the Render Engine's BPM. By default, `clear_previous` is True.";
    auto save_midi_description = "After rendering, you can save the MIDI to a file using absolute times (SMPTE format).";
    
    py::class_<SigWrapper>(m, "Signal")
        .def(py::init<float>(), arg("val"), "Init with a float")
        .def(py::init<int>(), arg("val"), "Init with an int")
    // todo: this ignores createLibContext()
        .def("__add__", [](const SigWrapper &s1, SigWrapper &s2) { return SigWrapper(sigAdd((SigWrapper&)s1, s2)); })
        .def("__add__", [](const SigWrapper &s1, float other) { return SigWrapper(sigAdd((SigWrapper&)s1, sigReal(other))); })
        .def("__radd__", [](const SigWrapper &s1, float other) { return SigWrapper(sigAdd((SigWrapper&)s1, sigReal(other))); })
        .def("__add__", [](const SigWrapper &s1, int other) { return SigWrapper(sigAdd((SigWrapper&)s1, sigInt(other))); })
        .def("__radd__", [](const SigWrapper &s1, int other) { return SigWrapper(sigAdd((SigWrapper&)s1, sigInt(other))); })
    
        .def("__sub__", [](const SigWrapper &s1, SigWrapper &s2) { return SigWrapper(sigSub((SigWrapper&)s1, s2)); })
        .def("__sub__", [](const SigWrapper &s1, float other) { return SigWrapper(sigSub((SigWrapper&)s1, sigReal(other))); })
        .def("__rsub__", [](const SigWrapper &s1, float other) { return SigWrapper(sigSub((SigWrapper&)s1, sigReal(other))); })
        .def("__sub__", [](const SigWrapper &s1, int other) { return SigWrapper(sigSub((SigWrapper&)s1, sigInt(other))); })
        .def("__rsub__", [](const SigWrapper &s1, int other) { return SigWrapper(sigSub((SigWrapper&)s1, sigInt(other))); })
    
        .def("__mul__", [](const SigWrapper &s1, SigWrapper &s2) { return SigWrapper(sigMul((SigWrapper&)s1, s2)); })
        .def("__mul__", [](const SigWrapper &s1, float other) { return SigWrapper(sigMul((SigWrapper&)s1, sigReal(other))); })
        .def("__rmul__", [](const SigWrapper &s1, float other) { return SigWrapper(sigMul((SigWrapper&)s1, sigReal(other))); })
        .def("__mul__", [](const SigWrapper &s1, int other) { return SigWrapper(sigMul((SigWrapper&)s1, sigInt(other))); })
        .def("__rmul__", [](const SigWrapper &s1, int other) { return SigWrapper(sigMul((SigWrapper&)s1, sigInt(other))); })
    
        .def("__truediv__", [](const SigWrapper &s1, SigWrapper &s2) { return SigWrapper(sigDiv((SigWrapper&)s1, s2)); })
        .def("__truediv__", [](const SigWrapper &s1, float other) { return SigWrapper(sigDiv((SigWrapper&)s1, sigReal(other))); })
        .def("__rtruediv__", [](const SigWrapper &s1, float other) { return SigWrapper(sigDiv((SigWrapper&)s1, sigReal(other))); })
        .def("__truediv__", [](const SigWrapper &s1, int other) { return SigWrapper(sigDiv((SigWrapper&)s1, sigInt(other))); })
        .def("__rtruediv__", [](const SigWrapper &s1, int other) { return SigWrapper(sigDiv((SigWrapper&)s1, sigInt(other))); })
    
        .def("__mod__", [](const SigWrapper &s1, SigWrapper &s2) { return SigWrapper(sigFmod((SigWrapper&)s1, s2)); })
        .def("__mod__", [](const SigWrapper &s1, float other) { return SigWrapper(sigFmod((SigWrapper&)s1, sigReal(other))); })
        .def("__mod__", [](const SigWrapper &s1, int other) { return SigWrapper(sigFmod((SigWrapper&)s1, sigInt(other))); })
    ;
    
    py::class_<FaustProcessor, ProcessorBase> faustProcessor(m, "FaustProcessor");
    
    faustProcessor
        .def("set_dsp", &FaustProcessor::setDSPFile, arg("filepath"), "Set the FAUST signal process with a file.")
        .def("set_dsp_string", &FaustProcessor::setDSPString, arg("faust_code"),
            "Set the FAUST signal process with a string containing FAUST code.")
        .def("compile", &FaustProcessor::compile, "Compile the FAUST object. You must have already set a dsp file path or dsp string.")
        .def_property("auto_import", &FaustProcessor::getAutoImport, &FaustProcessor::setAutoImport, "The auto import string. Default is `import(\"stdfaust.lib\");`")
        .def("get_parameters_description", &FaustProcessor::getPluginParametersDescription,
            "Get a list of dictionaries describing the parameters of the most recently compiled FAUST code.")
        .def("get_parameter", &FaustProcessor::getParamWithIndex, arg("param_index"))
        .def("get_parameter", &FaustProcessor::getParamWithPath, arg("parameter_path"))
        .def("set_parameter", &FaustProcessor::setParamWithIndex, arg("parameter_index"), arg("value"))
        .def("set_parameter", &FaustProcessor::setAutomationVal, arg("parameter_path"), arg("value"))
        .def_property_readonly("compiled", &FaustProcessor::isCompiled, "Did the most recent DSP code compile?")
        .def_property_readonly("code", &FaustProcessor::code, "Get the most recently compiled Faust DSP code.")
        .def_property("num_voices", &FaustProcessor::getNumVoices, &FaustProcessor::setNumVoices, "The number of voices for polyphony. Set to zero to disable polyphony. One or more enables polyphony.")
        .def_property("group_voices", &FaustProcessor::getGroupVoices, &FaustProcessor::setGroupVoices, "If grouped, all polyphonic voices will share the same parameters. This parameter only matters if polyphony is enabled.")
        .def_property("release_length", &FaustProcessor::getReleaseLength, &FaustProcessor::setReleaseLength, "If using polyphony, specifying the release length accurately can help avoid warnings about voices being stolen.")
        .def_property("faust_libraries_path", &FaustProcessor::getFaustLibrariesPath, &FaustProcessor::setFaustLibrariesPath, "Absolute path to directory containing your custom \".lib\" files containing Faust code.")
        .def_property_readonly("n_midi_events", &FaustProcessor::getNumMidiEvents, "The number of MIDI events stored in the buffer. \
Note that note-ons and note-offs are counted separately.")
        .def("load_midi", &FaustProcessor::loadMidi, arg("filepath"), kw_only(), arg("clear_previous")=true, arg("beats")=false, arg("all_events")=true, load_midi_description)
        .def("clear_midi", &FaustProcessor::clearMidi, "Remove all MIDI notes.")
        .def("add_midi_note", &FaustProcessor::addMidiNote,
            arg("note"), arg("velocity"), arg("start_time"), arg("duration"), kw_only(), arg("beats")=false, add_midi_description)
        .def("save_midi", &FaustProcessor::saveMIDI,
            arg("filepath"), save_midi_description)
        .def("set_soundfiles", &FaustProcessor::setSoundfiles, arg("soundfile_dict"), "Set the audio data that the FaustProcessor can use with the `soundfile` primitive.")
    
        .def("sigInt", &FaustProcessor::getSigInt, arg("val"), "Blah", returnPolicy)
        .def("sigReal", &FaustProcessor::getSigReal, arg("val"), "Blah", returnPolicy)
        .def("sigInput", &FaustProcessor::getSigInput, arg("index"), "Blah", returnPolicy)
        .def("sigDelay", &FaustProcessor::getSigDelay, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigIntCast", &FaustProcessor::getSigIntCast, arg("sig1"), "Blah", returnPolicy)
        .def("sigFloatCast", &FaustProcessor::getSigFloatCast, arg("sig1"), "Blah", returnPolicy)
    
        .def("sigWaveform", &FaustProcessor::getSigWaveform, arg("vals"), "Blah", returnPolicy)
        .def("sigSoundfile", &FaustProcessor::getSigSoundfile, arg("filepath"), arg("sig_read_index"), arg("sig_chan"), arg("sig_part"), "Blah", returnPolicy)
    
        .def("sigSelect2", &FaustProcessor::getSigSelect2, arg("selector"), arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigSelect3", &FaustProcessor::getSigSelect3, arg("selector"), arg("sig1"), arg("sig2"), arg("sig3"), "Blah", returnPolicy)

        .def("sigFConst", &FaustProcessor::getSigFConst, arg("type"), arg("name"), arg("file"), "Blah", returnPolicy)
        .def("sigFVar", &FaustProcessor::getSigFVar, arg("type"), arg("name"), arg("file"), "Blah", returnPolicy)

        .def("sigBinOp", &FaustProcessor::getSigBinOp, arg("op"), arg("x"), arg("y"), "Blah", returnPolicy)

        .def("sigAdd", &FaustProcessor::getSigAdd, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigSub", &FaustProcessor::getSigSub, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigMul", &FaustProcessor::getSigMul, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigDiv", &FaustProcessor::getSigDiv, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigRem", &FaustProcessor::getSigRem, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
    
        .def("sigLeftShift", &FaustProcessor::getSigLeftShift, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigLRightShift", &FaustProcessor::getSigLRightShift, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigARightShift", &FaustProcessor::getSigARightShift, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigGT", &FaustProcessor::getSigGT, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigLT", &FaustProcessor::getSigLT, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigGE", &FaustProcessor::getSigGE, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigLE", &FaustProcessor::getSigLE, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigEQ", &FaustProcessor::getSigEQ, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigNE", &FaustProcessor::getSigNE, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigAND", &FaustProcessor::getSigAND, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigOR", &FaustProcessor::getSigOR, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigXOR", &FaustProcessor::getSigXOR, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigAbs", &FaustProcessor::getSigAbs, arg("sig1"), "Blah", returnPolicy)
        .def("sigAcos", &FaustProcessor::getSigAcos, arg("sig1"), "Blah", returnPolicy)
        .def("sigTan", &FaustProcessor::getSigTan, arg("sig1"), "Blah", returnPolicy)
        .def("sigSqrt", &FaustProcessor::getSigSqrt, arg("sig1"), "Blah", returnPolicy)
        .def("sigSin", &FaustProcessor::getSigSin, arg("sig1"), "Blah", returnPolicy)
        .def("sigRint", &FaustProcessor::getSigRint, arg("sig1"), "Blah", returnPolicy)
        .def("sigLog", &FaustProcessor::getSigLog, arg("sig1"), "Blah", returnPolicy)
        .def("sigLog10", &FaustProcessor::getSigLog10, arg("sig1"), "Blah", returnPolicy)
        .def("sigFloor", &FaustProcessor::getSigFloor, arg("sig1"), "Blah", returnPolicy)
        .def("sigExp", &FaustProcessor::getSigExp, arg("sig1"), "Blah", returnPolicy)
        .def("sigExp10", &FaustProcessor::getSigExp10, arg("sig1"), "Blah", returnPolicy)
        .def("sigCos", &FaustProcessor::getSigCos, arg("sig1"), "Blah", returnPolicy)
        .def("sigCeil", &FaustProcessor::getSigCeil, arg("sig1"), "Blah", returnPolicy)
        .def("sigAtan", &FaustProcessor::getSigAtan, arg("sig1"), "Blah", returnPolicy)
        .def("sigAsin", &FaustProcessor::getSigAsin, arg("sig1"), "Blah", returnPolicy)
    
        .def("sigRemainder", &FaustProcessor::getSigRemainder, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigPow", &FaustProcessor::getSigPow, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigMin", &FaustProcessor::getSigMin, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigMax", &FaustProcessor::getSigMax, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigFmod", &FaustProcessor::getSigFmod, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigAtan2", &FaustProcessor::getSigAtan2, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigSelf", &FaustProcessor::getSigSelf, "Blah", returnPolicy)
        .def("sigRecursion", &FaustProcessor::getSigRecursion, arg("sig"), "Blah", returnPolicy)
    
        .def("sigButton", &FaustProcessor::getSigButton, arg("label"), "Blah", returnPolicy)
        .def("sigCheckbox", &FaustProcessor::getSigCheckbox, arg("label"), "Blah", returnPolicy)

        .def("sigVSlider", &FaustProcessor::getSigVSlider, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("sigHSlider", &FaustProcessor::getSigHSlider, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("sigNumEntry", &FaustProcessor::getSigNumEntry, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("sigVBargraph", &FaustProcessor::getSigVBargraph, arg("label"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("sigHBargraph", &FaustProcessor::getSigHBargraph, arg("label"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("sigAttach", &FaustProcessor::getSigAttach, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
    
        .def("sigSampleRate", &FaustProcessor::getSigSampleRate, "Blah", returnPolicy)
        .def("sigBufferSize", &FaustProcessor::getSigBufferSize, "Blah", returnPolicy)

        .def("compile_signals", &FaustProcessor::compileSignals, arg("name"), arg("signal"), arg("argv")=py::none(), "Blah")
    
        .doc() = "A Faust Processor can compile and execute FAUST code. See https://faust.grame.fr for more information.";

}


#endif

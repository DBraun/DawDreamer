#pragma once
#include "ProcessorBase.h"

# ifdef BUILD_DAWDREAMER_FAUST
#include "faust/dsp/poly-llvm-dsp.h"
#include "faust/dsp/poly-interpreter-dsp.h"
#include "generator/libfaust.h"
#include "faust/gui/APIUI.h"
#include "faust/gui/MidiUI.h"
#include "faust/gui/SoundUI.h"
#include "faust/midi/rt-midi.h"
#include "TMutex.h"
#include "faust/dsp/libfaust-box.h"
#include "faust/dsp/libfaust-signal.h"
#include "faust/dsp/llvm-dsp.h"
    //
#include "faust/dsp/interpreter-dsp.h"
//#include "faust/dsp/poly-dsp.h"
#include "faust/misc.h"
#include "faust/export.h"
#include <iostream>
#include <map>

class MySoundUI: public SoundUI
{

    public:

        virtual void addSoundfile(const char *label, const char *filename, Soundfile **sf_zone)
        {
            // Parse the possible list
            std::string saved_url_real = std::string(label);
            if (fSoundfileMap.find(saved_url_real) == fSoundfileMap.end())
            {
                 // If failure, use 'defaultsound'
                std::cerr << "addSoundfile : soundfile for " << label << " cannot be created !" << std::endl;
                *sf_zone = defaultsound;
                return;
            }

            // Get the soundfile *sf_zone = fSoundfileMap[saved_url_real];
        }

    virtual void addSoundfileFromBuffers(const char *label, std::vector<AudioSampleBuffer> buffers, int sample_rate)
    {
        // Parse the possible list
        std::string saved_url_real = std::string(label);
        if (fSoundfileMap.find(saved_url_real) == fSoundfileMap.end())
        {

            int total_length = 0;
            int numChannels = 1;    // start with at least 1 channel. This may increase due to code below.

            for (auto &buffer: buffers)
            {
                total_length += buffer.getNumSamples();
                numChannels = std::max(numChannels, buffer.getNumChannels());
            }

            total_length += (MAX_SOUNDFILE_PARTS - buffers.size()) *BUFFER_SIZE;

            Soundfile *soundfile = new Soundfile(numChannels, total_length, MAX_CHAN, (int) buffers.size(), false);

            // Manually fill in the soundfile:
            // The following code is a modification of SoundfileReader::createSoundfile and SoundfileReader::readFile

            int offset = 0;

            int i = 0;
            for (auto &buffer: buffers)
            {

                int numSamples = buffer.getNumSamples();

                soundfile->fLength[i] = numSamples;
                soundfile->fSR[i] = sample_rate;
                soundfile->fOffset[i] = offset;

                void *tmpBuffers = alloca(soundfile->fChannels* sizeof(float*));
                soundfile->getBuffersOffsetReal<float> (tmpBuffers, offset);

                for (int chan = 0; chan < buffer.getNumChannels(); chan++)
                {
                    for (int sample = 0; sample < numSamples; sample++)
                    {
                                 // todo: don't assume float
                        // todo: use memcpy or similar to be faster
                        static_cast< float **> (soundfile->fBuffers)[chan][offset + sample] = buffer.getSample(chan, sample);
                    }
                }

                offset += soundfile->fLength[i];
                i++;
            }

            // Complete with empty parts
            for (auto i = (int) buffers.size(); i < MAX_SOUNDFILE_PARTS; i++)
            {
                soundfile->emptyFile(i, offset);
            }

            // Share the same buffers for all other channels so that we have max_chan channels available
            soundfile->shareBuffers(numChannels, MAX_CHAN);

            fSoundfileMap[saved_url_real] = soundfile;
        }
    }
};

struct SigWrapper
{
    CTree * ptr;
    SigWrapper(CTree *ptr): ptr
    {
        ptr
    } {}
    SigWrapper(float val): ptr
    {
        sigReal(val)
    } {}    // todo: this ignores createLibContext();
    SigWrapper(int val): ptr
    {
        sigInt(val)
    } {}    // todo: this ignores createLibContext();
    operator CTree *()
    {
        return ptr;
    }
};

struct BoxWrapper
{
    CTree * ptr;
    BoxWrapper(CTree *ptr): ptr
    {
        ptr
    } {}
    BoxWrapper(float val): ptr
    {
        boxReal(val)
    } {}    // todo: this ignores createLibContext();
    BoxWrapper(int val): ptr
    {
        boxInt(val)
    } {}    // todo: this ignores createLibContext();
    operator CTree *()
    {
        return ptr;
    }
};

class FaustProcessor: public ProcessorBase
{
    public:

        FaustProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock);~FaustProcessor();

    bool canApplyBusesLayout(const juce::AudioProcessor::BusesLayout &layout) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    int
    getTotalNumOutputChannels() override
    {
        if (!m_compileState)
        {
            this->compile();
        }
        return ProcessorBase::getTotalNumOutputChannels();
    }

    int
    getTotalNumInputChannels() override
    {
        if (!m_compileState)
        {
            this->compile();
        }
        return ProcessorBase::getTotalNumInputChannels();
    }

    void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiBuffer) override;

    bool acceptsMidi() const override
    {
        return false;
    }    // todo: faust should be able to play MIDI.
    bool producesMidi() const override
    {
        return false;
    }

    void reset() override;

    void createParameterLayout();    // NB: this is different from other processors because it's called after a Faust DSP file is compiled.

    const juce::String getName() const override
    {
        return "FaustProcessor";
    }

    void automateParameters(AudioPlayHead::PositionInfo &posInfo, int numSamples) override;
    bool setAutomation(std::string parameterName, py::array input, std::uint32_t ppqn) override;

    // faust stuff
    void clear();
    bool compile();
    bool setDSPString(const std::string &code);
    bool setDSPFile(const std::string &path);
    bool setParamWithIndex(const int index, float p);
    float getParamWithIndex(const int index);
    float getParamWithPath(const std::string &n);
    std::string code();
    bool isCompiled()
    {
        return bool(m_compileState);
    };

    py::list getPluginParametersDescription();

    void setNumVoices(int numVoices);
    int getNumVoices();

    void setGroupVoices(bool groupVoices);
    int getGroupVoices();

    void setDynamicVoices(bool dynamicVoices);
    int getDynamicVoices();

    void setAutoImport(const std::string &s)
    {
        m_autoImport = s;
    }
    std::string getAutoImport()
    {
        return m_autoImport;
    }

    bool loadMidi(const std::string &path, bool clearPrevious, bool isBeats, bool allEvents);

    void clearMidi();

    int getNumMidiEvents();

    bool addMidiNote(const uint8 midiNote, const uint8 midiVelocity, const double noteStart, const double noteLength,
                    bool isBeats);

    void setSoundfiles(py::dict);

    double getReleaseLength();

    void setReleaseLength(double sec);

    void setFaustLibrariesPath(std::string faustLibrariesPath)
    {
        m_faustLibrariesPath = faustLibrariesPath;
    }

    std::string getFaustLibrariesPath()
    {
        return m_faustLibrariesPath;
    }

    std::map<std::string, std::vector<juce::AudioSampleBuffer>> m_SoundfileMap;

    void saveMIDI(std::string &savePath);

    private:

        double mySampleRate;

    std::string getPathToFaustLibraries();

    enum CompileState
    {
        kNotCompiled, kMono, kPoly, kSignalMono, kSignalPoly
    };

    CompileState m_compileState;

    protected:

        llvm_dsp_factory *m_factory = nullptr;
    llvm_dsp_poly_factory *m_poly_factory = nullptr;

    dsp *m_dsp = nullptr;
    dsp_poly *m_dsp_poly = nullptr;

    APIUI *m_ui = nullptr;
    MySoundUI *m_soundUI = nullptr;

    rt_midi m_midi_handler;

    int m_numInputChannels = 0;
    int m_numOutputChannels = 0;

    double m_releaseLengthSec = 0.5;

    std::string m_autoImport;
    std::string m_code;
    std::string m_faustLibrariesPath = "";

    int m_nvoices = 0;
    bool m_dynamicVoices = true;
    bool m_groupVoices = true;

    MidiBuffer myMidiBufferQN;
    MidiBuffer myMidiBufferSec;

    MidiMessageSequence myRecordedMidiSequence;    // for fetching by user later.

    MidiMessage myMidiMessageQN;
    MidiMessage myMidiMessageSec;

    int myMidiMessagePositionQN = -1;
    int myMidiMessagePositionSec = -1;

    MidiBuffer::Iterator *myMidiIteratorQN = nullptr;
    MidiBuffer::Iterator *myMidiIteratorSec = nullptr;

    bool myIsMessageBetweenQN = false;
    bool myIsMessageBetweenSec = false;

    bool myMidiEventsDoRemainQN = false;
    bool myMidiEventsDoRemainSec = false;

    juce::AudioSampleBuffer oneSampleInBuffer;
    juce::AudioSampleBuffer oneSampleOutBuffer;

    std::map<int, int> m_map_juceIndex_to_faustIndex;
    std::map<int, std::string > m_map_juceIndex_to_parAddress;

    TMutex guiUpdateMutex;

    std::string getTarget();

    // public libfaust signal API
    public:

        void compileSignals(const std::string &name,
            std::vector<SigWrapper> &wrappers,
            std::optional<std::vector<std::string>> in_argv);

    //       // todo:
    //    tvec getBoxToSignals(BoxWrapper& box1)
    //        {
    //        std::string error_msg;
    //        auto signals = boxesToSignals(box1, error_msg);
    //        return signals;
    //    }

    void compileBox(const std::string &name,
        BoxWrapper &box,
        std::optional<std::vector<std::string>> in_argv);

    std::tuple<BoxWrapper, int, int> dspToBox(const std::string &dsp_content)
    {
        int inputs = 0;
        int outputs = 0;
        std::string error_msg = "";
        std::string dsp_content2 = std::string("import(\"stdfaust.lib\");\n") + dsp_content;
        Box box = DSPToBoxes(dsp_content2, inputs, outputs, error_msg);
        if (error_msg != "")
        {
            throw std::runtime_error(error_msg);
        }

        return std::tuple<BoxWrapper, int, int> (BoxWrapper(box), inputs, outputs);
    }

    void boxToCPP(BoxWrapper & box)
    {

        auto pathToFaustLibraries = getPathToFaustLibraries();

        if (pathToFaustLibraries == "")
        {
            throw std::runtime_error("FaustProcessor::compile(): Error for path for faust libraries: " + pathToFaustLibraries);
        }

        int argc = 0;
        const char **argv = new
        const char *[256];

        argv[argc++] = "-I";
        argv[argc++] = pathToFaustLibraries.c_str();

        if (m_faustLibrariesPath != "")
        {
            argv[argc++] = "-I";
            argv[argc++] = m_faustLibrariesPath.c_str();
        }

        std::string error_msg;

        dsp_factory_base *factory = createCPPDSPFactoryFromBoxes("test",
            box,
            argc,
            argv,
            error_msg);

        for (int i = 0; i < argc; i++)
        {
            argv[i] = NULL;
        }
        delete[] argv;
        argv = nullptr;

        if (factory)
        {
            // Print the C++ class
            factory->write(&std::cout);
            delete(factory);
        }
        else
        {
            throw std::runtime_error(error_msg);
        }
    }
};

inline void create_bindings_for_faust_signal(py::module &m)
{

    using arg = py::arg;
    using kw_only = py::kw_only;

    py::return_value_policy returnPolicy = py::return_value_policy::reference;

    // todo: for consistency, lookup these descriptions from source.cpp
    auto add_midi_description = "Add a single MIDI note whose note and velocity are integers between 0 and 127. By default, when `beats` is False, the start time and duration are measured in seconds, otherwise beats.";
    auto load_midi_description = "Load MIDI from a file. If `all_events` is True, then all events (not just Note On/Off) will be loaded. By default, when `beats` is False, notes will be converted to absolute times and will not be affected by the Render Engine's BPM. By default, `clear_previous` is True.";
    auto save_midi_description = "After rendering, you can save the MIDI to a file using absolute times (SMPTE format).";

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
        .def_property("dynamic_voices", &FaustProcessor::getDynamicVoices, &FaustProcessor::setDynamicVoices, "If enabled (default), voices are dynamically enabled and disabled to save computation. This parameter only matters if polyphony is enabled.")
        .def_property("release_length", &FaustProcessor::getReleaseLength, &FaustProcessor::setReleaseLength, "If using polyphony, specifying the release length accurately can help avoid warnings about voices being stolen.")
        .def_property("faust_libraries_path", &FaustProcessor::getFaustLibrariesPath, &FaustProcessor::setFaustLibrariesPath, "Absolute path to directory containing your custom \".lib\" files containing Faust code.")
        .def_property_readonly("n_midi_events", &FaustProcessor::getNumMidiEvents, "The number of MIDI events stored in the buffer. \
Note that note-ons and note-offs are counted separately.")
        .def("load_midi", &FaustProcessor::loadMidi, arg("filepath"), kw_only(), arg("clear_previous") = true, arg("beats") = false, arg("all_events") = true, load_midi_description)
        .def("clear_midi", &FaustProcessor::clearMidi, "Remove all MIDI notes.")
        .def("add_midi_note", &FaustProcessor::addMidiNote,
            arg("note"), arg("velocity"), arg("start_time"), arg("duration"), kw_only(), arg("beats") = false, add_midi_description)
        .def("save_midi", &FaustProcessor::saveMIDI,
            arg("filepath"), save_midi_description)
        .def("set_soundfiles", &FaustProcessor::setSoundfiles, arg("soundfile_dict"), "Set the audio data that the FaustProcessor can use with the `soundfile` primitive.")
        .def("set_dsp", &FaustProcessor::setDSPFile, arg("filepath"), "Set the FAUST box process with a file.")
        .def("set_dsp_string", &FaustProcessor::setDSPString, arg("faust_code"),
            "Set the FAUST box process with a string containing FAUST code.")
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
        .def("load_midi", &FaustProcessor::loadMidi, arg("filepath"), kw_only(), arg("clear_previous") = true, arg("beats") = false, arg("all_events") = true, load_midi_description)
        .def("clear_midi", &FaustProcessor::clearMidi, "Remove all MIDI notes.")
        .def("add_midi_note", &FaustProcessor::addMidiNote,
            arg("note"), arg("velocity"), arg("start_time"), arg("duration"), kw_only(), arg("beats") = false, add_midi_description)
        .def("save_midi", &FaustProcessor::saveMIDI,
            arg("filepath"), save_midi_description)
        .def("set_soundfiles", &FaustProcessor::setSoundfiles, arg("soundfile_dict"), "Set the audio data that the FaustProcessor can use with the `soundfile` primitive.")

        .def("compile_signals", &FaustProcessor::compileSignals, arg("name"), arg("signal"), arg("argv") = py::none(), "Blah", returnPolicy)
        .def("compile_box", &FaustProcessor::compileBox, arg("name"), arg("box"), arg("argv") = py::none(), "Blah", returnPolicy)
        .def("boxToCPP", &FaustProcessor::boxToCPP, arg("box"), "Print C++ code of a box.", returnPolicy)
        .def("dsp_to_box", &FaustProcessor::dspToBox, arg("dsp_code"), "Convert Faust DSP code to a tuple containing a Box, number of inputs, and outputs.", returnPolicy)

        .doc() = "A Faust Processor can compile and execute FAUST code. See https://faust.grame.fr for more information.";

    auto faust = m.def_submodule("faust");

    py::class_<SigWrapper> (faust, "Signal")
        .def(py::init<float> (), arg("val"), "Init with a float")
        .def(py::init<int> (), arg("val"), "Init with an int")
        // todo: this ignores createLibContext()
        .def("__add__", [](const SigWrapper &s1, SigWrapper &s2)
        {
            return SigWrapper(sigAdd((SigWrapper &) s1, s2));
})
        .def("__add__", [](const SigWrapper &s1, float other)
        {
            return SigWrapper(sigAdd((SigWrapper &) s1, sigReal(other)));
})
        .def("__radd__", [](const SigWrapper &s1, float other)
        {
            return SigWrapper(sigAdd((SigWrapper &) s1, sigReal(other)));
})
        .def("__add__", [](const SigWrapper &s1, int other)
        {
            return SigWrapper(sigAdd((SigWrapper &) s1, sigInt(other)));
})
        .def("__radd__", [](const SigWrapper &s1, int other)
        {
            return SigWrapper(sigAdd((SigWrapper &) s1, sigInt(other)));
})

        .def("__sub__", [](const SigWrapper &s1, SigWrapper &s2)
        {
            return SigWrapper(sigSub((SigWrapper &) s1, s2));
})
        .def("__sub__", [](const SigWrapper &s1, float other)
        {
            return SigWrapper(sigSub((SigWrapper &) s1, sigReal(other)));
})
        .def("__rsub__", [](const SigWrapper &s1, float other)
        {
            return SigWrapper(sigSub((SigWrapper &) s1, sigReal(other)));
})
        .def("__sub__", [](const SigWrapper &s1, int other)
        {
            return SigWrapper(sigSub((SigWrapper &) s1, sigInt(other)));
})
        .def("__rsub__", [](const SigWrapper &s1, int other)
        {
            return SigWrapper(sigSub((SigWrapper &) s1, sigInt(other)));
})

        .def("__mul__", [](const SigWrapper &s1, SigWrapper &s2)
        {
            return SigWrapper(sigMul((SigWrapper &) s1, s2));
})
        .def("__mul__", [](const SigWrapper &s1, float other)
        {
            return SigWrapper(sigMul((SigWrapper &) s1, sigReal(other)));
})
        .def("__rmul__", [](const SigWrapper &s1, float other)
        {
            return SigWrapper(sigMul((SigWrapper &) s1, sigReal(other)));
})
        .def("__mul__", [](const SigWrapper &s1, int other)
        {
            return SigWrapper(sigMul((SigWrapper &) s1, sigInt(other)));
})
        .def("__rmul__", [](const SigWrapper &s1, int other)
        {
            return SigWrapper(sigMul((SigWrapper &) s1, sigInt(other)));
})

        .def("__truediv__", [](const SigWrapper &s1, SigWrapper &s2)
        {
            return SigWrapper(sigDiv((SigWrapper &) s1, s2));
})
        .def("__truediv__", [](const SigWrapper &s1, float other)
        {
            return SigWrapper(sigDiv((SigWrapper &) s1, sigReal(other)));
})
        .def("__rtruediv__", [](const SigWrapper &s1, float other)
        {
            return SigWrapper(sigDiv((SigWrapper &) s1, sigReal(other)));
})
        .def("__truediv__", [](const SigWrapper &s1, int other)
        {
            return SigWrapper(sigDiv((SigWrapper &) s1, sigInt(other)));
})
        .def("__rtruediv__", [](const SigWrapper &s1, int other)
        {
            return SigWrapper(sigDiv((SigWrapper &) s1, sigInt(other)));
})

        .def("__mod__", [](const SigWrapper &s1, SigWrapper &s2)
        {
            return SigWrapper(sigFmod((SigWrapper &) s1, s2));
})
        .def("__mod__", [](const SigWrapper &s1, float other)
        {
            return SigWrapper(sigFmod((SigWrapper &) s1, sigReal(other)));
})
        .def("__mod__", [](const SigWrapper &s1, int other)
        {
            return SigWrapper(sigFmod((SigWrapper &) s1, sigInt(other)));
    });

    py::class_<BoxWrapper> (faust, "Box")
        .def(py::init<float> (), arg("val"), "Init with a float", returnPolicy)
        .def(py::init<int> (), arg("val"), "Init with an int", returnPolicy)
        // todo: this ignores createLibContext()
        .def("__add__", [](const BoxWrapper &box1, BoxWrapper &box2)
        {
            return BoxWrapper(boxAdd((BoxWrapper &) box1, box2));
        }, returnPolicy)
        .def("__add__", [](const BoxWrapper &box1, float other)
        {
            return BoxWrapper(boxAdd((BoxWrapper &) box1, boxReal(other)));
        }, returnPolicy)
        .def("__radd__", [](const BoxWrapper &box1, float other)
        {
            return BoxWrapper(boxAdd((BoxWrapper &) box1, boxReal(other)));
        }, returnPolicy)
        .def("__add__", [](const BoxWrapper &box1, int other)
        {
            return BoxWrapper(boxAdd((BoxWrapper &) box1, boxInt(other)));
        }, returnPolicy)
        .def("__radd__", [](const BoxWrapper &box1, int other)
        {
            return BoxWrapper(boxAdd((BoxWrapper &) box1, boxInt(other)));
        }, returnPolicy)

        .def("__sub__", [](const BoxWrapper &box1, BoxWrapper &box2)
        {
            return BoxWrapper(boxSub((BoxWrapper &) box1, box2));
        }, returnPolicy)
        .def("__sub__", [](const BoxWrapper &box1, float other)
        {
            return BoxWrapper(boxSub((BoxWrapper &) box1, boxReal(other)));
        }, returnPolicy)
        .def("__rsub__", [](const BoxWrapper &box1, float other)
        {
            return BoxWrapper(boxSub((BoxWrapper &) box1, boxReal(other)));
        }, returnPolicy)
        .def("__sub__", [](const BoxWrapper &box1, int other)
        {
            return BoxWrapper(boxSub((BoxWrapper &) box1, boxInt(other)));
        }, returnPolicy)
        .def("__rsub__", [](const BoxWrapper &box1, int other)
        {
            return BoxWrapper(boxSub((BoxWrapper &) box1, boxInt(other)));
        }, returnPolicy)

        .def("__mul__", [](const BoxWrapper &box1, BoxWrapper &box2)
        {
            return BoxWrapper(boxMul((BoxWrapper &) box1, box2));
        }, returnPolicy)
        .def("__mul__", [](const BoxWrapper &box1, float other)
        {
            return BoxWrapper(boxMul((BoxWrapper &) box1, boxReal(other)));
        }, returnPolicy)
        .def("__rmul__", [](const BoxWrapper &box1, float other)
        {
            return BoxWrapper(boxMul((BoxWrapper &) box1, boxReal(other)));
        }, returnPolicy)
        .def("__mul__", [](const BoxWrapper &box1, int other)
        {
            return BoxWrapper(boxMul((BoxWrapper &) box1, boxInt(other)));
        }, returnPolicy)
        .def("__rmul__", [](const BoxWrapper &box1, int other)
        {
            return BoxWrapper(boxMul((BoxWrapper &) box1, boxInt(other)));
        }, returnPolicy)

        .def("__truediv__", [](const BoxWrapper &box1, BoxWrapper &box2)
        {
            return BoxWrapper(boxDiv((BoxWrapper &) box1, box2));
        }, returnPolicy)
        .def("__truediv__", [](const BoxWrapper &box1, float other)
        {
            return BoxWrapper(boxDiv((BoxWrapper &) box1, boxReal(other)));
        }, returnPolicy)
        .def("__rtruediv__", [](const BoxWrapper &box1, float other)
        {
            return BoxWrapper(boxDiv((BoxWrapper &) box1, boxReal(other)));
        }, returnPolicy)
        .def("__truediv__", [](const BoxWrapper &box1, int other)
        {
            return BoxWrapper(boxDiv((BoxWrapper &) box1, boxInt(other)));
        }, returnPolicy)
        .def("__rtruediv__", [](const BoxWrapper &box1, int other)
        {
            return BoxWrapper(boxDiv((BoxWrapper &) box1, boxInt(other)));
        }, returnPolicy)

        .def("__mod__", [](const BoxWrapper &box1, BoxWrapper &box2)
        {
            return BoxWrapper(boxFmod((BoxWrapper &) box1, box2));
        }, returnPolicy)
        .def("__mod__", [](const BoxWrapper &box1, float other)
        {
            return BoxWrapper(boxFmod((BoxWrapper &) box1, boxReal(other)));
        }, returnPolicy)
        .def("__mod__", [](const BoxWrapper &box1, int other)
        {
            return BoxWrapper(boxFmod((BoxWrapper &) box1, boxInt(other)));
        }, returnPolicy);

    faust
        .def("boxInt", [](int val)
        {
            return BoxWrapper(boxInt(val));
        }, arg("val"), "Blah", returnPolicy)
        .def("boxReal", [](double val)
        {
            return BoxWrapper(boxReal(val));
        }, arg("val"), "Blah", returnPolicy)
        .def("boxWire", []()
        {
            return BoxWrapper(boxWire());
        }, "Blah", returnPolicy)
        .def("boxCut", []()
        {
            return BoxWrapper(boxCut());
        }, "Blah", returnPolicy)

        .def("boxSeq", [](BoxWrapper &box1, BoxWrapper &box2)
        {
            return BoxWrapper(boxSeq(box1, box2));
        }, arg("box1"), arg("box2"), "The sequential composition of two blocks (e.g., A:B) expects: outputs(A)=inputs(B)", returnPolicy)

        .def("boxPar", [](BoxWrapper &box1, BoxWrapper &box2)
        {
            return BoxWrapper(boxPar(box1, box2));
        }, arg("box1"), arg("box2"), "The parallel composition of two blocks (e.g., A,B). It places the two block-diagrams one on top of the other, without connections.", returnPolicy)
        .def("boxPar3", [](BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3)
        {
            return BoxWrapper(boxPar3(box1, box2, box3));
        }, arg("box1"), arg("box2"), arg("box3"), "The parallel composition of three blocks (e.g., A,B,C).", returnPolicy)
        .def("boxPar4", [](BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3, BoxWrapper &box4)
        {
            return BoxWrapper(boxPar4(box1, box2, box3, box4));
        }, arg("box1"), arg("box2"), arg("box3"), arg("box4"), "The parallel composition of four blocks (e.g., A,B,C,D).", returnPolicy)
        .def("boxPar5", [](BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3, BoxWrapper &box4, BoxWrapper &box5)
        {
            return BoxWrapper(boxPar5(box1, box2, box3, box4, box5));
        }, arg("box1"), arg("box2"), arg("box3"), arg("box4"), arg("box5"), "The parallel composition of five blocks (e.g., A,B,C,D,E).", returnPolicy)

        .def("boxSplit", [](BoxWrapper &box1, BoxWrapper &box2)
        {
            return BoxWrapper(boxSplit(box1, box2));
        }, arg("box1"), arg("box2"), "The split composition (e.g., A<:B) operator is used to distribute the outputs of A to the inputs of B. The number of inputs of B must be a multiple of the number of outputs of A: outputs(A).k=inputs(B)", returnPolicy)
        .def("boxMerge", [](BoxWrapper &box1, BoxWrapper &box2)
        {
            return BoxWrapper(boxMerge(box1, box2));
        }, arg("box1"), arg("box2"), "The merge composition (e.g., A:>B) is the dual of the split composition. The number of outputs of A must be a multiple of the number of inputs of B: outputs(A)=k.inputs(B)", returnPolicy)

        .def("boxRoute", [](BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3)
        {
            return BoxWrapper(boxRoute(box1, box2, box3));
        }, arg("box_n"), arg("box_m"), arg("box_r"), "Blah", returnPolicy)

        .def("boxDelay", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            if (box1.has_value() && box2.has_value())
            {
                return BoxWrapper(boxDelay(*box1, *box2));
            }
            else
            {
                return BoxWrapper(boxDelay());
            }
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)

        .def("boxIntCast", [](std::optional<BoxWrapper> box1)
        {
            if (box1.has_value())
            {
                return BoxWrapper(boxIntCast(*box1));
            }
            else
            {
                return BoxWrapper(boxIntCast());
            }
        }, arg("box1") = py::none(), "Blah", returnPolicy)

        .def("boxFloatCast", [](std::optional<BoxWrapper> box1)
        {
            if (box1.has_value())
            {
                return BoxWrapper(boxFloatCast(*box1));
            }
            else
            {
                return BoxWrapper(boxFloatCast());
            }
        }, arg("box1") = py::none(), "Blah", returnPolicy)

        .def("boxReadOnlyTable", [](std::optional<BoxWrapper> n, std::optional<BoxWrapper> init, std::optional<BoxWrapper> ridx)
        {
            if (n.has_value() && init.has_value() && ridx.has_value())
            {
                return BoxWrapper(boxReadOnlyTable(boxIntCast(*n), *init, boxIntCast(*ridx)));
            }
            else
            {
                return BoxWrapper(boxReadOnlyTable());
            }
        }, arg("n") = py::none(), arg("init") = py::none(), arg("ridx") = py::none(), "Blah", returnPolicy)

        .def("boxWriteReadTable", [](std::optional<BoxWrapper> n, std::optional<BoxWrapper> init, std::optional<BoxWrapper> widx, std::optional<BoxWrapper> wsig, std::optional<BoxWrapper> ridx)
        {
            if (n.has_value() && init.has_value() && widx.has_value() && wsig.has_value() && ridx.has_value())
            {
                return BoxWrapper(boxWriteReadTable(boxIntCast(*n), *init, boxIntCast(*widx), boxIntCast(*wsig), boxIntCast(*ridx)));
            }
            else
            {
                return BoxWrapper(boxWriteReadTable());
            }
        }, arg("n") = py::none(), arg("init") = py::none(), arg("widx") = py::none(), arg("wsig") = py::none(), arg("ridx") = py::none(), "Blah", returnPolicy)

        .def("boxWaveform", [](std::vector<float> vals)
        {
            tvec waveform;
            for (auto &val: vals)
            {
                waveform.push_back(boxReal(val));
            }
            return BoxWrapper(boxWaveform(waveform));
        }, arg("vals"), "Blah", returnPolicy)
        .def("boxSoundfile", [](std::string &label, BoxWrapper &chan, std::optional<BoxWrapper> part, std::optional<BoxWrapper> rdx)
        {
            if (part.has_value() && rdx.has_value())
            {
                return BoxWrapper(boxSoundfile(label, boxIntCast(chan), boxIntCast(*part), boxIntCast(*rdx)));
            }
            else
            {
                return BoxWrapper(boxSoundfile(label, chan));
            }
        }, arg("filepath"), arg("chan"), arg("part") = py::none(), arg("ridx") = py::none(), "Blah", returnPolicy)

        .def("boxSelect2", [](std::optional<BoxWrapper> selector, std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            if (selector.has_value() && box1.has_value() && box2.has_value())
            {
                return BoxWrapper(boxSelect2(*selector, *box1, *box2));
            }
            else
            {
                return BoxWrapper(boxSelect2());
            }
        }, arg("selector") = py::none(), arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)

        .def("boxSelect3", [](std::optional<BoxWrapper> selector, std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2, std::optional<BoxWrapper> box3)
        {
            if (selector.has_value() && box1.has_value() && box2.has_value())
            {
                return BoxWrapper(boxSelect3(*selector, *box1, *box2, *box3));
            }
            else
            {
                return BoxWrapper(boxSelect3());
            }
        }, arg("selector") = py::none(), arg("box1") = py::none(), arg("box2") = py::none(), arg("box3") = py::none(), "Blah", returnPolicy)

        .def("boxFConst", [](SType type, const std::string &name, const std::string &file)
        {
            return BoxWrapper(boxFConst(type, name, file));
        }, arg("type"), arg("name"), arg("file"), "Blah", returnPolicy)
        .def("boxFVar", [](SType type, const std::string &name, const std::string &file)
        {
            return BoxWrapper(boxFVar(type, name, file));
        }, arg("type"), arg("name"), arg("file"), "Blah", returnPolicy)

        .def("boxBinOp", [](SOperator op, std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            if (box1.has_value() && box2.has_value())
            {
                return BoxWrapper(boxBinOp(op, *box1, *box2));
            }
            else
            {
                return BoxWrapper(boxBinOp(op));
            }
        }, arg("op"), arg("x") = py::none(), arg("y") = py::none(), "Blah", returnPolicy)

        .def("boxAdd", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxAdd(*box1, *box2)) : BoxWrapper(boxAdd());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxSub", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxSub(*box1, *box2)) : BoxWrapper(boxSub());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxMul", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxMul(*box1, *box2)) : BoxWrapper(boxMul());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxDiv", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxDiv(*box1, *box2)) : BoxWrapper(boxDiv());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxRem", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxRem(*box1, *box2)) : BoxWrapper(boxRem());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)

        .def("boxLeftShift", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxLeftShift(*box1, *box2)) : BoxWrapper(boxLeftShift());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxLRightShift", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxLRightShift(*box1, *box2)) : BoxWrapper(boxLRightShift());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxARightShift", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxARightShift(*box1, *box2)) : BoxWrapper(boxARightShift());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)

        .def("boxGT", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxGT(*box1, *box2)) : BoxWrapper(boxGT());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxLT", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxLT(*box1, *box2)) : BoxWrapper(boxLT());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxGE", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxGE(*box1, *box2)) : BoxWrapper(boxGE());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxLE", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxLE(*box1, *box2)) : BoxWrapper(boxLE());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxEQ", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxEQ(*box1, *box2)) : BoxWrapper(boxEQ());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxNE", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxNE(*box1, *box2)) : BoxWrapper(boxNE());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)

        .def("boxAND", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxAND(*box1, *box2)) : BoxWrapper(boxAND());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxOR", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxOR(*box1, *box2)) : BoxWrapper(boxOR());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxXOR", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxXOR(*box1, *box2)) : BoxWrapper(boxXOR());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)

        .def("boxAbs", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxAbs(*box1) : boxAbs());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxAcos", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxAcos(*box1) : boxAcos());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxTan", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxTan(*box1) : boxTan());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxSqrt", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxSqrt(*box1) : boxSqrt());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxSin", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxSin(*box1) : boxSin());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxRint", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxRint(*box1) : boxRint());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxLog", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxLog(*box1) : boxLog());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxLog10", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxLog10(*box1) : boxLog10());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxFloor", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxFloor(*box1) : boxFloor());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxExp", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxExp(*box1) : boxExp());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxExp10", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxExp10(*box1) : boxExp10());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxCos", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxCos(*box1) : boxCos());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxCeil", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxCeil(*box1) : boxCeil());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxAtan", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxAtan(*box1) : boxAtan());
        }, arg("box1") = py::none(), "Blah", returnPolicy)
        .def("boxAsin", [](std::optional<BoxWrapper> box1)
        {
            return BoxWrapper(box1.has_value() ? boxAsin(*box1) : boxAsin());
        }, arg("box1") = py::none(), "Blah", returnPolicy)

        .def("boxRemainder", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxRemainder(*box1, *box2)) : BoxWrapper(boxRemainder());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxPow", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxPow(*box1, *box2)) : BoxWrapper(boxPow());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxMin", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxMin(*box1, *box2)) : BoxWrapper(boxMin());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxMax", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxMax(*box1, *box2)) : BoxWrapper(boxMax());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxFmod", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxFmod(*box1, *box2)) : BoxWrapper(boxFmod());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)
        .def("boxAtan2", [](std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2)
        {
            return box1.has_value() && box2.has_value() ? BoxWrapper(boxAtan2(*box1, *box2)) : BoxWrapper(boxAtan2());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)

        .def("boxRec", [](BoxWrapper &box1, BoxWrapper &box2)
        {
            return BoxWrapper(boxRec(box1, box2));
        }, arg("box1"), arg("box2"), "The recursive composition (e.g., A~B) is used to create cycles in the block-diagram in order to express recursive computations. It is the most complex operation in terms of connections: outputs(A)≥inputs(B) and inputs(A)≥outputs(B)", returnPolicy)

        .def("boxButton", [](std::string &label)
        {
            return BoxWrapper(boxButton(label));
        }, arg("label"), "Blah", returnPolicy)
        .def("boxCheckbox", [](std::string &label)
        {
            return BoxWrapper(boxCheckbox(label));
        }, arg("label"), "Blah", returnPolicy)

        .def("boxVSlider", [](std::string &label, BoxWrapper &boxInit, BoxWrapper &boxMin, BoxWrapper &boxMax, BoxWrapper &boxStep)
        {
            return BoxWrapper(boxVSlider(label, boxInit, boxMin, boxMax, boxStep));
        }, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("boxVSlider", [](std::string &label, float init, float minVal, float maxVal, float step)
        {
            return BoxWrapper(boxVSlider(label, boxReal(init), boxReal(minVal), boxReal(maxVal), boxReal(step)));
        }, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("boxHSlider", [](std::string &label, BoxWrapper &boxInit, BoxWrapper &boxMin, BoxWrapper &boxMax, BoxWrapper &boxStep)
        {
            return BoxWrapper(boxHSlider(label, boxInit, boxMin, boxMax, boxStep));
        }, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("boxHSlider", [](std::string &label, float init, float minVal, float maxVal, float step)
        {
            return BoxWrapper(boxHSlider(label, boxReal(init), boxReal(minVal), boxReal(maxVal), boxReal(step)));
        }, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("boxNumEntry", [](std::string &label, BoxWrapper &boxInit, BoxWrapper &boxMin, BoxWrapper &boxMax, BoxWrapper &boxStep)
        {
            return BoxWrapper(boxNumEntry(label, boxInit, boxMin, boxMax, boxStep));
        }, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("boxNumEntry", [](std::string &label, float init, float minVal, float maxVal, float step)
        {
            return BoxWrapper(boxNumEntry(label, boxReal(init), boxReal(minVal), boxReal(maxVal), boxReal(step)));
        }, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("boxVBargraph", [](std::string &label, BoxWrapper &boxMin, BoxWrapper &boxMax, BoxWrapper &box)
        {
            return BoxWrapper(boxVBargraph(label, boxMin, boxMax, box));
        }, arg("label"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("boxHBargraph", [](std::string &label, BoxWrapper &boxMin, BoxWrapper &boxMax, BoxWrapper &box)
        {
            return BoxWrapper(boxHBargraph(label, boxMin, boxMax, box));
        }, arg("label"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("boxAttach", [](std::optional<BoxWrapper> s1, std::optional<BoxWrapper> s2)
        {
            return BoxWrapper((s1.has_value() && s2.has_value()) ? boxAttach(*s1, *s2) : boxAttach());
        }, arg("box1") = py::none(), arg("box2") = py::none(), "Blah", returnPolicy)

        .def("boxSampleRate", []()
        {
            return BoxWrapper(boxMin(boxReal(192000.0),
                boxMax(boxReal(1.0),
                    boxFConst(SType::kSInt, "fSamplingFreq", "<math.h>"))));
        }, "Blah", returnPolicy)
        .def("boxBufferSize", []()
        {
            return BoxWrapper(boxFVar(SType::kSInt, "count", "<math.h>"));
        }, "Blah", returnPolicy)

        .def("boxFromDSP", [](const std::string &dsp_content)
        {
            int inputs = 0;
            int outputs = 0;
            std::string error_msg = "";
            std::string dsp_content2 = std::string("import(\"stdfaust.lib\");\n") + dsp_content;
            Box box = DSPToBoxes(dsp_content2, inputs, outputs, error_msg);
            if (error_msg != "")
            {
                throw std::runtime_error(error_msg);
            }

            return BoxWrapper(box);
        }, arg("dsp_code"), "Convert Faust DSP code to a Box.", returnPolicy)
        // SIGNAL API
        .def("sigInt", [](int val)
        {
            return SigWrapper(sigInt(val));
        }, arg("val"), "Blah", returnPolicy)
        .def("sigReal", [](double val)
        {
            return SigWrapper(sigReal(val));
        }, arg("val"), "Blah", returnPolicy)
        .def("sigInput", [](int index)
        {
            return SigWrapper(sigInput(index));
        }, arg("index"), "Blah", returnPolicy)
        .def("sigDelay", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigDelay(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigIntCast", [](SigWrapper & sig1)
        {
            return SigWrapper(sigIntCast(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigFloatCast", [](SigWrapper & sig1)
        {
            return SigWrapper(sigFloatCast(sig1));
        }, arg("sig1"), "Blah", returnPolicy)

        .def("sigReadOnlyTable", [](SigWrapper &n, SigWrapper &init, SigWrapper &ridx)
        {
            return SigWrapper(sigReadOnlyTable(n, init, sigIntCast(ridx)));
        }, arg("n"), arg("init"), arg("ridx"), "Blah", returnPolicy)
        .def("sigWriteReadTable", [](SigWrapper &n, SigWrapper &init, SigWrapper &widx, SigWrapper &wsig, SigWrapper &ridx)
        {
            return SigWrapper(sigWriteReadTable(n, init, sigIntCast(widx), sigIntCast(wsig), sigIntCast(ridx)));
        }, arg("n"), arg("init"), arg("widx"), arg("wsig"), arg("ridx"), "Blah", returnPolicy)

        .def("sigWaveform", [](std::vector<float> vals)
        {
            tvec waveform;
            for (auto &val: vals)
            {
                waveform.push_back(sigReal(val));
            }
            auto mySigWaveform = sigWaveform(waveform);

            auto result = std::vector < SigWrapper>
            {
                SigWrapper(sigInt((int) waveform.size())),
                SigWrapper(mySigWaveform)
            };

            return result;
        }, arg("vals"), "Blah", returnPolicy)
        .def("sigSoundfile", [](std::string &name, SigWrapper &rdx, SigWrapper &chan, SigWrapper &part)
        {
            // Soundfile definition
            Signal sf = sigSoundfile(name);
            Signal partInt = sigIntCast(part);
            // Wrapped index to avoid reading outside the buffer
            Signal wridx = sigIntCast(sigMax(sigInt(0),
                sigMin(sigIntCast(rdx),
                    sigSub(sigSoundfileLength(sf, partInt),
                        sigInt(1)))));

            auto result = std::vector < SigWrapper>
            {
                SigWrapper(sigSoundfileLength(sf, partInt)),
                SigWrapper(sigSoundfileRate(sf, partInt)),
                SigWrapper(sigSoundfileBuffer(sf, sigIntCast(chan), partInt, wridx))
            };

            return result;
        }, arg("filepath"), arg("sig_read_index"), arg("sig_chan"), arg("sig_part"), "Blah", returnPolicy)

        .def("sigSelect2", [](SigWrapper &selector, SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigSelect2(selector, sig1, sig2));
        }, arg("selector"), arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigSelect3", [](SigWrapper &selector, SigWrapper &sig1, SigWrapper &sig2, SigWrapper &sig3)
        {
            return SigWrapper(sigSelect3(selector, sig1, sig2, sig3));
        }, arg("selector"), arg("sig1"), arg("sig2"), arg("sig3"), "Blah", returnPolicy)

        .def("sigFConst", [](SType type, const std::string &name, const std::string &file)
        {
            return SigWrapper(sigFConst(type, name, file));
        }, arg("type"), arg("name"), arg("file"), "Blah", returnPolicy)
        .def("sigFVar", [](SType type, const std::string &name, const std::string &file)
        {
            return SigWrapper(sigFVar(type, name, file));
        }, arg("type"), arg("name"), arg("file"), "Blah", returnPolicy)

        .def("sigBinOp", [](SOperator op, SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigBinOp(op, sig1, sig2));
        }, arg("op"), arg("x"), arg("y"), "Blah", returnPolicy)

        .def("sigAdd", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigAdd(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigSub", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigSub(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigMul", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigMul(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigDiv", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigDiv(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigRem", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigRem(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigLeftShift", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigLeftShift(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigLRightShift", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigLRightShift(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigARightShift", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigARightShift(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigGT", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigGT(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigLT", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigLT(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigGE", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigGE(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigLE", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigLE(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigEQ", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigEQ(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigNE", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigNE(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigAND", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigAND(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigOR", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigOR(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigXOR", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigXOR(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigAbs", [](SigWrapper & sig1)
        {
            return SigWrapper(sigAbs(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigAcos", [](SigWrapper & sig1)
        {
            return SigWrapper(sigAcos(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigTan", [](SigWrapper & sig1)
        {
            return SigWrapper(sigTan(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigSqrt", [](SigWrapper & sig1)
        {
            return SigWrapper(sigSqrt(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigSin", [](SigWrapper & sig1)
        {
            return SigWrapper(sigSin(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigRint", [](SigWrapper & sig1)
        {
            return SigWrapper(sigRint(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigLog", [](SigWrapper & sig1)
        {
            return SigWrapper(sigLog(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigLog10", [](SigWrapper & sig1)
        {
            return SigWrapper(sigLog10(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigFloor", [](SigWrapper & sig1)
        {
            return SigWrapper(sigFloor(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigExp", [](SigWrapper & sig1)
        {
            return SigWrapper(sigExp(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigExp10", [](SigWrapper & sig1)
        {
            return SigWrapper(sigExp10(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigCos", [](SigWrapper & sig1)
        {
            return SigWrapper(sigCos(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigCeil", [](SigWrapper & sig1)
        {
            return SigWrapper(sigCeil(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigAtan", [](SigWrapper & sig1)
        {
            return SigWrapper(sigAtan(sig1));
        }, arg("sig1"), "Blah", returnPolicy)
        .def("sigAsin", [](SigWrapper & sig1)
        {
            return SigWrapper(sigAsin(sig1));
        }, arg("sig1"), "Blah", returnPolicy)

        .def("sigRemainder", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigRemainder(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigPow", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigPow(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigMin", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigMin(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigMax", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigMax(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigFmod", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigFmod(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigAtan2", [](SigWrapper &sig1, SigWrapper &sig2)
        {
            return SigWrapper(sigAtan2(sig1, sig2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigSelf", []()
        {
            return SigWrapper(sigSelf());
        }, "Blah", returnPolicy)
        .def("sigRecursion", [](SigWrapper & sig1)
        {
            return SigWrapper(sigRecursion(sig1));
        }, arg("sig"), "Blah", returnPolicy)

        .def("sigButton", [](std::string &label)
        {
            return SigWrapper(sigButton(label));
        }, arg("label"), "Blah", returnPolicy)
        .def("sigCheckbox", [](std::string &label)
        {
            return SigWrapper(sigCheckbox(label));
        }, arg("label"), "Blah", returnPolicy)

        .def("sigVSlider", [](std::string &label, SigWrapper &sigInit, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sigStep)
        {
            return SigWrapper(sigVSlider(label, sigInit, sigMin, sigMax, sigStep));
        }, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("sigHSlider", [](std::string &label, SigWrapper &sigInit, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sigStep)
        {
            return SigWrapper(sigHSlider(label, sigInit, sigMin, sigMax, sigStep));
        }, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("sigNumEntry", [](std::string &label, SigWrapper &sigInit, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sigStep)
        {
            return SigWrapper(sigNumEntry(label, sigInit, sigMin, sigMax, sigStep));
        }, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("sigVBargraph", [](std::string &label, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sig)
        {
            return SigWrapper(sigVBargraph(label, sigMin, sigMax, sig));
        }, arg("label"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("sigHBargraph", [](std::string &label, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sig)
        {
            return SigWrapper(sigHBargraph(label, sigMin, sigMax, sig));
        }, arg("label"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("sigAttach", [](SigWrapper &s1, SigWrapper &s2)
        {
            return SigWrapper(sigAttach(s1, s2));
        }, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigSampleRate", []()
        {
            return SigWrapper(sigMin(sigReal(192000.0),
                sigMax(sigReal(1.0),
                    sigFConst(SType::kSInt, "fSamplingFreq", "<math.h>"))));
        }, "Blah", returnPolicy)
        .def("sigBufferSize", []()
        {
            return SigWrapper(sigFVar(SType::kSInt, "count", "<math.h>"));
        }, "Blah", returnPolicy)

        .doc() = "Faust Box API";
}

# endif

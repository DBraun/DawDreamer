#pragma once
#include "ProcessorBase.h"

#ifdef BUILD_DAWDREAMER_FAUST
#include <faust/compiler/generator/libfaust.h>
#include <faust/compiler/utils/TMutex.h>
#include <faust/dsp/interpreter-dsp.h>
#include <faust/dsp/llvm-dsp.h>
#include <faust/dsp/poly-interpreter-dsp.h>
#include <faust/dsp/poly-llvm-dsp.h>
#include <faust/export.h>
#include <faust/gui/APIUI.h>
#include <faust/gui/MidiUI.h>
#include <faust/gui/SoundUI.h>
#include <faust/midi/rt-midi.h>

#include <map>

#include "FaustSignalAPI.h"

/*
A custom implementation of SoundUI. For a requested soundfile primitive in
Faust, we first try to find it in our Python dictionary of buffers. If it's not
found, we resort to using the parent's JuceReader implementation which is still
capable of loading wav files directly from the filesystem.
*/
class MySoundUI : public SoundUI {
 private:
  std::map<std::string, std::vector<juce::AudioSampleBuffer>> *m_SoundfileMap;
  int m_sampleRate = -1;

 public:
  MySoundUI(
      std::map<std::string, std::vector<juce::AudioSampleBuffer>> *soundfileMap,
      const std::string &sound_directory = "", int sample_rate = -1,
      SoundfileReader *reader = nullptr, bool is_double = false)
      : SoundUI(sound_directory, sample_rate, reader, is_double) {
    jassert(soundfileMap);
    m_SoundfileMap = soundfileMap;
    m_sampleRate = sample_rate;
  }

  void addSoundfile(const char *label, const char *url, Soundfile **sf_zone) {
    // Parse the possible list
    std::string saved_url_real = std::string(label);
    if (fSoundfileMap.find(saved_url_real) != fSoundfileMap.end()) {
      // Get the soundfile.
      *sf_zone = fSoundfileMap[saved_url_real].get();
      return;
    }

    if (m_SoundfileMap->find(saved_url_real) != m_SoundfileMap->end()) {
      int total_length = 0;
      int numChannels = 1;  // start with at least 1 channel. This may
                            // increase due to code below.

      auto buffers = m_SoundfileMap->at(saved_url_real);

      for (auto &buffer : buffers) {
        total_length += buffer.getNumSamples();
        numChannels = std::max(numChannels, buffer.getNumChannels());
      }

      total_length += (MAX_SOUNDFILE_PARTS - buffers.size()) * BUFFER_SIZE;

      Soundfile *soundfile = new Soundfile(numChannels, total_length, MAX_CHAN,
                                           (int)buffers.size(), false);

      // Manually fill in the soundfile:
      // The following code is a modification of
      // SoundfileReader::createSoundfile and SoundfileReader::readFile

      int offset = 0;

      int i = 0;
      for (auto &buffer : buffers) {
        int numSamples = buffer.getNumSamples();

        soundfile->fLength[i] = numSamples;
        soundfile->fSR[i] = m_sampleRate;
        soundfile->fOffset[i] = offset;

        void *tmpBuffers = alloca(soundfile->fChannels * sizeof(float *));
        soundfile->getBuffersOffsetReal<float>(tmpBuffers, offset);

        for (int chan = 0; chan < buffer.getNumChannels(); chan++) {
          for (int sample = 0; sample < numSamples; sample++) {
            // todo: don't assume float
            // todo: use memcpy or similar to be faster
            static_cast<float **>(soundfile->fBuffers)[chan][offset + sample] =
                buffer.getSample(chan, sample);
          }
        }

        offset += soundfile->fLength[i];
        i++;
      }

      // Complete with empty parts
      for (auto i = (int)buffers.size(); i < MAX_SOUNDFILE_PARTS; i++) {
        soundfile->emptyFile(i, offset);
      }

      // Share the same buffers for all other channels so that we have
      // max_chan channels available
      soundfile->shareBuffers(numChannels, MAX_CHAN);
      fSoundfileMap[saved_url_real] = std::shared_ptr<Soundfile>(soundfile);

      // Get the soundfile pointer
      *sf_zone = fSoundfileMap[saved_url_real].get();
      return;
    }

    // The requested sound url wasn't in our python dictionary, so use the
    // inherited method to load it from the filesystem.
    SoundUI::addSoundfile(label, url, sf_zone);
  }
};

template <typename Ch, typename Traits = std::char_traits<Ch>,
          typename Sequence = std::vector<Ch>>
struct basic_seqbuf : std::basic_streambuf<Ch, Traits> {
  typedef std::basic_streambuf<Ch, Traits> base_type;
  typedef typename base_type::int_type int_type;
  typedef typename base_type::traits_type traits_type;

  virtual int_type overflow(int_type ch) {
    if (traits_type::eq_int_type(ch, traits_type::eof()))
      return traits_type::eof();
    c.push_back(traits_type::to_char_type(ch));
    return ch;
  }

  Sequence const &get_sequence() const { return c; }

 protected:
  Sequence c;
};

// convenient typedefs
typedef basic_seqbuf<char> seqbuf;

class FaustProcessor : public ProcessorBase {
 public:
  FaustProcessor(std::string newUniqueName, double sampleRate,
                 int samplesPerBlock);
  ~FaustProcessor();

  bool canApplyBusesLayout(
      const juce::AudioProcessor::BusesLayout &layout) override;
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;

  int getTotalNumOutputChannels() override {
    if (!m_compileState) {
      this->compile();
    }
    return ProcessorBase::getTotalNumOutputChannels();
  }

  int getTotalNumInputChannels() override {
    if (!m_compileState) {
      this->compile();
    }
    return ProcessorBase::getTotalNumInputChannels();
  }

  void processBlock(juce::AudioSampleBuffer &buffer,
                    juce::MidiBuffer &midiBuffer) override;

  bool acceptsMidi() const override {
    return false;
  }  // todo: faust should be able to play MIDI.
  bool producesMidi() const override { return false; }

  void reset() override;

  void createParameterLayout();

  const juce::String getName() const override { return "FaustProcessor"; }

  void automateParameters(AudioPlayHead::PositionInfo &posInfo,
                          int numSamples) override;
  bool setAutomation(std::string &parameterName, py::array input,
                     std::uint32_t ppqn) override;

  // faust stuff
  void clear();
  bool compile();
  bool setDSPString(const std::string &code);
  bool setDSPFile(const std::string &path);
  bool setParamWithIndex(const int index, float p);
  float getParamWithIndex(const int index);
  float getParamWithPath(const std::string &n);
  std::string code();
  bool isCompiled() { return bool(m_compileState); };

  py::list getPluginParametersDescription();

  void setNumVoices(int numVoices);
  int getNumVoices();

  void setGroupVoices(bool groupVoices);
  int getGroupVoices();

  void setDynamicVoices(bool dynamicVoices);
  int getDynamicVoices();

  void setAutoImport(const std::string &s) { m_autoImport = s; }
  std::string getAutoImport() { return m_autoImport; }

  bool loadMidi(const std::string &path, bool clearPrevious, bool isBeats,
                bool allEvents);

  void clearMidi();

  int getNumMidiEvents();

  bool addMidiNote(const uint8 midiNote, const uint8 midiVelocity,
                   const double noteStart, const double noteLength,
                   bool isBeats);

  void setSoundfiles(py::dict);

  double getReleaseLength();

  void setReleaseLength(double sec);

  void setFaustLibrariesPath(std::string faustLibrariesPath) {
    m_faustLibrariesPath = faustLibrariesPath;
  }

  std::string getFaustLibrariesPath() { return m_faustLibrariesPath; }

  void setFaustAssetsPath(std::string faustAssetsPath) {
    m_faustAssetsPath = faustAssetsPath;
  }

  std::string getFaustAssetsPath() { return m_faustAssetsPath; }

  std::map<std::string, std::vector<juce::AudioSampleBuffer>> m_SoundfileMap;

  void saveMIDI(std::string &savePath);

 private:
  double mySampleRate;

  enum CompileState { kNotCompiled, kMono, kPoly, kSignalMono, kSignalPoly };

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
  std::string m_faustAssetsPath = "";

  int m_nvoices = 0;
  bool m_dynamicVoices = true;
  bool m_groupVoices = true;

  MidiBuffer myMidiBufferQN;
  MidiBuffer myMidiBufferSec;

  MidiMessageSequence myRecordedMidiSequence;  // for fetching by user later.

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
  std::map<int, std::string> m_map_juceIndex_to_parAddress;

  TMutex guiUpdateMutex;

  std::string getTarget();

  // public libfaust API
 public:
  bool compileSignals(std::vector<SigWrapper> &wrappers,
                      std::optional<std::vector<std::string>> in_argv);

  bool compileBox(BoxWrapper &box,
                  std::optional<std::vector<std::string>> in_argv);
};

inline void create_bindings_for_faust_processor(py::module &m) {
  using arg = py::arg;
  using kw_only = py::kw_only;

  auto returnPolicy = py::return_value_policy::take_ownership;

  // todo: for consistency, lookup these descriptions from source.cpp
  auto add_midi_description =
      "Add a single MIDI note whose note and velocity are integers between 0 "
      "and 127. By default, when `beats` is False, the start time and duration "
      "are measured in seconds, otherwise beats.";
  auto load_midi_description =
      "Load MIDI from a file. If `all_events` is True, then all events (not "
      "just Note On/Off) will be loaded. By default, when `beats` is False, "
      "notes will be converted to absolute times and will not be affected by "
      "the Render Engine's BPM. By default, `clear_previous` is True.";
  auto save_midi_description =
      "After rendering, you can save the MIDI to a file using absolute times "
      "(SMPTE format).";

  py::class_<FaustProcessor, ProcessorBase> faustProcessor(m, "FaustProcessor");

  faustProcessor
      .def("set_dsp", &FaustProcessor::setDSPFile, arg("filepath"),
           "Set the FAUST box process with a file.")
      .def("set_dsp_string", &FaustProcessor::setDSPString, arg("faust_code"),
           "Set the FAUST box process with a string containing FAUST code.")
      .def("compile", &FaustProcessor::compile,
           "Compile the FAUST object. You must have already set a dsp file "
           "path or dsp string.")
      .def_property(
          "auto_import", &FaustProcessor::getAutoImport,
          &FaustProcessor::setAutoImport,
          "The auto import string. Default is `import(\"stdfaust.lib\");`")
      .def("get_parameters_description",
           &FaustProcessor::getPluginParametersDescription,
           "Get a list of dictionaries describing the parameters of the most "
           "recently compiled FAUST code.")
      .def("get_parameter", &FaustProcessor::getParamWithIndex,
           arg("param_index"))
      .def("get_parameter", &FaustProcessor::getParamWithPath,
           arg("parameter_path"))
      .def("set_parameter", &FaustProcessor::setParamWithIndex,
           arg("parameter_index"), arg("value"))
      .def("set_parameter", &FaustProcessor::setAutomationVal,
           arg("parameter_path"), arg("value"))
      .def_property_readonly("compiled", &FaustProcessor::isCompiled,
                             "Did the most recent DSP code compile?")
      .def_property_readonly("code", &FaustProcessor::code,
                             "Get the most recently compiled Faust DSP code.")
      .def_property("num_voices", &FaustProcessor::getNumVoices,
                    &FaustProcessor::setNumVoices,
                    "The number of voices for polyphony. Set to zero to "
                    "disable polyphony. One or more enables polyphony.")
      .def_property(
          "group_voices", &FaustProcessor::getGroupVoices,
          &FaustProcessor::setGroupVoices,
          "If grouped, all polyphonic voices will share the same parameters. "
          "This parameter only matters if polyphony is enabled.")
      .def_property("dynamic_voices", &FaustProcessor::getDynamicVoices,
                    &FaustProcessor::setDynamicVoices,
                    "If enabled (default), voices are dynamically enabled and "
                    "disabled to save computation. This parameter only matters "
                    "if polyphony is enabled.")
      .def_property(
          "release_length", &FaustProcessor::getReleaseLength,
          &FaustProcessor::setReleaseLength,
          "If using polyphony, specifying the release length accurately can "
          "help avoid warnings about voices being stolen.")
      .def_property("faust_libraries_path",
                    &FaustProcessor::getFaustLibrariesPath,
                    &FaustProcessor::setFaustLibrariesPath,
                    "Absolute path to directory containing your custom "
                    "\".lib\" files containing Faust code.")
      .def_property("faust_assets_path", &FaustProcessor::getFaustAssetsPath,
                    &FaustProcessor::setFaustAssetsPath,
                    "Absolute path to directory containing audio files to be "
                    "used by Faust.")
      .def_property_readonly("n_midi_events", &FaustProcessor::getNumMidiEvents,
                             "The number of MIDI events stored in the buffer. \
		Note that note-ons and note-offs are counted separately.")
      .def("load_midi", &FaustProcessor::loadMidi, arg("filepath"), kw_only(),
           arg("clear_previous") = true, arg("beats") = false,
           arg("all_events") = true, load_midi_description)
      .def("clear_midi", &FaustProcessor::clearMidi, "Remove all MIDI notes.")
      .def("add_midi_note", &FaustProcessor::addMidiNote, arg("note"),
           arg("velocity"), arg("start_time"), arg("duration"), kw_only(),
           arg("beats") = false, add_midi_description)
      .def("save_midi", &FaustProcessor::saveMIDI, arg("filepath"),
           save_midi_description)
      .def("set_soundfiles", &FaustProcessor::setSoundfiles,
           arg("soundfile_dict"),
           "Set the audio data that the FaustProcessor can use with the "
           "`soundfile` primitive.")

      .def("compile_signals", &FaustProcessor::compileSignals, arg("signals"),
           arg("argv") = py::none(), returnPolicy)
      .def("compile_box", &FaustProcessor::compileBox, arg("box"),
           arg("argv") = py::none(), returnPolicy)

      .doc() =
      "A Faust Processor can compile and execute FAUST code. See "
      "https://faust.grame.fr for more information.";
}

#endif

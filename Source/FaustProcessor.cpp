#include "FaustProcessor.h"

#ifdef BUILD_DAWDREAMER_FAUST

#include <filesystem>

#include "faust/midi/RtMidi.cpp"

#ifdef WIN32
__declspec(selectany) std::list<GUI*> GUI::fGuiList;
__declspec(selectany) ztimedmap GUI::gTimedZoneMap;
#else
std::list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) \
  do {                 \
    if (x) {           \
      delete x;        \
      x = NULL;        \
    }                  \
  } while (0)
#define SAFE_DELETE_ARRAY(x) \
  do {                       \
    if (x) {                 \
      delete[] x;            \
      x = NULL;              \
    }                        \
  } while (0)
#define SAFE_RELEASE(x) \
  do {                  \
    if (x) {            \
      x->release();     \
      x = NULL;         \
    }                   \
  } while (0)
#define SAFE_ADD_REF(x) \
  do {                  \
    if (x) {            \
      x->add_ref();     \
    }                   \
  } while (0)
#define SAFE_REF_ASSIGN(lhs, rhs) \
  do {                            \
    SAFE_RELEASE(lhs);            \
    (lhs) = (rhs);                \
    SAFE_ADD_REF(lhs);            \
  } while (0)
#endif

#define COMPILE_FAUST    \
  if (!m_compileState) { \
    this->compile();     \
  }

FaustProcessor::FaustProcessor(std::string newUniqueName, double sampleRate,
                               int samplesPerBlock)
    : ProcessorBase{newUniqueName} {
  mySampleRate = sampleRate;

  m_factory = NULL;
  m_poly_factory = NULL;
  m_dsp = NULL;
  m_dsp_poly = NULL;
  m_ui = NULL;
  m_soundUI = NULL;
  m_numInputChannels = 0;
  m_numOutputChannels = 0;
  // auto import
  m_autoImport =
      "// FaustProcessor (DawDreamer) auto "
      "import:\nimport(\"stdfaust.lib\");\n";
}

FaustProcessor::~FaustProcessor() {
  clear();

  myMidiBufferQN.clear();
  myMidiBufferSec.clear();
  myRecordedMidiSequence.clear();

  delete myMidiIteratorQN;
  delete myMidiIteratorSec;
}

bool FaustProcessor::setAutomation(std::string& parameterName, py::array input,
                                   std::uint32_t ppqn) {
  COMPILE_FAUST
  return ProcessorBase::setAutomation(parameterName, input, ppqn);
}

bool FaustProcessor::canApplyBusesLayout(
    const juce::AudioProcessor::BusesLayout& layout) {
  return (layout.getMainInputChannels() == m_numInputChannels) &&
         (layout.getMainOutputChannels() == m_numOutputChannels);
}

void FaustProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  COMPILE_FAUST
}

void FaustProcessor::processBlock(juce::AudioSampleBuffer& buffer,
                                  juce::MidiBuffer& midiBuffer) {
  // todo: Faust should be able to use the incoming midiBuffer too.
  auto posInfo = getPlayHead()->getPosition();

  if (!m_compileState) {
    throw std::runtime_error(
        "Faust Processor called processBlock but it wasn't compiled.");
  }

  if (m_compileState == kMono || m_compileState == kSignalMono) {
    if (m_dsp != NULL) {
      m_dsp->compute(buffer.getNumSamples(),
                     (float**)buffer.getArrayOfReadPointers(),
                     buffer.getArrayOfWritePointers());
    } else {
      throw std::runtime_error("Faust Processor: m_dsp is null");
    }
  } else if (m_dsp_poly) {
    auto start = *posInfo->getTimeInSamples();

    auto pulseStart = std::floor(*posInfo->getPpqPosition() * PPQN);
    auto pulseStep = (*posInfo->getBpm() * PPQN) / (mySampleRate * 60.);

    // render one sample at a time because we want accurate timing of
    // keyOn/keyOff.

    auto oneSampReadPtrs = (float**)oneSampleInBuffer.getArrayOfReadPointers();
    auto oneSampWritePtrs =
        (float**)oneSampleOutBuffer.getArrayOfWritePointers();
    const int midiChannel = 0;
    int chan = 0;

    for (int i = 0; i < buffer.getNumSamples(); i++) {
      {
        myIsMessageBetweenSec = myMidiMessagePositionSec >= start &&
                                myMidiMessagePositionSec < start + 1;
        while (myIsMessageBetweenSec && myMidiEventsDoRemainSec) {
          // steps for saving midi to file output
          auto messageCopy = MidiMessage(myMidiMessageSec);
          messageCopy.setTimeStamp((*posInfo->getTimeInSamples() + i) *
                                   (2400. / mySampleRate));
          if (!(messageCopy.isEndOfTrackMetaEvent() ||
                messageCopy.isTempoMetaEvent())) {
            myRecordedMidiSequence.addEvent(messageCopy);
          }

          // steps for playing MIDI
          if (myMidiMessageSec.isNoteOn()) {
            m_dsp_poly->keyOn(midiChannel, myMidiMessageSec.getNoteNumber(),
                              myMidiMessageSec.getVelocity());
          } else if (myMidiMessageSec.isNoteOff()) {
            m_dsp_poly->keyOff(midiChannel, myMidiMessageSec.getNoteNumber(),
                               myMidiMessageSec.getVelocity());
          }

          myMidiEventsDoRemainSec = myMidiIteratorSec->getNextEvent(
              myMidiMessageSec, myMidiMessagePositionSec);
          myIsMessageBetweenSec = myMidiMessagePositionSec >= start &&
                                  myMidiMessagePositionSec < start + 1;
        }
      }

      {
        myIsMessageBetweenQN = myMidiMessagePositionQN >= pulseStart &&
                               myMidiMessagePositionQN < pulseStart + 1;
        while (myIsMessageBetweenQN && myMidiEventsDoRemainQN) {
          // steps for saving midi to file output
          auto messageCopy = MidiMessage(myMidiMessageQN);
          messageCopy.setTimeStamp((*posInfo->getTimeInSamples() + i) *
                                   (2400. / mySampleRate));
          if (!(messageCopy.isEndOfTrackMetaEvent() ||
                messageCopy.isTempoMetaEvent())) {
            myRecordedMidiSequence.addEvent(messageCopy);
          }

          // steps for playing MIDI
          if (myMidiMessageQN.isNoteOn()) {
            m_dsp_poly->keyOn(midiChannel, myMidiMessageQN.getNoteNumber(),
                              myMidiMessageQN.getVelocity());
          } else if (myMidiMessageQN.isNoteOff()) {
            m_dsp_poly->keyOff(midiChannel, myMidiMessageQN.getNoteNumber(),
                               myMidiMessageQN.getVelocity());
          }

          myMidiEventsDoRemainQN = myMidiIteratorQN->getNextEvent(
              myMidiMessageQN, myMidiMessagePositionQN);

          myIsMessageBetweenQN = myMidiMessagePositionQN >= pulseStart &&
                                 myMidiMessagePositionQN < pulseStart + 1;
        }
      }

      for (chan = 0; chan < m_numInputChannels; chan++) {
        oneSampleInBuffer.setSample(chan, 0, buffer.getSample(chan, i));
      }

      m_dsp_poly->compute(1, oneSampReadPtrs, oneSampWritePtrs);

      for (chan = 0; chan < m_numOutputChannels; chan++) {
        buffer.setSample(chan, i, oneSampleOutBuffer.getSample(chan, 0));
      }

      start += 1;
      pulseStart += pulseStep;
    }
  } else {
    throw std::runtime_error("Faust Processor: dsp is null.");
  }

  ProcessorBase::processBlock(buffer, midiBuffer);
}

#include <iostream>

bool hasEnding(std::string const& fullString, std::string const& ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare(fullString.length() - ending.length(),
                                    ending.length(), ending));
  } else {
    return false;
  }
}

bool hasStart(std::string const& fullString, std::string const& start) {
  return fullString.rfind(start, 0) == 0;
}

void FaustProcessor::automateParameters(AudioPlayHead::PositionInfo& posInfo,
                                        int numSamples) {
  if (!m_ui) {
    return;
  }

  const Array<AudioProcessorParameter*>& processorParams =
      this->getParameters();

  bool anyAutomation = false;
  for (int i = 0; i < this->getNumParameters(); i++) {
    int faustIndex = m_map_juceIndex_to_faustIndex[i];
    auto theParameter = (AutomateParameterFloat*)processorParams[i];

    bool hasAutomation = theParameter->isAutomated();
    anyAutomation |= hasAutomation;
    if (hasAutomation) {
      m_ui->setParamValue(faustIndex, theParameter->sample(posInfo));
    }
  }

  // If polyphony is enabled and we're grouping voices,
  // several voices might share the same parameters in a group.
  // Therefore we have to call updateAllGuis to update all dependent parameters.
  if (anyAutomation && m_nvoices > 0 && m_groupVoices) {
    // When you want to access shared memory:
    if (guiUpdateMutex.Lock()) {
      // Have Faust update all GUIs.
      GUI::updateAllGuis();

      guiUpdateMutex.Unlock();
    }
  }
}

void FaustProcessor::reset() {
  if (m_dsp) {
    m_dsp->instanceClear();
  }

  if (m_dsp_poly) {
    m_dsp_poly->instanceClear();
  }

  COMPILE_FAUST

  {
    // update all parameters once.

    juce::AudioPlayHead::PositionInfo posInfo;
    // It's important to initialize these.
    posInfo.setTimeInSamples(0);
    posInfo.setTimeInSeconds(0);
    posInfo.setPpqPosition(0);

    const Array<AudioProcessorParameter*>& processorParams =
        this->getParameters();

    for (int i = 0; i < this->getNumParameters(); i++) {
      int faustIndex = m_map_juceIndex_to_faustIndex[i];

      m_ui->setParamValue(
          faustIndex,
          ((AutomateParameterFloat*)processorParams[i])->sample(posInfo));
    }
    if (m_nvoices > 0 && m_groupVoices) {
      // When you want to access shared memory:
      if (guiUpdateMutex.Lock()) {
        // Have Faust update all GUIs.
        GUI::updateAllGuis();

        guiUpdateMutex.Unlock();
      }
    }
  }

  delete myMidiIteratorQN;
  myMidiIteratorQN =
      new MidiBuffer::Iterator(myMidiBufferQN);  // todo: deprecated.
  myMidiEventsDoRemainQN =
      myMidiIteratorQN->getNextEvent(myMidiMessageQN, myMidiMessagePositionQN);

  delete myMidiIteratorSec;
  myMidiIteratorSec =
      new MidiBuffer::Iterator(myMidiBufferSec);  // todo: deprecated.
  myMidiEventsDoRemainSec = myMidiIteratorSec->getNextEvent(
      myMidiMessageSec, myMidiMessagePositionSec);

  myRecordedMidiSequence.clear();
  myRecordedMidiSequence.addEvent(juce::MidiMessage::midiStart());
  myRecordedMidiSequence.addEvent(
      juce::MidiMessage::timeSignatureMetaEvent(4, 4));
  myRecordedMidiSequence.addEvent(
      juce::MidiMessage::tempoMetaEvent(500 * 1000));
  myRecordedMidiSequence.addEvent(juce::MidiMessage::midiChannelMetaEvent(1));

  ProcessorBase::reset();
}

void FaustProcessor::clear() {
  m_numInputChannels = 0;
  m_numOutputChannels = 0;

  // todo: do something with m_midi_handler
  if (m_dsp_poly) {
    m_midi_handler.removeMidiIn(m_dsp_poly);
    m_midi_handler.stopMidi();
  }

  SAFE_DELETE(m_soundUI);
  SAFE_DELETE(m_ui);

  if (m_dsp_poly) {
      delete (mydsp_poly*) m_dsp_poly;
    m_dsp_poly = nullptr;
	  // we don't need to delete m_dsp because m_dsp_poly would have done it
      m_dsp = nullptr;
  } else {
    SAFE_DELETE(m_dsp);
    delete m_dsp_poly;
    m_dsp_poly = nullptr;
    m_dsp = nullptr;
  }

  SAFE_DELETE(m_poly_factory);
  deleteDSPFactory(m_factory);
  m_factory = nullptr;

  m_compileState = kNotCompiled;
}

void FaustProcessor::setNumVoices(int numVoices) {
  m_compileState = kNotCompiled;
  m_nvoices = std::max(0, numVoices);
}

int FaustProcessor::getNumVoices() { return m_nvoices; }

void FaustProcessor::setGroupVoices(bool groupVoices) {
  m_compileState = kNotCompiled;
  m_groupVoices = groupVoices;
};

int FaustProcessor::getGroupVoices() { return m_groupVoices; };

void FaustProcessor::setDynamicVoices(bool dynamicVoices) {
  m_compileState = kNotCompiled;
  m_dynamicVoices = dynamicVoices;
};

int FaustProcessor::getDynamicVoices() { return m_dynamicVoices; };

bool FaustProcessor::setDSPString(const std::string& code) {
  m_compileState = kNotCompiled;

  if (code.empty()) {
    throw std::runtime_error("DSP string is empty.");
  }

  // save
  m_code = code;

  return true;
}

std::string FaustProcessor::getTarget() {
#if __APPLE__
  // on macOS, specifying the target like this helps us handle LLVM on both
  // x86_64 and arm64. Crucially, LLVM must have been compiled separately for
  // each architecture. A different libfaust must have also been compiled for
  // the corresponding LLVM architecture and the `LLVM_BUILD_UNIVERSAL`
  // preprocessor must have been set while building libfaust. Then the two
  // libfaust.2.dylib can get combined with `lipo x86_64.libfaust.2.dylib
  // arm64.libfaust.2.dylib -create -output libfaust.2.dylib`
  auto target = getDSPMachineTarget();
#else
  auto target = std::string("");
#endif
  return target;
}

bool FaustProcessor::compile() {
  m_compileState = kNotCompiled;

  // clean up
  clear();

  // optimization level
  const int optimize = -1;
  // arguments

  auto pathToFaustLibraries = getPathToFaustLibraries();

  if (pathToFaustLibraries.empty()) {
    throw std::runtime_error(
        "FaustProcessor::compile(): Error for path for faust libraries: " +
        pathToFaustLibraries);
  }

  int argc = 0;
  const char** argv = new const char*[64];

  argv[argc++] = "-I";
  argv[argc++] = pathToFaustLibraries.c_str();

  if (!m_faustLibrariesPath.empty()) {
    argv[argc++] = "-I";
    argv[argc++] = m_faustLibrariesPath.c_str();
  }

  auto theCode = m_autoImport + "\n" + m_code;

  std::string m_errorString;

  auto target = getTarget();

  // create new factory
  bool is_polyphonic = m_nvoices > 0;
  if (is_polyphonic) {
    m_poly_factory = createPolyDSPFactoryFromString(
        "DawDreamer", theCode, argc, argv, target, m_errorString, optimize);
  } else {
    m_factory = createDSPFactoryFromString("DawDreamer", theCode, argc, argv,
                                           target, m_errorString, optimize);
  }

  for (int i = 0; i < argc; i++) {
    argv[i] = NULL;
  }
  delete[] argv;
  argv = nullptr;

  // check for error
  if (!m_errorString.empty()) {
    // output error
    clear();
    throw std::runtime_error(
        "FaustProcessor::compile(): " + m_errorString +
        ". Check the faustlibraries path: " + pathToFaustLibraries);
  }

  //// print where faustlib is looking for stdfaust.lib and the other lib files.
  // auto pathnames = m_factory->getIncludePathnames();
  // std::cout << "pathnames:\n" << std::endl;
  // for (auto name : pathnames) {
  //    std::cout << name << "\n" << std::endl;
  //}
  // std::cout << "library list:\n" << std::endl;
  // auto librarylist = m_factory->getLibraryList();
  // for (auto name : librarylist) {
  //    std::cout << name << "\n" << std::endl;
  //}

  if (is_polyphonic) {
    // (false, true) works
    m_dsp_poly = m_poly_factory->createPolyDSPInstance(
        m_nvoices, m_dynamicVoices, m_groupVoices);
    if (!m_dsp_poly) {
      clear();
      throw std::runtime_error(
          "FaustProcessor::compile(): Cannot create Poly DSP instance.");
    }
    m_dsp_poly->setReleaseLength(m_releaseLengthSec);
  } else {
    // create DSP instance
    m_dsp = m_factory->createDSPInstance();
    if (!m_dsp) {
      clear();
      throw std::runtime_error(
          "FaustProcessor::compile(): Cannot create DSP instance.");
    }
  }

  dsp* theDsp = is_polyphonic ? m_dsp_poly : m_dsp;

  // get channels
  int inputs = theDsp->getNumInputs();
  int outputs = theDsp->getNumOutputs();

  m_numInputChannels = inputs;
  m_numOutputChannels = outputs;

  setMainBusInputsAndOutputs(inputs, outputs);

  // make new UI
  if (is_polyphonic) {
    m_midi_handler = rt_midi("my_midi");
    m_midi_handler.addMidiIn(m_dsp_poly);

    oneSampleInBuffer.setSize(m_numInputChannels, 1);
    oneSampleOutBuffer.setSize(m_numOutputChannels, 1);
  }

  m_ui = new APIUI();
  theDsp->buildUserInterface(m_ui);

  // soundfile UI.
  m_soundUI = new MySoundUI();
  for (const auto& [label, buffers] : m_SoundfileMap) {
    m_soundUI->addSoundfileFromBuffers(label.c_str(), buffers,
                                       (int)(mySampleRate + .5));
  }
  theDsp->buildUserInterface(m_soundUI);

  // init
  theDsp->init((int)(mySampleRate + .5));

  createParameterLayout();

  m_compileState = is_polyphonic ? kPoly : kMono;
  return true;
}

void FaustProcessor::compileSignals(
    std::vector<SigWrapper>& wrappers,
    std::optional<std::vector<std::string>> in_argv) {
  clear();

  int argc = 0;
  const char* argv[64];

  if (in_argv.has_value()) {
    for (auto& s : *in_argv) {
      argv[argc++] = s.c_str();
    }
  }

  tvec signals;
  for (auto wrapper : wrappers) {
    signals.push_back(wrapper);
  }

  auto target = getTarget();
  std::string error_msg;

  m_factory = createDSPFactoryFromSignals("dawdreamer", signals, argc, argv,
                                          target, error_msg);

  if (!m_factory) {
    clear();
    throw std::runtime_error("FaustProcessor: " + error_msg);
  }

  m_dsp = m_factory->createDSPInstance();
  if (!m_dsp) {
    throw std::runtime_error("FaustProcessor: m_dsp not created.");
  }

  // create new factory
  bool is_polyphonic = m_nvoices > 0;
  auto theDSP = m_dsp;
  if (is_polyphonic) {
    // Allocate polyphonic DSP
    m_dsp_poly =
        new mydsp_poly(m_dsp, m_nvoices, m_dynamicVoices, m_groupVoices);
    m_dsp_poly->setReleaseLength(m_releaseLengthSec);
    theDSP = m_dsp_poly;
  }

  // get channels
  int inputs = theDSP->getNumInputs();
  int outputs = theDSP->getNumOutputs();

  m_numInputChannels = inputs;
  m_numOutputChannels = outputs;

  setMainBusInputsAndOutputs(inputs, outputs);

  // make new UI
  if (is_polyphonic) {
    m_midi_handler = rt_midi("my_midi");
    m_midi_handler.addMidiIn(m_dsp_poly);

    oneSampleInBuffer.setSize(m_numInputChannels, 1);
    oneSampleOutBuffer.setSize(m_numOutputChannels, 1);
  }

  m_ui = new APIUI();
  theDSP->buildUserInterface(m_ui);

  // soundfile UI.
  m_soundUI = new MySoundUI();
  for (const auto& [label, buffers] : m_SoundfileMap) {
    m_soundUI->addSoundfileFromBuffers(label.c_str(), buffers,
                                       (int)(mySampleRate + .5));
  }
  theDSP->buildUserInterface(m_soundUI);

  // init
  theDSP->init((int)(mySampleRate + .5));

  createParameterLayout();

  m_compileState = is_polyphonic ? kSignalPoly : kSignalMono;
}

void FaustProcessor::compileBox(
    BoxWrapper& box, std::optional<std::vector<std::string>> in_argv) {
  clear();

  int argc = 0;
  const char* argv[64];

  if (in_argv.has_value()) {
    for (auto& s : *in_argv) {
      argv[argc++] = s.c_str();
    }
  }

  auto target = this->getTarget();
  std::string error_msg;

  m_factory = createDSPFactoryFromBoxes("dawdreamer", box, argc, argv, target,
                                        error_msg);

  if (!m_factory) {
    clear();
    throw std::runtime_error("FaustProcessor: " + error_msg);
  }

  m_dsp = m_factory->createDSPInstance();
  assert(m_dsp);

  // create new factory
  bool is_polyphonic = m_nvoices > 0;
  auto theDSP = m_dsp;
  if (is_polyphonic) {
    // Allocate polyphonic DSP
    m_dsp_poly =
        new mydsp_poly(m_dsp, m_nvoices, m_dynamicVoices, m_groupVoices);
    m_dsp_poly->setReleaseLength(m_releaseLengthSec);
    theDSP = m_dsp_poly;
  }

  // get channels
  int inputs = theDSP->getNumInputs();
  int outputs = theDSP->getNumOutputs();

  m_numInputChannels = inputs;
  m_numOutputChannels = outputs;

  // make new UI
  if (is_polyphonic) {
    m_midi_handler = rt_midi("my_midi");
    m_midi_handler.addMidiIn(m_dsp_poly);

    oneSampleInBuffer.setSize(m_numInputChannels, 1);
    oneSampleOutBuffer.setSize(m_numOutputChannels, 1);
  }

  m_ui = new APIUI();
  theDSP->buildUserInterface(m_ui);

  // soundfile UI.
  m_soundUI = new MySoundUI();
  for (const auto& [label, buffers] : m_SoundfileMap) {
    m_soundUI->addSoundfileFromBuffers(label.c_str(), buffers,
                                       (int)(mySampleRate + .5));
  }
  theDSP->buildUserInterface(m_soundUI);

  // init
  theDSP->init((int)(mySampleRate + .5));

  createParameterLayout();

  m_compileState = is_polyphonic ? kSignalPoly : kSignalMono;

  setMainBusInputsAndOutputs(inputs, outputs);
}

bool FaustProcessor::setDSPFile(const std::string& path) {
  m_compileState = kNotCompiled;

  if (!std::filesystem::exists(path.c_str())) {
    throw std::runtime_error("File not found: " + path);
  }

  if (path.empty()) {
    throw std::runtime_error("Path to DSP file is empty.");
  }

  // open file
  std::ifstream fin(path.c_str());
  // check
  if (!fin.good()) {
    // error
    throw std::runtime_error(
        "FaustProcessor::setDSPFile(): ERROR opening file: '" + path + "'");
  }

  // clear code string
  m_code = "";
  // get it
  for (std::string line; std::getline(fin, line);) {
    m_code += line + '\n';
  }

  return true;
}

bool FaustProcessor::setParamWithIndex(const int index, float p) {
  COMPILE_FAUST
  if (!m_ui) {
    throw std::runtime_error("No UI for FaustProcessor.");
  }

  auto it = m_map_juceIndex_to_parAddress.find(index);
  if (it == m_map_juceIndex_to_parAddress.end()) {
    throw std::runtime_error("A parameter with index " + std::to_string(index) +
                             " is not valid for this FaustProcessor.");
  }

  auto& parAddress = it->second;

  return this->setAutomationValByStr(parAddress, p);
}

float FaustProcessor::getParamWithIndex(const int index) {
  COMPILE_FAUST
  if (!m_ui) return 0;  // todo: better handling

  auto it = m_map_juceIndex_to_parAddress.find(index);
  if (it == m_map_juceIndex_to_parAddress.end()) {
    return 0.;  // todo: better handling
  }

  auto& parAddress = it->second;

  return this->getAutomationAtZero(parAddress);
}

float FaustProcessor::getParamWithPath(const std::string& n) {
  COMPILE_FAUST
  if (!m_ui) return 0;  // todo: better handling

  return this->getAutomationAtZero(n);
}

std::string FaustProcessor::code() { return m_code; }

void FaustProcessor::createParameterLayout() {

  juce::AudioProcessorParameterGroup group;

  m_map_juceIndex_to_faustIndex.clear();
  m_map_juceIndex_to_parAddress.clear();

  int numParamsAdded = 0;

  for (int i = 0; i < m_ui->getParamsCount(); ++i) {
    auto parameterName = m_ui->getParamAddress(i);

    auto parnameString = std::string(parameterName);

    // Ignore the Panic button.
    if (hasEnding(parnameString, std::string("/Panic"))) {
      continue;
    }

    // ignore the names of parameters which are reserved for controlling
    // polyphony:
    // https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters
    // Note that an advanced user might want to not do this in order to
    // have much more control over the frequencies of individual voices,
    // like how MPE works.
    if (m_nvoices > 0) {
      if (hasEnding(parnameString, std::string("/freq")) ||
          hasEnding(parnameString, std::string("/note")) ||
          hasEnding(parnameString, std::string("/gain")) ||
          hasEnding(parnameString, std::string("/gate")) ||
          hasEnding(parnameString, std::string("/vel")) ||
          hasEnding(parnameString, std::string("/velocity"))) {
        continue;
      }

      if (!m_groupVoices &&
          hasStart(parnameString,
                   std::string("/Sequencer/DSP1/Polyphonic/Voices/"))) {
        // If we aren't grouping voices, FAUST for some reason is still adding
        // the "grouped" parameters to the UI, so we have to ignore them. The
        // per-voice "ungrouped" parameters won't be skipped.
        continue;
      }
    }

    m_map_juceIndex_to_faustIndex[numParamsAdded] = i;
    m_map_juceIndex_to_parAddress[numParamsAdded] = parnameString;

    auto parameterLabel = m_ui->getParamLabel(i);
    group.addChild(std::make_unique<AutomateParameterFloat>(
        parameterName, parameterName,
        NormalisableRange<float>(m_ui->getParamMin(i), m_ui->getParamMax(i)),
        m_ui->getParamInit(i), parameterLabel));

    numParamsAdded += 1;
  }

  this->setParameterTree(std::move(group));

  int i = 0;
  for (auto* parameter : this->getParameters()) {
    int j = m_map_juceIndex_to_faustIndex[i];
    // give it a valid single sample of automation.
    ProcessorBase::setAutomationValByIndex(i, m_ui->getParamInit(j));
    i++;
  }

}

py::list FaustProcessor::getPluginParametersDescription() {

  COMPILE_FAUST

  py::list myList;
  
  if (m_compileState) {
    int i = 0;
    for (auto* parameter : this->getParameters()) {
      std::string theName =
          parameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString();
      std::string label = parameter->getLabel().toStdString();

      auto it = m_map_juceIndex_to_faustIndex.find(i);
      if (it == m_map_juceIndex_to_faustIndex.end()) {
        i++;
        continue;
      }

      int faustIndex = it->second;

      auto paramItemType = m_ui->getParamItemType(faustIndex);

      bool isDiscrete = (paramItemType == APIUI::kButton) ||
                        (paramItemType == APIUI::kCheckButton) ||
                        (paramItemType == APIUI::kNumEntry);
      int numSteps =
          (m_ui->getParamMax(faustIndex) - m_ui->getParamMin(faustIndex)) /
              m_ui->getParamStep(faustIndex) +
          1;

      // todo: It would be better for DawDreamer to store the discrete
      // parameters correctly, but we're still saving them all as
      // AutomateParameterFloat.
      // bool isDiscrete = parameter->isDiscrete();
      // int numSteps = processorParams->getNumSteps();

      py::dict myDictionary;
      myDictionary["index"] = i;
      myDictionary["name"] = theName;
      myDictionary["numSteps"] = numSteps;
      myDictionary["isDiscrete"] = isDiscrete;
      myDictionary["label"] = label;

      myDictionary["min"] = m_ui->getParamMin(faustIndex);
      myDictionary["max"] = m_ui->getParamMax(faustIndex);
      myDictionary["step"] = m_ui->getParamStep(faustIndex);
      myDictionary["value"] = this->getAutomationAtZero(theName);

      myList.append(myDictionary);
      i++;
    }
  } else {
    throw std::runtime_error("ERROR: The Faust process isn't compiled.");
  }

  return myList;
}

int FaustProcessor::getNumMidiEvents() {
  return myMidiBufferSec.getNumEvents() + myMidiBufferQN.getNumEvents();
};

bool FaustProcessor::loadMidi(const std::string& path, bool clearPrevious,
                              bool isBeats, bool allEvents) {
  if (!std::filesystem::exists(path.c_str())) {
    throw std::runtime_error("File not found: " + path);
  }

  File file = File(path);
  FileInputStream fileStream(file);
  MidiFile midiFile;
  midiFile.readFrom(fileStream);

  if (clearPrevious) {
    myMidiBufferSec.clear();
    myMidiBufferQN.clear();
  }

  if (!isBeats) {
    midiFile.convertTimestampTicksToSeconds();

    for (int t = 0; t < midiFile.getNumTracks(); t++) {
      const MidiMessageSequence* track = midiFile.getTrack(t);
      for (int i = 0; i < track->getNumEvents(); i++) {
        MidiMessage& m = track->getEventPointer(i)->message;
        int sampleOffset = (int)(mySampleRate * m.getTimeStamp());
        if (allEvents || m.isNoteOff() || m.isNoteOn()) {
          myMidiBufferSec.addEvent(m, sampleOffset);
        }
      }
    }
  } else {
    auto timeFormat = midiFile.getTimeFormat();  // the ppqn (Ableton makes midi
                                                 // files with 96 ppqn)

    for (int t = 0; t < midiFile.getNumTracks(); t++) {
      const MidiMessageSequence* track = midiFile.getTrack(t);
      for (int i = 0; i < track->getNumEvents(); i++) {
        MidiMessage& m = track->getEventPointer(i)->message;

        if (allEvents || m.isNoteOff() || m.isNoteOn()) {
          // convert timestamp from its original time format to our high
          // resolution PPQN
          auto timeStamp = m.getTimeStamp() * PPQN / timeFormat;
          myMidiBufferQN.addEvent(m, timeStamp);
        }
      }
    }
  }

  return true;
}

void FaustProcessor::clearMidi() {
  myMidiBufferSec.clear();
  myMidiBufferQN.clear();
}

bool FaustProcessor::addMidiNote(uint8 midiNote, uint8 midiVelocity,
                                 const double noteStart,
                                 const double noteLength, bool isBeats) {
  if (midiNote > 255) midiNote = 255;
  if (midiNote < 0) midiNote = 0;
  if (midiVelocity > 255) midiVelocity = 255;
  if (midiVelocity < 0) midiVelocity = 0;
  if (noteLength <= 0) {
    throw std::runtime_error("The note length must be greater than zero.");
  }

  // Get the note on midiBuffer.
  MidiMessage onMessage = MidiMessage::noteOn(1, midiNote, midiVelocity);

  MidiMessage offMessage = MidiMessage::noteOff(1, midiNote, midiVelocity);

  if (!isBeats) {
    auto startTime = noteStart * mySampleRate;
    onMessage.setTimeStamp(startTime);
    offMessage.setTimeStamp(startTime + noteLength * mySampleRate);
    myMidiBufferSec.addEvent(onMessage, (int)onMessage.getTimeStamp());
    myMidiBufferSec.addEvent(offMessage, (int)offMessage.getTimeStamp());
  } else {
    auto startTime = noteStart * PPQN;
    onMessage.setTimeStamp(startTime);
    offMessage.setTimeStamp(startTime + noteLength * PPQN);
    myMidiBufferQN.addEvent(onMessage, (int)onMessage.getTimeStamp());
    myMidiBufferQN.addEvent(offMessage, (int)offMessage.getTimeStamp());
  }

  return true;
}

void FaustProcessor::saveMIDI(std::string& savePath) {
  MidiFile file;

  // 30*80 = 2400, so that's why the MIDI messages had their
  // timestamp set to seconds*2400
  file.setSmpteTimeFormat(30, 80);

  File myFile(savePath);

  file.addTrack(myRecordedMidiSequence);

  juce::FileOutputStream stream(myFile);
  if (stream.openedOk()) {
    // overwrite existing file.
    stream.setPosition(0);
    stream.truncate();
  }
  file.writeTo(stream);
}

using myaudiotype =
    py::array_t<float, py::array::c_style | py::array::forcecast>;

void FaustProcessor::setSoundfiles(py::dict d) {
  m_compileState = kNotCompiled;

  m_SoundfileMap.clear();

  for (auto&& [potentialString, potentialListOfAudio] : d) {
    if (!py::isinstance<py::str>(potentialString)) {
      throw std::runtime_error(
          "Error with FaustProcessor::setSoundfiles. Something was wrong with "
          "the keys of the dictionary.");
      return;
    }

    auto soundfileName = potentialString.cast<std::string>();

    if (!py::isinstance<py::list>(potentialListOfAudio)) {
      // todo: if it's audio, it's ok. Just use it.
      throw std::runtime_error(
          "Error with FaustProcessor::setSoundfiles. The values of the "
          "dictionary must be lists of audio data.");
      return;
    }

    py::list listOfAudio = potentialListOfAudio.cast<py::list>();

    for (py::handle potentialAudio : listOfAudio) {
      // if (py::isinstance<myaudiotype>(potentialAudio)) {

      // todo: safer casting?
      auto audioData = potentialAudio.cast<myaudiotype>();

      float* input_ptr = (float*)audioData.data();

      AudioSampleBuffer buffer;

      buffer.setSize((int)audioData.shape(0), (int)audioData.shape(1));

      for (int y = 0; y < audioData.shape(1); y++) {
        for (int x = 0; x < audioData.shape(0); x++) {
          buffer.setSample(x, y, input_ptr[x * audioData.shape(1) + y]);
        }
      }

      m_SoundfileMap[soundfileName].push_back(buffer);

      //}
      // else {
      //	std::cerr << "key's value's list didn't contain audio data." <<
      // std::endl;
      //}
    }
  }
}

double FaustProcessor::getReleaseLength() { return m_releaseLengthSec; }

void FaustProcessor::setReleaseLength(double sec) {
  m_releaseLengthSec = sec;
  if (m_dsp_poly) {
    m_dsp_poly->setReleaseLength(m_releaseLengthSec);
  }
}

#ifdef WIN32

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Find path to .dll */
// https://stackoverflow.com/a/57738892/12327461
HMODULE hMod;
std::wstring MyDLLPathFull;
std::wstring MyDLLDir;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  hMod = hModule;
  const int BUFSIZE = 4096;
  wchar_t buffer[BUFSIZE];
  if (::GetModuleFileNameW(hMod, buffer, BUFSIZE - 1) <= 0) {
    return TRUE;
  }

  MyDLLPathFull = buffer;

  size_t found = MyDLLPathFull.find_last_of(L"/\\");
  MyDLLDir = MyDLLPathFull.substr(0, found);

  return TRUE;
}

#else

// this applies to both __APPLE__ and linux?

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

// https://stackoverflow.com/a/51993539/911207
const char* getMyDLLPath(void) {
  Dl_info dl_info;
  dladdr((void*)getMyDLLPath, &dl_info);
  return (dl_info.dli_fname);
}
#endif

std::string getPathToFaustLibraries() {
  // Get the path to the directory containing basics.lib, stdfaust.lib etc.

  try {
#ifdef WIN32
    const std::wstring ws_shareFaustDir = MyDLLDir + L"\\faustlibraries";
    // std::cerr << "MyDLLDir: ";
    // std::wcerr << MyDLLDir << L'\n';
    // convert const wchar_t to char
    // https://stackoverflow.com/a/4387335
    const wchar_t* wc_shareFaustDir = ws_shareFaustDir.c_str();
    // Count required buffer size (plus one for null-terminator).
    size_t size = (wcslen(wc_shareFaustDir) + 1) * sizeof(wchar_t);
    char* char_shareFaustDir = new char[size];
    std::wcstombs(char_shareFaustDir, wc_shareFaustDir, size);

    std::string p(char_shareFaustDir);

    delete[] char_shareFaustDir;
    return p;
#else
    // this applies to __APPLE__ and LINUX
    const char* myDLLPath = getMyDLLPath();
    // std::cerr << "myDLLPath: " << myDLLPath << std::endl;
    std::filesystem::path p = std::filesystem::path(myDLLPath);
    p = p.parent_path() / "faustlibraries";
    return p.string();
#endif
  } catch (...) {
    throw std::runtime_error("Error getting path to faustlibraries.");
  }
}

#endif

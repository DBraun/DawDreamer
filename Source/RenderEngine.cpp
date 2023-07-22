#include "RenderEngine.h"

#include <unordered_map>

RenderEngine::RenderEngine(double sr, int bs)
    : AudioPlayHead{},
      mySampleRate{sr},
      myBufferSize{bs},
      m_mainProcessorGraph(std::make_unique<juce::AudioProcessorGraph>()) {
  m_mainProcessorGraph->setNonRealtime(true);
  m_mainProcessorGraph->setPlayHead(this);

  m_bpmAutomation.setSize(1, 1);
  m_bpmAutomation.setSample(0, 0, 120.);  // default 120 bpm
}

bool RenderEngine::removeProcessor(const std::string& name) {
  if (m_UniqueNameToNodeID.find(name) != m_UniqueNameToNodeID.end()) {
    m_mainProcessorGraph->removeNode(m_UniqueNameToNodeID[name]);
    m_UniqueNameToNodeID.erase(name);
    return true;
  }
  return false;
}

bool RenderEngine::loadGraph(DAG inDagNodes) {
  bool success = true;

  std::vector<DAGNode>* dagNodes = (std::vector<DAGNode>*)&inDagNodes;

  m_stringDag.clear();

  for (auto node : *dagNodes) {
    auto processorBase = node.processorBase;

    if (m_UniqueNameToNodeID.find(processorBase->getUniqueName()) ==
        m_UniqueNameToNodeID.end()) {
      m_stringDag.clear();
      throw std::runtime_error(
          "Error: Unable to find processor with unique name: " +
          processorBase->getUniqueName() + ".");
    }

    for (auto inputName : node.inputs) {
      if (m_UniqueNameToNodeID.find(processorBase->getUniqueName()) ==
          m_UniqueNameToNodeID.end()) {
        m_stringDag.clear();
        throw std::runtime_error(
            "Error: Unable to find processor with unique name: " + inputName +
            ".");
      }
    }

    m_stringDag.push_back(std::pair<std::string, std::vector<std::string>>(
        processorBase->getUniqueName(), node.inputs));
  }

  return success;
}

bool RenderEngine::connectGraph() {
  // remove all connections
  for (auto connection : m_mainProcessorGraph->getConnections()) {
    m_mainProcessorGraph->removeConnection(connection);
  }

  m_connectedProcessors.clear();

  for (auto entry : m_stringDag) {
    if (m_UniqueNameToNodeID.find(entry.first) == m_UniqueNameToNodeID.end()) {
      m_stringDag.clear();
      throw std::runtime_error(
          "Error: Unable to find processor with unique name: " + entry.first +
          ".");
    }

    auto node =
        m_mainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[entry.first]);

    auto processor = dynamic_cast<ProcessorBase*>(node->getProcessor());
    m_connectedProcessors.push_back(processor);

    if (!processor) {
      throw std::runtime_error(
          "Unable to cast to ProcessorBase during connectGraph.");
    }

    // Don't remove this prepareToPlay. In the case of Faust, it compiles the
    // code. This makes the number of input/output channels correct, which we
    // use a lines below in this function.
    processor->prepareToPlay(mySampleRate, myBufferSize);

    int numInputAudioChans = 0;

    std::string myUniqueName = processor->getUniqueName();

    auto inputNames = entry.second;

    for (auto inputName : inputNames) {
      if (m_UniqueNameToNodeID.find(inputName) == m_UniqueNameToNodeID.end()) {
        m_stringDag.clear();
        throw std::runtime_error(
            "Error: Unable to find processor with unique name: " + inputName +
            ".");
      }

      auto otherNode =
          m_mainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[inputName]);

      numInputAudioChans +=
          otherNode->getProcessor()->getMainBusNumOutputChannels();
    }

    int numOutputAudioChans = processor->getMainBusNumOutputChannels();
    int expectedInputChannels = processor->getMainBusNumInputChannels();
    if (numInputAudioChans > expectedInputChannels) {
      std::cerr << "Warning: Signals will be skipped. Processor named "
                << myUniqueName << " expects " << expectedInputChannels
                << " input signals, but you are trying to connect "
                << numInputAudioChans << " signals." << std::endl;
    }

    processor->setPlayConfigDetails(expectedInputChannels, numOutputAudioChans,
                                    mySampleRate, myBufferSize);

    int chanDest = 0;

    for (const std::string& inputName : inputNames) {
      auto inputNode =
          m_mainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[inputName]);

      auto inputProcessor = inputNode->getProcessor();

      for (int chanSource = 0;
           chanSource < inputProcessor->getMainBusNumOutputChannels();
           chanSource++) {
        AudioProcessorGraph::Connection connection = {
            {inputNode->nodeID, chanSource}, {node->nodeID, chanDest}};

        bool result = m_mainProcessorGraph->canConnect(connection) &&
                      m_mainProcessorGraph->addConnection(connection);
        if (!result) {
          // todo: because we failed here, connectGraph should return false at
          // the very end or immediately.
          std::cerr << "Warning: Unable to connect " << inputName << " channel "
                    << chanSource << " to " << myUniqueName << " channel "
                    << chanDest << std::endl;
          // return false;
        }

        chanDest++;
      }
    }

    processor->setConnectedInGraph(true);
  }

  m_mainProcessorGraph->setPlayConfigDetails(0, 0, mySampleRate, myBufferSize);
  m_mainProcessorGraph->prepareToPlay(mySampleRate, myBufferSize);

  return true;
}

float RenderEngine::getBPM(double ppqPosition) {
  int index = int(m_BPM_PPQN * ppqPosition);
  index = std::min(m_bpmAutomation.getNumSamples() - 1, index);

  auto bpm = m_bpmAutomation.getSample(0, index);

  return bpm;
}

int64_t RenderEngine::getRenderLength(const double renderLength, bool isBeats) {
  if (renderLength <= 0) {
    throw std::runtime_error("Render length must be greater than zero.");
  }

  if (!isBeats) {
    int64_t numRenderedSamples = (int64_t)(renderLength * mySampleRate);
    return numRenderedSamples;
  } else {
    double stepInSeconds = double(myBufferSize) / mySampleRate;

    PositionInfo posInfo;
    // It's important to initialize these.
    posInfo.setTimeInSamples(0);
    posInfo.setTimeInSeconds(0);
    posInfo.setPpqPosition(0);

    while (*posInfo.getPpqPosition() < renderLength) {
      posInfo.setBpm(getBPM(*posInfo.getPpqPosition()));
      posInfo.setTimeInSamples(*posInfo.getTimeInSamples() +
                               (int64_t)myBufferSize);
      posInfo.setTimeInSeconds(*posInfo.getTimeInSeconds() + stepInSeconds);
      posInfo.setPpqPosition(*posInfo.getPpqPosition() +
                             (stepInSeconds / 60.) * (*posInfo.getBpm()));
    }

    return *posInfo.getTimeInSamples();
  }
}

bool RenderEngine::render(const double renderLength, bool isBeats) {
  if (m_stringDag.empty()) {
    throw std::runtime_error("Cannot render an empty graph.");
  }

  int64_t numRenderedSamples = getRenderLength(renderLength, isBeats);

  auto numberOfBuffers =
      myBufferSize == 1 ? numRenderedSamples
                        : (int64_t)std::ceil(((double)numRenderedSamples - 1.) /
                                             myBufferSize);

  bool graphIsConnected = true;
  int audioBufferNumChans = 0;

  for (auto entry : m_stringDag) {
    if (m_UniqueNameToNodeID.find(entry.first) == m_UniqueNameToNodeID.end()) {
      throw std::runtime_error(
          "Unable to find processor named: " + entry.first + ".");
    }

    auto node =
        m_mainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[entry.first]);
    auto processor = node->getProcessor();

    audioBufferNumChans =
        std::max(audioBufferNumChans, processor->getTotalNumInputChannels());
    audioBufferNumChans =
        std::max(audioBufferNumChans, processor->getMainBusNumOutputChannels());

    auto processorBase = dynamic_cast<ProcessorBase*>(processor);
    if (processorBase) {
      graphIsConnected =
          graphIsConnected && processorBase->isConnectedInGraph();
    } else {
      throw std::runtime_error(
          "Unable to cast to Processor Base during render.");
    }
  }

  m_positionInfo = PositionInfo();
  m_positionInfo.setPpqPosition(0.);
  m_positionInfo.setIsPlaying(true);
  m_positionInfo.setIsRecording(true);
  m_positionInfo.setTimeInSeconds(0.);
  m_positionInfo.setTimeInSamples(0.);
  m_positionInfo.setBarCount(99999.);                        // todo:
  m_positionInfo.setHostTimeNs(1000000000. / mySampleRate);  // todo:
  m_positionInfo.setEditOriginTime(0.);                      // todo:
  auto timeSignature = TimeSignature();
  timeSignature.numerator = timeSignature.denominator = 4;
  m_positionInfo.setTimeSignature(timeSignature);
  m_positionInfo.setIsLooping(false);
  m_positionInfo.setBpm(getBPM(*m_positionInfo.getPpqPosition()));

  if (!graphIsConnected) {
    bool result = connectGraph();
    if (!result) {
      throw std::runtime_error("Unable to connect graph.");
    }
  }

  AudioSampleBuffer audioBuffer(audioBufferNumChans, myBufferSize);

  bool lastProcessorRecordEnable = false;

  for (auto entry : m_stringDag) {
    auto node =
        m_mainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[entry.first]);
    auto processor = dynamic_cast<ProcessorBase*>(node->getProcessor());

    if (processor) {
      if (entry == m_stringDag.at(m_stringDag.size() - 1)) {
        // Always force the last processor to record.
        lastProcessorRecordEnable = processor->getRecordEnable();
        processor->setRecordEnable(true);
      }
      processor->setRecorderLength((int)numRenderedSamples);
    } else {
      throw std::runtime_error(
          "Unable to cast to ProcessorBase during render.");
    }
  }

  // note that it's important for setRecorderLength to be called before reset,
  // because setRecorderLength sets `m_expectedRecordNumSamples` which is used
  // in reset.
  m_mainProcessorGraph->reset();

  MidiBuffer renderMidiBuffer;

  auto stepInMinutes = double(myBufferSize) / (mySampleRate * 60);

  for (int64_t i = 0; i < numberOfBuffers; ++i) {
    m_positionInfo.setBpm(getBPM(*m_positionInfo.getPpqPosition()));

    for (ProcessorBase* processor : m_connectedProcessors) {
      processor->automateParameters(m_positionInfo, myBufferSize);
      processor->recordAutomation(m_positionInfo, myBufferSize);
    }

    m_mainProcessorGraph->processBlock(audioBuffer, renderMidiBuffer);

    m_positionInfo.setTimeInSamples(*m_positionInfo.getTimeInSamples() +
                                    (int64_t)myBufferSize);
    m_positionInfo.setTimeInSeconds(double(*m_positionInfo.getTimeInSamples()) /
                                    mySampleRate);
    m_positionInfo.setPpqPosition(*m_positionInfo.getPpqPosition() +
                                  stepInMinutes * *m_positionInfo.getBpm());
  }

  m_positionInfo.setIsPlaying(false);
  m_positionInfo.setIsRecording(false);

  // restore the record-enable of the last processor.
  if (m_stringDag.size()) {
    auto node = m_mainProcessorGraph->getNodeForId(
        m_UniqueNameToNodeID[m_stringDag.at(m_stringDag.size() - 1).first]);
    auto processor = dynamic_cast<ProcessorBase*>(node->getProcessor());

    if (processor) {
      processor->setRecordEnable(lastProcessorRecordEnable);
    }
  }

  return true;
}

void RenderEngine::setBPM(double bpm) {
  if (bpm <= 0) {
    throw std::runtime_error("BPM must be positive.");
    return;
  }

  m_bpmAutomation.setSize(1, 1);
  m_bpmAutomation.setSample(0, 0, bpm);
}

bool RenderEngine::setBPMwithPPQN(py::array_t<float> input,
                                  std::uint32_t ppqn) {
  if (ppqn <= 0) {
    throw std::runtime_error(
        "The BPM's PPQN cannot be less than or equal to zero.");
  }

  if (input.ndim() > 1) {
    throw std::runtime_error("The BPM automation must be single dimensional.");
  }

  m_BPM_PPQN = ppqn;

  int numSamples = (int)input.shape(0);

  m_bpmAutomation.setSize(1, numSamples);

  m_bpmAutomation.copyFrom(0, 0, (float*)input.data(), numSamples);

  return true;
}

py::array_t<float> RenderEngine::getAudioFrames() {
  if (m_mainProcessorGraph->getNumNodes() == 0 || m_stringDag.size() == 0) {
    // NB: For some reason we can't initialize the array as shape (2, 0)
    py::array_t<float, py::array::c_style> arr({2, 1});
    arr.resize({2, 0});

    return arr;
  }

  return getAudioFramesForName(m_stringDag.at(m_stringDag.size() - 1).first);
}

py::array_t<float> RenderEngine::getAudioFramesForName(std::string& name) {
  if (m_UniqueNameToNodeID.find(name) != m_UniqueNameToNodeID.end()) {
    auto node = m_mainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[name]);

    auto processor = dynamic_cast<ProcessorBase*>(node->getProcessor());
    if (processor) {
      if (std::strcmp(processor->getUniqueName().c_str(), name.c_str()) == 0) {
        return processor->getAudioFrames();
      }
    }
  }

  // NB: For some reason we can't initialize the array as shape (2, 0)
  py::array_t<float, py::array::c_style> arr({2, 1});
  arr.resize({2, 0});

  return arr;
}

juce::Optional<juce::AudioPlayHead::PositionInfo> RenderEngine::getPosition()
    const {
  return m_positionInfo;
}

/** Returns true if this object can control the transport. */
bool RenderEngine::canControlTransport() { return false; }

/** Starts or stops the audio. */
void RenderEngine::transportPlay(bool shouldStartPlaying) {}

/** Starts or stops recording the audio. */
void RenderEngine::transportRecord(bool shouldStartRecording) {}

/** Rewinds the audio. */
void RenderEngine::transportRewind() {}

// pybind11 functions:

void RenderEngine::prepareProcessor(ProcessorBase* processor,
                                    const std::string& name) {
  if (this->removeProcessor(name)) {
    std::cerr << "Warning: a processor with the name \"" << name
              << "\" already exists and was removed to make room for the new "
                 "processor."
              << std::endl;
  };

  auto node = m_mainProcessorGraph->addNode(
      (std::unique_ptr<ProcessorBase>)(processor));
  m_UniqueNameToNodeID[name] = node->nodeID;
}

OscillatorProcessor* RenderEngine::makeOscillatorProcessor(
    const std::string& name, float freq) {
  auto processor = new OscillatorProcessor{name, freq};
  this->prepareProcessor(processor, name);
  return processor;
}

PluginProcessorWrapper* RenderEngine::makePluginProcessor(
    const std::string& name, const std::string& path) {
  auto processor =
      new PluginProcessorWrapper{name, mySampleRate, myBufferSize, path};
  this->prepareProcessor(processor, name);
  return processor;
}

PlaybackProcessor* RenderEngine::makePlaybackProcessor(const std::string& name,
                                                       py::array data) {
  auto processor = new PlaybackProcessor{name, data};
  this->prepareProcessor(processor, name);
  return processor;
}

#ifdef BUILD_DAWDREAMER_RUBBERBAND
PlaybackWarpProcessor* RenderEngine::makePlaybackWarpProcessor(
    const std::string& name, py::array data, double data_sr) {
  auto processor = new PlaybackWarpProcessor{name, data, mySampleRate, data_sr};
  this->prepareProcessor(processor, name);
  return processor;
}
#endif

FilterProcessor* RenderEngine::makeFilterProcessor(const std::string& name,
                                                   const std::string& mode,
                                                   float freq, float q,
                                                   float gain) {
  float validFreq = std::fmax(.0001f, freq);
  float validQ = std::fmax(.0001f, q);
  float validGain = std::fmax(.0001f, gain);

  auto processor =
      new FilterProcessor{name, mode, validFreq, validQ, validGain};
  this->prepareProcessor(processor, name);
  return processor;
}

CompressorProcessor* RenderEngine::makeCompressorProcessor(
    const std::string& name, float threshold = 0.f, float ratio = 2.f,
    float attack = 2.f, float release = 50.f) {
  // ratio must be >= 1.0
  // attack and release are in milliseconds
  float validRatio = std::fmax(1.0f, ratio);
  float validAttack = std::fmax(0.f, attack);
  float validRelease = std::fmax(0.f, release);

  auto processor = new CompressorProcessor{name, threshold, validRatio,
                                           validAttack, validRelease};
  this->prepareProcessor(processor, name);
  return processor;
}

AddProcessor* RenderEngine::makeAddProcessor(const std::string& name,
                                             std::vector<float> gainLevels) {
  auto processor = new AddProcessor{name, gainLevels};
  this->prepareProcessor(processor, name);
  return processor;
}

ReverbProcessor* RenderEngine::makeReverbProcessor(
    const std::string& name, float roomSize = 0.5f, float damping = 0.5f,
    float wetLevel = 0.33f, float dryLevel = 0.4f, float width = 1.0f) {
  auto processor =
      new ReverbProcessor{name, roomSize, damping, wetLevel, dryLevel, width};
  this->prepareProcessor(processor, name);
  return processor;
}

PannerProcessor* RenderEngine::makePannerProcessor(const std::string& name,
                                                   std::string& rule,
                                                   float pan) {
  float safeVal = std::fmax(-1.f, pan);
  safeVal = std::fmin(1.f, pan);

  auto processor = new PannerProcessor{name, rule, safeVal};
  this->prepareProcessor(processor, name);
  return processor;
}

DelayProcessor* RenderEngine::makeDelayProcessor(const std::string& name,
                                                 std::string& rule, float delay,
                                                 float wet) {
  float safeDelay = std::fmax(0.f, delay);

  float safeWet = std::fmin(1.f, std::fmax(0.f, wet));
  auto processor = new DelayProcessor{name, rule, safeDelay, safeWet};

  this->prepareProcessor(processor, name);

  return processor;
}

SamplerProcessor* RenderEngine::makeSamplerProcessor(const std::string& name,
                                                     py::array data) {
  auto processor = new SamplerProcessor{name, data, mySampleRate, myBufferSize};
  this->prepareProcessor(processor, name);
  return processor;
}

#ifdef BUILD_DAWDREAMER_FAUST
FaustProcessor* RenderEngine::makeFaustProcessor(const std::string& name) {
  auto processor = new FaustProcessor{name, mySampleRate, myBufferSize};
  this->prepareProcessor(processor, name);
  return processor;
}
#endif

bool RenderEngine::loadGraphWrapper(py::object dagObj) {
  if (!py::isinstance<py::list>(dagObj)) {
    throw std::runtime_error("Error: load_graph. No processors were passed.");
  }

  std::unique_ptr<DAG> buildingDag(new DAG());

  for (py::handle theTuple : dagObj) {  // iterators!

    if (!py::isinstance<py::tuple>(theTuple) &&
        !py::isinstance<py::list>(theTuple)) {
      throw std::runtime_error(
          "Error: load_graph. Received graph that is not a list.");
    }
    py::list castedTuple = theTuple.cast<py::list>();

    if (castedTuple.size() != 2) {
      throw std::runtime_error(
          "Error: load_graph. Each tuple in the graph must be size 2.");
    }

    // todo: enable this:
    // if (!py::isinstance<ProcessorBase*>(castedTuple[0])) {
    // std::cout << "Error: load_graph. First argument in tuple wasn't a
    // Processor object." << std::endl;
    //    return false;
    //}
    if (!py::isinstance<py::list>(castedTuple[1])) {
      throw std::runtime_error(
          "Error: load_graph. Something was wrong with the list of inputs.");
    }

    py::list listOfStrings = castedTuple[1].cast<py::list>();

    std::vector<std::string> inputs;

    for (py::handle potentialString : listOfStrings) {
      if (!py::isinstance<py::str>(potentialString)) {
        throw std::runtime_error(
            "Error: load_graph. Something was wrong with the list of inputs.");
      }

      inputs.push_back(potentialString.cast<std::string>());
    }

    DAGNode dagNode;
    try {
      dagNode.processorBase = castedTuple[0].cast<ProcessorBase*>();
    } catch (std::exception&) {
      throw std::runtime_error(
          "Error: Load_graph. First argument in tuple wasn't a Processor "
          "object.");
    }

    dagNode.inputs = inputs;

    buildingDag->nodes.push_back(dagNode);
  }

  auto result = RenderEngine::loadGraph(*buildingDag);

  return result;
}

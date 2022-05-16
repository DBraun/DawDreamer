#include "RenderEngine.h"

#include <unordered_map>

RenderEngine::RenderEngine(double sr, int bs) :
    mySampleRate{ sr },
    myBufferSize{ bs },
    myMainProcessorGraph(new juce::AudioProcessorGraph())
{
    myMainProcessorGraph->setNonRealtime(true);
    myMainProcessorGraph->setPlayHead(this);

    bpmAutomation.setSize(1, 1);
    bpmAutomation.setSample(0, 0, 120.); // default 120 bpm
}

bool
RenderEngine::removeProcessor(const std::string& name) {
    if (m_UniqueNameToNodeID.find(name) != m_UniqueNameToNodeID.end()) {
        myMainProcessorGraph->removeNode(m_UniqueNameToNodeID[name]);
        m_UniqueNameToNodeID.erase(name);
        return true;
    }
    return false;
}

bool
RenderEngine::loadGraph(DAG inDagNodes) {
    bool success = true;
    
    std::vector<DAGNode>* dagNodes = (std::vector<DAGNode>*) &inDagNodes;

    m_stringDag.clear();

    for (auto node : *dagNodes) {
        auto processorBase = node.processorBase;

        if (m_UniqueNameToNodeID.find(processorBase->getUniqueName()) == m_UniqueNameToNodeID.end()) {
            m_stringDag.clear();
            throw std::runtime_error("Error: Unable to find processor with unique name: " + processorBase->getUniqueName() + ".");
        }

        for (auto inputName : node.inputs) {
            if (m_UniqueNameToNodeID.find(processorBase->getUniqueName()) == m_UniqueNameToNodeID.end()) {
                m_stringDag.clear();
                throw std::runtime_error("Error: Unable to find processor with unique name: " + inputName + ".");
            }
        }

        m_stringDag.push_back(
            std::pair<std::string, std::vector<std::string>>(processorBase->getUniqueName(), node.inputs)
        );
    }

    return success;
}

bool
RenderEngine::connectGraph() {

    // remove all connections
    for (auto connection : myMainProcessorGraph->getConnections()) {
        myMainProcessorGraph->removeConnection(connection);
    }

    for (auto entry : m_stringDag) {

        if (m_UniqueNameToNodeID.find(entry.first) == m_UniqueNameToNodeID.end()) {
            m_stringDag.clear();
            throw std::runtime_error("Error: Unable to find processor with unique name: " + entry.first + ".");
        }

        auto node = myMainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[entry.first]);
        
        auto processor = dynamic_cast<ProcessorBase*> (node->getProcessor());

        if (!processor) {
            throw std::runtime_error("Unable to cast to ProcessorBase during connectGraph.");
        }
        
        int numInputAudioChans = 0;
                
        std::string myUniqueName = processor->getUniqueName();
        
        auto inputNames = entry.second;

        for (auto inputName : inputNames) {

            if (m_UniqueNameToNodeID.find(inputName) == m_UniqueNameToNodeID.end()) {
                m_stringDag.clear();
                throw std::runtime_error("Error: Unable to find processor with unique name: " + inputName + ".");
            }

            auto otherNode = myMainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[inputName]);

            numInputAudioChans += otherNode->getProcessor()->getMainBusNumOutputChannels();
        }
                
        processor->setPlayHead(this);
        processor->prepareToPlay(mySampleRate, myBufferSize);
        processor->automateParameters();
        int numOutputAudioChans = processor->getMainBusNumOutputChannels();
        int expectedInputChannels = processor->getMainBusNumInputChannels();
        if (numInputAudioChans > expectedInputChannels) {
            std::cerr << "Warning: Signals will be skipped. Processor named " << myUniqueName << " expects " << expectedInputChannels << " input signals, but you are trying to connect " << numInputAudioChans << " signals." << std::endl;
        }
        
        processor->setPlayConfigDetails(expectedInputChannels, numOutputAudioChans, mySampleRate, myBufferSize);
        
        int chanDest = 0;

        for (const std::string inputName : inputNames) {
            
            auto inputNode = myMainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[inputName]);

            auto inputProcessor = inputNode->getProcessor();
            
            for (int chanSource = 0; chanSource < inputProcessor->getMainBusNumOutputChannels(); chanSource++) {

                AudioProcessorGraph::Connection connection = {
                    { inputNode->nodeID, chanSource },
                    { node->nodeID, chanDest } };

                bool result = myMainProcessorGraph->canConnect(connection) && myMainProcessorGraph->addConnection(connection);
                if (!result) {
                    // todo: because we failed here, connectGraph should return false at the very end or immediately.
                    std::cerr << "Warning: Unable to connect " << inputName << " channel " << chanSource << " to " << myUniqueName << " channel " << chanDest << std::endl;
                    //return false;
                }
                
                chanDest++;
            }
        }

        processor->setConnectedInGraph(true);
        
    }

    myMainProcessorGraph->setPlayConfigDetails(0, 0, mySampleRate, myBufferSize);
    myMainProcessorGraph->prepareToPlay(mySampleRate, myBufferSize);
    
    return true;
}

float RenderEngine::getBPM(double ppqPosition) {

    int index = int(ProcessorBase::PPQN * ppqPosition);
    index = std::min(bpmAutomation.getNumSamples() - 1, index);

    auto bpm = bpmAutomation.getSample(0, index);

    return bpm;
}

bool
RenderEngine::render(const double renderLength) {

    std::uint64_t numRenderedSamples = renderLength * mySampleRate;
    if (numRenderedSamples <= 0) {
        throw std::runtime_error("Render length must be greater than zero.");
    }
    
    std::uint64_t numberOfBuffers = myBufferSize == 1 ? numRenderedSamples : std::uint64_t(std::ceil((numRenderedSamples -1.) / myBufferSize));

    bool graphIsConnected = true;
    int audioBufferNumChans = 0;

    for (auto entry : m_stringDag) {

        if (m_UniqueNameToNodeID.find(entry.first) == m_UniqueNameToNodeID.end()) {
            throw std::runtime_error("Unable to find processor named: " + entry.first + ".");
        }

        auto node = myMainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[entry.first]);
        auto processor = node->getProcessor();

        audioBufferNumChans = std::max(audioBufferNumChans, processor->getTotalNumInputChannels());
        audioBufferNumChans = std::max(audioBufferNumChans, processor->getMainBusNumOutputChannels());

        auto processorBase = dynamic_cast<ProcessorBase*> (processor);
        if (processorBase) {
            graphIsConnected = graphIsConnected && processorBase->isConnectedInGraph();
        }
        else {
            throw std::runtime_error("Unable to cast to Processor Base during render.");
        }
    }

    myMainProcessorGraph->reset();
    myMainProcessorGraph->setPlayHead(this);

    myCurrentPositionInfo.resetToDefault();
    myCurrentPositionInfo.ppqPosition = 0.;
    myCurrentPositionInfo.isPlaying = true;
    myCurrentPositionInfo.isRecording = true;
    myCurrentPositionInfo.timeInSeconds = 0;
    myCurrentPositionInfo.timeInSamples = 0;
    myCurrentPositionInfo.timeSigNumerator = 4;
    myCurrentPositionInfo.timeSigDenominator = 4;
    myCurrentPositionInfo.isLooping = false;
    myCurrentPositionInfo.bpm = getBPM(myCurrentPositionInfo.ppqPosition);
    
    if (!graphIsConnected) {
        bool result = connectGraph();
        if (!result) {
            throw std::runtime_error("Unable to connect graph.");
        }
    }
    
    AudioSampleBuffer audioBuffer(audioBufferNumChans, myBufferSize);

    bool lastProcessorRecordEnable = false;
    
    for (auto entry : m_stringDag) {
        auto node = myMainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[entry.first]);
        auto processor = dynamic_cast<ProcessorBase*> (node->getProcessor());

        if (processor) {
            if (entry == m_stringDag.at(m_stringDag.size()-1)) {
                // Always force the last processor to record.
                lastProcessorRecordEnable = processor->getRecordEnable();
                processor->setRecordEnable(true);
            }
            processor->setRecorderLength(processor->getRecordEnable() ? numRenderedSamples : 0);
        }
        else {
            throw std::runtime_error("Unable to cast to ProcessorBase during render.");
        }
    }

    MidiBuffer renderMidiBuffer;

    auto stepInSeconds = double(myBufferSize) / mySampleRate;
    
    for (std::uint64_t i = 0; i < numberOfBuffers; ++i)
    {
        myCurrentPositionInfo.bpm = getBPM(myCurrentPositionInfo.ppqPosition);

        myMainProcessorGraph->processBlock(audioBuffer, renderMidiBuffer);

        myCurrentPositionInfo.timeInSamples += myBufferSize;
        myCurrentPositionInfo.timeInSeconds += stepInSeconds;
        myCurrentPositionInfo.ppqPosition += (stepInSeconds / 60.) * myCurrentPositionInfo.bpm;
    }

    myCurrentPositionInfo.isPlaying = false;
    myCurrentPositionInfo.isRecording = false;

    // restore the record-enable of the last processor.
    if (m_stringDag.size()) {
        
        auto node = myMainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[m_stringDag.at(m_stringDag.size() - 1).first]);
        auto processor = dynamic_cast<ProcessorBase*> (node->getProcessor());

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
    bpmAutomation.setSize(1, 1);
    bpmAutomation.setSample(0, 0, bpm);
}

bool RenderEngine::setBPMVec(py::array_t<float> input) {

    auto numSamples = input.shape(0);

    bpmAutomation.setSize(1, numSamples);

    bpmAutomation.copyFrom(0, 0, (float*)input.data(), numSamples);

    return true;
}

py::array_t<float>
RenderEngine::getAudioFrames()
{    
    if (myMainProcessorGraph->getNumNodes() == 0 || m_stringDag.size() == 0) {
        // NB: For some reason we can't initialize the array as shape (2, 0)
        py::array_t<float, py::array::c_style> arr({ 2, 1 });
        arr.resize({ 2, 0 });

        return arr;
    }

    return getAudioFramesForName(m_stringDag.at(m_stringDag.size() - 1).first);
}

py::array_t<float>
RenderEngine::getAudioFramesForName(std::string& name)
{

    if (m_UniqueNameToNodeID.find(name) != m_UniqueNameToNodeID.end()) {
        auto node = myMainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[name]);

        auto processor = dynamic_cast<ProcessorBase*>(node->getProcessor());
        if (processor) {
            if (std::strcmp(processor->getUniqueName().c_str(), name.c_str()) == 0) {
                return processor->getAudioFrames();
            }
        }
    }

    // NB: For some reason we can't initialize the array as shape (2, 0)
    py::array_t<float, py::array::c_style> arr({ 2, 1 });
    arr.resize({ 2, 0 });

    return arr;
}

bool
RenderEngine::getCurrentPosition(CurrentPositionInfo& result) {
    result = myCurrentPositionInfo;
    return true;
};

/** Returns true if this object can control the transport. */
bool
RenderEngine::canControlTransport() { return true; }

/** Starts or stops the audio. */
void
RenderEngine::transportPlay(bool shouldStartPlaying) { }

/** Starts or stops recording the audio. */
void
RenderEngine::transportRecord(bool shouldStartRecording) { }

/** Rewinds the audio. */
void
RenderEngine::transportRewind() {}

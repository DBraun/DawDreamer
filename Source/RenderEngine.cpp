#include "RenderEngine.h"
#include "AllProcessors.h"

#include <unordered_map>

RenderEngine::RenderEngine(double sr, int bs) :
    mySampleRate{ sr },
    myBufferSize{ bs },
    myMainProcessorGraph(new juce::AudioProcessorGraph())
{
    myMainProcessorGraph->setNonRealtime(true);
    myMainProcessorGraph->setPlayHead(this);
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

    myMainProcessorGraph->enableAllBuses();
    // NB: don't enableAllBuses on all the nodes in the graph because
    // it will actually mess them up (FaustProcessor)
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
    for (auto entry : m_stringDag) {
        auto node = myMainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[entry.first]);
        node->getProcessor()->prepareToPlay(mySampleRate, myBufferSize);
    }
    
    return true;
}

bool
RenderEngine::render(const double renderLength) {

    int numRenderedSamples = renderLength * mySampleRate;
    if (numRenderedSamples <= 0) {
        throw std::runtime_error("Render length must be greater than zero.");
    }
    
    int numberOfBuffers = myBufferSize == 1 ? numRenderedSamples : int(std::ceil((numRenderedSamples -1.) / myBufferSize));

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

        auto faustProcessor = dynamic_cast<FaustProcessor*> (processor);
        if (faustProcessor && (!faustProcessor->isCompiled())) {
            if (!faustProcessor->compile()) {
                return false;
            }
        }
    }

    myMainProcessorGraph->reset();
    myMainProcessorGraph->setPlayHead(this);

    myCurrentPositionInfo.resetToDefault();
    myCurrentPositionInfo.bpm = myBPM;
    myCurrentPositionInfo.isPlaying = true;
    myCurrentPositionInfo.isRecording = true;
    myCurrentPositionInfo.timeInSamples = 0;
    myCurrentPositionInfo.timeSigNumerator = 4;
    myCurrentPositionInfo.timeSigDenominator = 4;
    myCurrentPositionInfo.isLooping = false;
    
    if (!graphIsConnected) {
        bool result = connectGraph();
        if (!result) {
            throw std::runtime_error("Unable to connect graph.");
        }
    }
    
    AudioSampleBuffer audioBuffer(audioBufferNumChans, myBufferSize);
    
    for (auto entry : m_stringDag) {
        auto node = myMainProcessorGraph->getNodeForId(m_UniqueNameToNodeID[entry.first]);
        auto processor = dynamic_cast<ProcessorBase*> (node->getProcessor());

        if (processor) {
            if (entry == m_stringDag.at(m_stringDag.size()-1)) {
                // Always force the last processor to record.
                processor->setRecordEnable(true);
            }
            processor->setRecorderLength(processor->getRecordEnable() ? numRenderedSamples : 0);
        }
        else {
            throw std::runtime_error("Unable to cast to ProcessorBase during render.");
        }
    }

    MidiBuffer renderMidiBuffer;
    
    for (long long int i = 0; i < numberOfBuffers; ++i)
    {
        myMainProcessorGraph->processBlock(audioBuffer, renderMidiBuffer);

        myCurrentPositionInfo.timeInSamples += myBufferSize;
        myCurrentPositionInfo.ppqPosition = (myCurrentPositionInfo.timeInSamples / (mySampleRate * 60.)) * myBPM;
    }

    myCurrentPositionInfo.isPlaying = false;
    myCurrentPositionInfo.isRecording = false;
    
    return true;
}

void RenderEngine::setBPM(double bpm) {
    if (bpm <= 0) {
        throw std::runtime_error("BPM must be positive.");
        return;
    }
    myBPM = bpm;
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

#include "RenderEngine.h"
#include <unordered_map>

RenderEngine::RenderEngine(double sr, int bs) :
    mySampleRate{ sr },
    myBufferSize{ bs }
{
    myMainProcessorGraph.reset(new juce::AudioProcessorGraph());
    myMainProcessorGraph->setNonRealtime(true);
    myMainProcessorGraph->setPlayHead(this);
}

RenderEngine::~RenderEngine()
{
    //int numNodes = myMainProcessorGraph->getNumNodes();
    //for (int i = 0; i < numNodes; i++) {
    //    auto processor = myMainProcessorGraph->getNode(i)->decReferenceCountWithoutDeleting();
    //}
    //myMainProcessorGraph->releaseResources();
    myMainProcessorGraph.reset();
}

bool
RenderEngine::loadGraph(DAG inDagNodes) {

    bool success = true;
    
    std::vector<DAGNode>* dagNodes = (std::vector<DAGNode>*) &inDagNodes;

    myMainProcessorGraph->clear();

    using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;

    int nodeInt = 0;

    m_UniqueNameToSlotIndex.clear();
    m_UniqueNameToInputs.clear();
    
    slots.clear();

    for (auto node : *dagNodes) {
        auto processorBase = node.processorBase;

        auto myNode = myMainProcessorGraph->addNode((std::unique_ptr<ProcessorBase>)processorBase);

        // todo: does incReferenceCount() cause memory leak??
        // If we don't do it, later calls to this function to load a new graph crash at myMainProcessorGraph->clear();
        myNode.get()->incReferenceCount();

        slots.set(nodeInt, myNode);
        m_UniqueNameToSlotIndex[processorBase->getUniqueName()] = nodeInt;
        m_UniqueNameToInputs[processorBase->getUniqueName()] = node.inputs;

        nodeInt++;
    }

    myMainProcessorGraph->enableAllBuses();
    // NB: don't enableAllBuses on all the processors in the graph because
    // it will actually mess them up (FaustProcessor)
    
    return success;
}

bool
RenderEngine::connectGraph() {

    // remove all connections
    for (auto connection : myMainProcessorGraph->getConnections()) {
        myMainProcessorGraph->removeConnection(connection);
    }
     
    int numNodes = myMainProcessorGraph->getNumNodes();
    for (int i = 0; i < numNodes; i++) {
        auto processor = dynamic_cast<ProcessorBase*> (myMainProcessorGraph->getNode(i)->getProcessor());
        
        int numInputAudioChans = 0;
                
        std::string myUniqueName = processor->getUniqueName();
        
        auto inputs = m_UniqueNameToInputs[myUniqueName];
                
        for (auto otherNode : myMainProcessorGraph->getNodes()) {
            auto otherName = ((ProcessorBase*) otherNode->getProcessor())->getUniqueName();
            
            if (std::find(inputs.begin(), inputs.end(), otherName) != inputs.end()) {
                numInputAudioChans += otherNode->getProcessor()->getMainBusNumOutputChannels();
            }
        }

        processor->setPlayHead(this);
        processor->automateParameters();
        int numOutputAudioChans = processor->getMainBusNumOutputChannels();
        int expectedInputChannels = processor->getMainBusNumInputChannels();
        if (numInputAudioChans > expectedInputChannels) {
            // todo: enable this warning
            //std::cerr << "Warning: Signals will be skipped. Processor named " << myUniqueName << " expects " << expectedInputChannels << " input signals, but you are trying to connect " << numInputAudioChans << " signals." << std::endl;
        }
        
        processor->setPlayConfigDetails(expectedInputChannels, numOutputAudioChans, mySampleRate, myBufferSize);
        
        int chanDest = 0;

        for (const std::string inputName : m_UniqueNameToInputs[myUniqueName]) {

            if (m_UniqueNameToSlotIndex.find(inputName) == m_UniqueNameToSlotIndex.end())
            {
                throw std::runtime_error("Error: Unable to connect " + inputName + " to " + myUniqueName + "; You might need to place " + inputName + " earlier in the graph.");
                return false;
            }
            
            int slotIndexOfInput = m_UniqueNameToSlotIndex[inputName];

            auto inputProcessor = myMainProcessorGraph->getNode(slotIndexOfInput)->getProcessor();
            
            for (int chanSource = 0; chanSource < inputProcessor->getMainBusNumOutputChannels(); chanSource++) {

                AudioProcessorGraph::Connection connection = { { slots.getUnchecked(slotIndexOfInput)->nodeID, chanSource },
                                                { slots.getUnchecked(i)->nodeID, chanDest } };

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
    for (auto node : myMainProcessorGraph->getNodes()) {
        node->getProcessor()->prepareToPlay(mySampleRate, myBufferSize);
    }
    
    return true;
}

bool
RenderEngine::render(const double renderLength) {

    int numRenderedSamples = renderLength * mySampleRate;
    if (numRenderedSamples <= 0) {
        throw std::runtime_error("Render length must be greater than zero.");
        return false;
    }
    
    int numberOfBuffers = myBufferSize == 1 ? numRenderedSamples : int(std::ceil((numRenderedSamples -1.) / myBufferSize));

    bool graphIsConnected = true;
    int audioBufferNumChans = 0;

    int numNodes = myMainProcessorGraph->getNumNodes();
    for (int i = 0; i < numNodes; i++) {
        auto processor = myMainProcessorGraph->getNode(i)->getProcessor();

        audioBufferNumChans = std::max(audioBufferNumChans, processor->getTotalNumInputChannels());
        audioBufferNumChans = std::max(audioBufferNumChans, processor->getMainBusNumOutputChannels());

        auto faustProcessor = dynamic_cast<FaustProcessor*> (processor);
        if (faustProcessor && (!faustProcessor->isCompiled())) {
            if (!faustProcessor->compile()) {
                return false;
            }
        }
        auto processorBase = dynamic_cast<ProcessorBase*> (myMainProcessorGraph->getNode(i)->getProcessor());
        if (processorBase) {
            graphIsConnected = graphIsConnected && processorBase->isConnectedInGraph();
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
            return false;
        }
    }
    
    AudioSampleBuffer audioBuffer(audioBufferNumChans, myBufferSize);
    
    for (int i = 0; i < numNodes; i++) {
        auto processor = dynamic_cast<ProcessorBase*> (myMainProcessorGraph->getNode(i)->getProcessor());
        if (processor) {
            if (i == numNodes-1) {
                // always force the last processor to record. todo: maybe this is clumsy.
                processor->setRecordEnable(true);
            }
            processor->setRecorderLength(processor->getRecordEnable() ? numRenderedSamples : 0);
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
    auto nodes = myMainProcessorGraph->getNodes();
    
    if (nodes.size() == 0) {
        // NB: For some reason we can't initialize the array as shape (2, 0)
        py::array_t<float, py::array::c_style> arr({ 2, 1 });
        arr.resize({ 2, 0 });

        return arr;
    }
    
    auto node = nodes.getLast();
    
    auto processor = dynamic_cast<ProcessorBase*>(node->getProcessor());
    if (processor) {
        auto uniqueName = processor->getUniqueName();
        return getAudioFramesForName(uniqueName);
    }

    // NB: For some reason we can't initialize the array as shape (2, 0)
    py::array_t<float, py::array::c_style> arr({ 2, 1 });
    arr.resize({ 2, 0 });

    return arr;
}

py::array_t<float>
RenderEngine::getAudioFramesForName(std::string& name)
{
    auto nodes = myMainProcessorGraph->getNodes();
    for (auto& node : nodes) {

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

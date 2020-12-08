#include "RenderEngine.h"
#include <unordered_map>

RenderEngine::RenderEngine(double sr, int bs) :
    mySampleRate{ sr },
    myBufferSize{ bs }
{
    myMainProcessorGraph.reset(new juce::AudioProcessorGraph());
    myMainProcessorGraph->setNonRealtime(true);
}

RenderEngine::~RenderEngine()
{
    myMainProcessorGraph->releaseResources();
}

bool
RenderEngine::loadGraph(DAG inDagNodes, int numInputAudioChans=2, int numOutputAudioChans=2) {

    std::vector<DAGNode>* dagNodes = (std::vector<DAGNode>*) &inDagNodes;

    myMainProcessorGraph->clear();

    myNumInputAudioChans = numInputAudioChans;
    myNumOutputAudioChans = numOutputAudioChans;

    using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;

    myMidiInputNode = myMainProcessorGraph->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiInputNode));

    juce::ReferenceCountedArray<juce::AudioProcessorGraph::Node> slots;
    int nodeInt = 0;

    std::unordered_map<std::string, int> uniqueNameToSlotIndex;

    for (auto node : *dagNodes) {
        auto processorBase = node.processorBase;
        std::vector<std::string> inputs = node.inputs;

        auto myNode = myMainProcessorGraph->addNode((std::unique_ptr<ProcessorBase>)processorBase);

        // todo: does incReferenceCount() cause memory leak??
        // If we don't do it, later calls to this function to load a new graph crash at
        // myMainProcessorGraph->clear();
        myNode.get()->incReferenceCount();

        slots.set(nodeInt, myNode);
        //slots.getUnchecked(nodeInt)->getProcessor()->setNonRealtime(true); // assume processors are initialized in non-real-time mode.
        slots.getUnchecked(nodeInt)->getProcessor()->setPlayConfigDetails(myNumOutputAudioChans *(int)(inputs.size()),
            myNumOutputAudioChans,
            mySampleRate, myBufferSize);

        if (processorBase->acceptsMidi()) {
            // Connect MIDI.
            // Assume the first node is the one that needs to receive MIDI.
            myMainProcessorGraph->addConnection({ { myMidiInputNode->nodeID,  juce::AudioProcessorGraph::midiChannelIndex },
                                            { slots.getUnchecked(nodeInt)->nodeID, juce::AudioProcessorGraph::midiChannelIndex } });
        }

        uniqueNameToSlotIndex[processorBase->getUniqueName()] = nodeInt;

        int inputIndex = 0;
        for (const std::string inputName : inputs) {

            if (uniqueNameToSlotIndex.find(inputName) == uniqueNameToSlotIndex.end())
            {
                std::cout << "Error connecting " << inputName << " to " << processorBase->getUniqueName() << ";" << std::endl;
                std::cout << "You might need to place " << inputName << " earlier in the graph." << std::endl;
                continue;
            }

            int slotIndexOfInput = uniqueNameToSlotIndex[inputName];

            for (int channel = 0; channel < myNumOutputAudioChans; ++channel) {
                int chanSource = channel;
                int chanDest = inputIndex * myNumOutputAudioChans + channel;
                bool result = myMainProcessorGraph->addConnection({ { slots.getUnchecked(slotIndexOfInput)->nodeID, chanSource },
                                                { slots.getUnchecked(nodeInt)->nodeID, chanDest } });
                if (!result) {
                    std::cout << "Error connecting " << inputName << " " << chanSource << " to " << processorBase->getUniqueName() << " " << chanDest << std::endl;
                }
            }

            inputIndex++;
        }

        nodeInt++;
    }

    if (!slots.isEmpty()) {

        auto lastNodeID = slots.getLast()->nodeID;

        slots.set(nodeInt, myMainProcessorGraph->addNode(std::make_unique<RecorderProcessor>("_output_recorder")));
        slots.getUnchecked(nodeInt)->getProcessor()->setPlayConfigDetails(myNumInputAudioChans,
            myNumOutputAudioChans,
            mySampleRate, myBufferSize);
        slots.getUnchecked(nodeInt)->getProcessor()->prepareToPlay(mySampleRate, myBufferSize);

        auto recorderNodeID = slots.getUnchecked(nodeInt)->nodeID;

        for (int channel = 0; channel < myNumOutputAudioChans; ++channel)
        {
            bool result = myMainProcessorGraph->addConnection({ { lastNodeID, channel },
                    { recorderNodeID, channel } });
            if (!result) {
                std::cout << "unable to connect to recorderNode" << std::endl;
            }
        }
    }

    for (auto node : myMainProcessorGraph->getNodes()) {
        node->getProcessor()->enableAllBuses();
    }

    myMainProcessorGraph->setPlayConfigDetails(myNumInputAudioChans,
        myNumOutputAudioChans,
        mySampleRate, myBufferSize);

    myMainProcessorGraph->prepareToPlay(mySampleRate, myBufferSize);
    for (auto node : myMainProcessorGraph->getNodes()) {
        node->getProcessor()->prepareToPlay(mySampleRate, myBufferSize);
    }

    return true;
}

void
RenderEngine::render(const double renderLength) {
    int numberOfBuffers = int(std::ceil(renderLength * mySampleRate / myBufferSize));

    const long long int numSamples = numberOfBuffers * myBufferSize;
    AudioSampleBuffer audioBuffer(myNumOutputAudioChans, myBufferSize);

    // Clear main buffer and prepare to record samples over multiple buffer passes.
    myRecordedSamples.clear();
    myRecordedSamples = std::vector<std::vector<float>>(myNumOutputAudioChans, std::vector<float>(numSamples));
    for (size_t i = 0; i < myNumOutputAudioChans; i++)
    {
        std::fill(myRecordedSamples[i].begin(), myRecordedSamples[i].end(), 0.f);
    }

    myMainProcessorGraph->reset();

    RecorderProcessor* recorder = (RecorderProcessor*)(myMainProcessorGraph->getNode(myMainProcessorGraph->getNumNodes() - 1)->getProcessor());

    // the Recorder holds a pointer to the engine's recorded samples buffer.
    recorder->setRecorderBuffer(&myRecordedSamples);

    MidiBuffer renderMidiBuffer;

    for (long long int i = 0; i < numberOfBuffers; ++i)
    {
        // This gets RecorderProcessor to write to this RenderEngine's myRecordedSamples.
        myMainProcessorGraph->processBlock(audioBuffer, renderMidiBuffer);
    }
}

const std::vector<std::vector<float>>
RenderEngine::getAudioFrames()
{
    return myRecordedSamples;
}

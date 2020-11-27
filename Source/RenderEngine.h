#pragma once

#include <random>
#include <array>
#include <iomanip>
#include <sstream>
#include <string>

#include "AllProcessors.h"

using namespace juce;

class DAGNode {
public:
    ProcessorBase* processorBase;
    std::vector<std::string> inputs;
};

class DAG {
public:
    std::vector<DAGNode> nodes;
};

class RenderEngine
{
public:
    RenderEngine(double sr, int bs);
    ~RenderEngine();
    // RenderEngine(const RenderEngine&) = delete;

    bool loadGraph(DAG dagNodes, int numInputAudioChans, int numOutputAudioChans);
    
    int hello () {
        DBG("hello");
        return 1;
    }

    void render (const double renderLength);

    const std::vector<std::vector<float>> getAudioFrames();

protected:

    double mySampleRate;
    int myBufferSize;

private:
                           
    std::vector<std::vector<float>> myRecordedSamples;

    std::unique_ptr<juce::AudioProcessorGraph> myMainProcessorGraph;

    juce::AudioProcessorGraph::Node::Ptr myMidiInputNode;

    int myNumInputAudioChans = 2;
    int myNumOutputAudioChans = 2;

};

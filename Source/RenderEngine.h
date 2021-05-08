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

class RenderEngine : AudioPlayHead
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

    void setBPM(double bpm);

    const std::vector<std::vector<float>> getAudioFrames();

    bool getCurrentPosition(CurrentPositionInfo& result) override;
    bool canControlTransport() override;
    void transportPlay(bool shouldStartPlaying) override;
    void transportRecord(bool shouldStartRecording) override;
    void transportRewind() override;

protected:

    double mySampleRate;
    int myBufferSize;
    double myBPM = 120.;

private:
                           
    std::vector<std::vector<float>> myRecordedSamples;

    std::unique_ptr<juce::AudioProcessorGraph> myMainProcessorGraph;

    juce::AudioProcessorGraph::Node::Ptr myMidiInputNode;

    int myNumInputAudioChans = 2;
    int myNumOutputAudioChans = 2;

    CurrentPositionInfo myCurrentPositionInfo;

};

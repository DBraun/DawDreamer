#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "custom_pybind_wrappers.h"
#include "CustomParameters.h"

#include "ProcessorBase.h"

#include <random>
#include <array>
#include <iomanip>
#include <sstream>
#include <string>

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

    bool loadGraph(DAG dagNodes);

    bool removeProcessor(const std::string& name);
    
    bool render(const double renderLength, bool convertToSeconds);

    uint64_t getRenderLength(const double renderLength, bool convertToSeconds);

    void setBPM(double bpm);

    bool setBPMwithPPQN(py::array_t<float> input, std::uint32_t ppqn);

    py::array_t<float> getAudioFrames();

    py::array_t<float> getAudioFramesForName(std::string& name);

    bool getCurrentPosition(CurrentPositionInfo& result) override;
    bool canControlTransport() override;
    void transportPlay(bool shouldStartPlaying) override;
    void transportRecord(bool shouldStartRecording) override;
    void transportRewind() override;

protected:

    double mySampleRate;
    int myBufferSize;
    std::unordered_map<std::string, juce::AudioProcessorGraph::NodeID> m_UniqueNameToNodeID;
    
    bool connectGraph();

    std::unique_ptr<juce::AudioProcessorGraph> myMainProcessorGraph;

    std::vector<std::pair<std::string, std::vector<std::string>>> m_stringDag;

private:

    CurrentPositionInfo myCurrentPositionInfo;
    AudioSampleBuffer bpmAutomation;
    std::uint32_t myBPMPPQN = 960;

    float getBPM(double ppqPosition);
};

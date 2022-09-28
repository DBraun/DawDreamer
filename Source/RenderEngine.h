#pragma once

#include <array>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

#include "../JuceLibraryCode/JuceHeader.h"
#include "AddProcessor.h"
#include "CompressorProcessor.h"
#include "CustomParameters.h"
#include "DelayProcessor.h"
#include "FaustProcessor.h"
#include "FilterProcessor.h"
#include "OscillatorProcessor.h"
#include "PannerProcessor.h"
#include "PlaybackProcessor.h"
#include "PlaybackWarpProcessor.h"
#include "PluginProcessor.h"
#include "ProcessorBase.h"
#include "ReverbProcessor.h"
#include "SamplerProcessor.h"
#include "custom_pybind_wrappers.h"

class DAGNode {
 public:
  ProcessorBase* processorBase;
  std::vector<std::string> inputs;
};

class DAG {
 public:
  std::vector<DAGNode> nodes;
};

class RenderEngine : public AudioPlayHead {
 public:
  RenderEngine(double sr, int bs);

  bool loadGraph(DAG dagNodes);

  bool removeProcessor(const std::string& name);

  bool render(const double renderLength, bool isBeats);

  int64_t getRenderLength(const double renderLength, bool isBeats);

  void setBPM(double bpm);

  bool setBPMwithPPQN(py::array_t<float> input, std::uint32_t ppqn);

  py::array_t<float> getAudioFrames();

  py::array_t<float> getAudioFramesForName(std::string& name);

  juce::Optional<PositionInfo> getPosition() const override;
  bool canControlTransport() override;
  void transportPlay(bool shouldStartPlaying) override;
  void transportRecord(bool shouldStartRecording) override;
  void transportRewind() override;

  // pybind11-related public:
  OscillatorProcessor* makeOscillatorProcessor(const std::string& name,
                                               float freq);

  PluginProcessorWrapper* makePluginProcessor(const std::string& name,
                                              const std::string& path);

  PlaybackProcessor* makePlaybackProcessor(const std::string& name,
                                           py::array input);

#ifdef BUILD_DAWDREAMER_RUBBERBAND
  PlaybackWarpProcessor* makePlaybackWarpProcessor(const std::string& name,
                                                   py::array input, double sr);
#endif

  FilterProcessor* makeFilterProcessor(const std::string& name,
                                       const std::string& mode, float freq,
                                       float q, float gain);

  CompressorProcessor* makeCompressorProcessor(const std::string& name,
                                               float threshold, float ratio,
                                               float attack, float release);

  AddProcessor* makeAddProcessor(const std::string& name,
                                 std::vector<float> gainLevels);

  ReverbProcessor* makeReverbProcessor(const std::string& name, float roomSize,
                                       float damping, float wetLevel,
                                       float dryLevel, float width);

  PannerProcessor* makePannerProcessor(const std::string& name,
                                       std::string& rule, float panVal);

  DelayProcessor* makeDelayProcessor(const std::string& name, std::string& rule,
                                     float delay, float wet);

  SamplerProcessor* makeSamplerProcessor(const std::string& name,
                                         py::array input);

#ifdef BUILD_DAWDREAMER_FAUST
  FaustProcessor* makeFaustProcessor(const std::string& name);
#endif

  bool loadGraphWrapper(py::object dagObj);

 protected:
  double mySampleRate;
  int myBufferSize;
  std::unordered_map<std::string, juce::AudioProcessorGraph::NodeID>
      m_UniqueNameToNodeID;

  bool connectGraph();

  std::unique_ptr<juce::AudioProcessorGraph> m_mainProcessorGraph;

  std::vector<std::pair<std::string, std::vector<std::string>>> m_stringDag;
  std::vector<ProcessorBase*> m_connectedProcessors;

  PositionInfo m_positionInfo;
  AudioSampleBuffer m_bpmAutomation;
  std::uint32_t m_BPM_PPQN = 960;

  float getBPM(double ppqPosition);

  void prepareProcessor(ProcessorBase* processor, const std::string& name);
};

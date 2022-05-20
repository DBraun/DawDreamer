#pragma once

#include "RenderEngine.h"
#include "custom_pybind_wrappers.h"

#include "ProcessorBase.h"
#include "AddProcessor.h"
#include "CompressorProcessor.h"
#include "DelayProcessor.h"
#include "FaustProcessor.h"
#include "FilterProcessor.h"
#include "OscillatorProcessor.h"
#include "PlaybackProcessor.h"
#include "PlaybackWarpProcessor.h"
#include "PluginProcessor.h"
#include "ReverbProcessor.h"
#include "PannerProcessor.h"
#include "SamplerProcessor.h"

class RenderEngineWrapper : public RenderEngine
{
public:

    RenderEngineWrapper(double sr, int bs);

    OscillatorProcessor* makeOscillatorProcessor(const std::string& name, float freq);

    PluginProcessorWrapper* makePluginProcessor(const std::string& name, const std::string& path);

    PlaybackProcessor* makePlaybackProcessor(const std::string& name, py::array input);

#ifdef BUILD_DAWDREAMER_RUBBERBAND
    PlaybackWarpProcessor* makePlaybackWarpProcessor(const std::string& name, py::array input, double sr);
#endif

    FilterProcessor* makeFilterProcessor(const std::string& name, const std::string& mode, float freq, float q, float gain);

    CompressorProcessor* makeCompressorProcessor(const std::string& name, float threshold, float ratio, float attack, float release);

    AddProcessor* makeAddProcessor(const std::string& name, std::vector<float> gainLevels);

    ReverbProcessor* makeReverbProcessor(const std::string& name, float roomSize, float damping, float wetLevel, float dryLevel, float width);

    PannerProcessor* makePannerProcessor(const std::string& name, std::string& rule, float panVal);

    DelayProcessor* makeDelayProcessor(const std::string& name, std::string& rule, float delay, float wet);

    SamplerProcessor* makeSamplerProcessor(const std::string& name, py::array input);

#ifdef BUILD_DAWDREAMER_FAUST
    FaustProcessor* makeFaustProcessor(const std::string& name);
#endif

    bool loadGraphWrapper(py::object dagObj);

private:
    void prepareProcessor(ProcessorBase* processor, const std::string& name);

};

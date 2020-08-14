#pragma once

#include "RenderEngine.h"
#include "custom_pybind_wrappers.h"

class RenderEngineWrapper : public RenderEngine
{
public:

    RenderEngineWrapper(double sr, int bs);
    //RenderEngineWrapper(const RenderEngineWrapper&) = delete;

    py::list wrapperGetAudioFrames();

    /// @brief
    std::shared_ptr<OscillatorProcessor> makeOscillatorProcessor(const std::string& name, float freq);

    /// @brief
    std::shared_ptr<PluginProcessorWrapper> makePluginProcessor(const std::string& name, const std::string& path);

    /// @brief
    std::shared_ptr<PlaybackProcessor> makePlaybackProcessor(const std::string& name, py::array input);

    /// @brief
    std::shared_ptr<FilterProcessor> makeFilterProcessor(const std::string& name, const std::string& mode, float freq, float q, float gain);

    /// @brief
    std::shared_ptr<CompressorProcessor> makeCompressorProcessor(const std::string& name, float threshold, float ratio, float attack, float release);

    /// @brief
    std::shared_ptr<AddProcessor> makeAddProcessor(const std::string& name, std::vector<float> gainLevels);

    /// @brief
    std::shared_ptr<ReverbProcessor> makeReverbProcessor(const std::string& name);

    bool loadGraphWrapper(py::object dagObj, int numInputAudioChans, int numOutputAudioChans);

};
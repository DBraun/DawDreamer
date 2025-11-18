#pragma once

#include <array>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

#include "../JuceLibraryCode/JuceHeader.h"
#include "AddProcessor.h"
#include "CompressorProcessor.h"
#include "custom_nanobind_wrappers.h"
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

class DAGNode
{
  public:
    ProcessorBase* processorBase;
    std::vector<std::string> inputs;
};

class DAG
{
  public:
    std::vector<DAGNode> nodes;
};

class RenderEngine : public AudioPlayHead
{
  public:
    RenderEngine(double sr, int bs);

    bool loadGraph(DAG dagNodes);

    bool removeProcessor(const std::string& name);

    bool render(const double renderLength, bool isBeats);

    int64_t getRenderLength(const double renderLength, bool isBeats);

    void setBPM(double bpm);

    bool setBPMwithPPQN(nb::ndarray<nb::numpy, float> input, std::uint32_t ppqn);

    nb::ndarray<nb::numpy, float> getAudioFrames();

    nb::ndarray<nb::numpy, float> getAudioFramesForName(std::string& name);

    juce::Optional<PositionInfo> getPosition() const override;
    bool canControlTransport() override;
    void transportPlay(bool shouldStartPlaying) override;
    void transportRecord(bool shouldStartRecording) override;
    void transportRewind() override;

    // pybind11-related public:
    OscillatorProcessor* makeOscillatorProcessor(const std::string& name, float freq);

    PluginProcessorWrapper* makePluginProcessor(const std::string& name, const std::string& path);

    PlaybackProcessor* makePlaybackProcessor(const std::string& name, nb::ndarray<float> input);

#ifdef BUILD_DAWDREAMER_RUBBERBAND
    PlaybackWarpProcessor* makePlaybackWarpProcessor(const std::string& name,
                                                     nb::ndarray<float> input, double sr);
#endif

    FilterProcessor* makeFilterProcessor(const std::string& name, const std::string& mode,
                                         float freq, float q, float gain);

    CompressorProcessor* makeCompressorProcessor(const std::string& name, float threshold,
                                                 float ratio, float attack, float release);

    AddProcessor* makeAddProcessor(const std::string& name, std::vector<float> gainLevels);

    ReverbProcessor* makeReverbProcessor(const std::string& name, float roomSize, float damping,
                                         float wetLevel, float dryLevel, float width);

    PannerProcessor* makePannerProcessor(const std::string& name, std::string& rule, float panVal);

    DelayProcessor* makeDelayProcessor(const std::string& name, std::string& rule, float delay,
                                       float wet);

    SamplerProcessor* makeSamplerProcessor(const std::string& name, nb::ndarray<float> input);

#ifdef BUILD_DAWDREAMER_FAUST
    FaustProcessor* makeFaustProcessor(const std::string& name);
#endif

    nb::dict getPickleState()
    {
        nb::dict state;
        state["sample_rate"] = mySampleRate;
        state["buffer_size"] = myBufferSize;
        state["ppqn"] = m_BPM_PPQN;

        // BPM automation
        if (m_bpmAutomation.getNumSamples() > 0)
        {
            juce::AudioSampleBuffer bpm_copy(m_bpmAutomation);
            // Note: bufferToPyArray is a protected method in ProcessorBase,
            // so we need to convert manually
            size_t num_samples = m_bpmAutomation.getNumSamples();
            size_t shape[1] = {num_samples};
            float* array_data = new float[num_samples];
            const float* src = m_bpmAutomation.getReadPointer(0);
            std::memcpy(array_data, src, num_samples * sizeof(float));
            auto capsule =
                nb::capsule(array_data, [](void* p) noexcept { delete[] static_cast<float*>(p); });
            state["bpm_automation"] = nb::ndarray<nb::numpy, float>(array_data, 1, shape, capsule);
        }

        // Save processors with their types and states
        nb::list processors_list;
        for (auto* proc : m_connectedProcessors)
        {
            nb::dict proc_dict;

#ifdef BUILD_DAWDREAMER_FAUST
            if (auto* faust_proc = dynamic_cast<FaustProcessor*>(proc))
            {
                proc_dict["type"] = "FaustProcessor";
                proc_dict["state"] = faust_proc->getPickleState();
                processors_list.append(proc_dict);
                continue;
            }
#endif

            if (auto* playback_proc = dynamic_cast<PlaybackProcessor*>(proc))
            {
                proc_dict["type"] = "PlaybackProcessor";
                proc_dict["state"] = playback_proc->getPickleState();
                processors_list.append(proc_dict);
                continue;
            }

            // If we get here, the processor type is not supported for pickling
            // We'll skip it rather than error
        }
        state["processors"] = processors_list;

        // Save DAG structure
        nb::list dag_list;
        for (const auto& [proc_name, inputs] : m_stringDag)
        {
            nb::dict dag_node;
            dag_node["processor_name"] = proc_name;
            nb::list inputs_list;
            for (const auto& input : inputs)
                inputs_list.append(input);
            dag_node["inputs"] = inputs_list;
            dag_list.append(dag_node);
        }
        state["dag"] = dag_list;

        return state;
    }

    void setPickleState(nb::dict state)
    {
        double sample_rate = nb::cast<double>(state["sample_rate"]);
        int buffer_size = nb::cast<int>(state["buffer_size"]);

        // Use placement new to construct the RenderEngine in-place
        new (this) RenderEngine(sample_rate, buffer_size);

        // Restore PPQN
        if (state.contains("ppqn"))
        {
            m_BPM_PPQN = nb::cast<std::uint32_t>(state["ppqn"]);
        }

        // Restore BPM automation
        if (state.contains("bpm_automation"))
        {
            nb::ndarray<nb::numpy, float> bpm_data =
                nb::cast<nb::ndarray<nb::numpy, float>>(state["bpm_automation"]);
            setBPMwithPPQN(bpm_data, m_BPM_PPQN);
        }

        // Recreate processors from their states
        std::map<std::string, ProcessorBase*> name_to_processor;
        std::map<std::string, nb::dict> processor_params_to_restore;

        if (state.contains("processors"))
        {
            nb::list processors_list = nb::cast<nb::list>(state["processors"]);

            for (nb::handle proc_handle : processors_list)
            {
                nb::dict proc_dict = nb::cast<nb::dict>(proc_handle);
                std::string proc_type = nb::cast<std::string>(proc_dict["type"]);
                nb::dict proc_state = nb::cast<nb::dict>(proc_dict["state"]);

                ProcessorBase* new_proc = nullptr;

#ifdef BUILD_DAWDREAMER_FAUST
                if (proc_type == "FaustProcessor")
                {
                    // Create FaustProcessor and manually restore its state
                    std::string proc_name = nb::cast<std::string>(proc_state["unique_name"]);
                    double proc_sr = nb::cast<double>(proc_state["sample_rate"]);

                    FaustProcessor* faust_proc = makeFaustProcessor(proc_name);

                    // Set Faust code using public method
                    std::string faust_code = nb::cast<std::string>(proc_state["faust_code"]);
                    faust_proc->setDSPString(faust_code);
                    faust_proc->setAutoImport(nb::cast<std::string>(proc_state["auto_import"]));

                    // Set library paths
                    if (proc_state.contains("library_paths"))
                    {
                        nb::list lib_paths = nb::cast<nb::list>(proc_state["library_paths"]);
                        std::vector<std::string> paths;
                        for (nb::handle path : lib_paths)
                            paths.push_back(nb::cast<std::string>(path));
                        faust_proc->setFaustLibrariesPaths(paths);
                    }

                    // Set asset paths
                    if (proc_state.contains("asset_paths"))
                    {
                        nb::list asset_paths = nb::cast<nb::list>(proc_state["asset_paths"]);
                        std::vector<std::string> paths;
                        for (nb::handle path : asset_paths)
                            paths.push_back(nb::cast<std::string>(path));
                        faust_proc->setFaustAssetsPaths(paths);
                    }

                    // Set compile flags
                    if (proc_state.contains("compile_flags"))
                    {
                        nb::list compile_flags = nb::cast<nb::list>(proc_state["compile_flags"]);
                        std::vector<std::string> flags;
                        for (nb::handle flag : compile_flags)
                            flags.push_back(nb::cast<std::string>(flag));
                        faust_proc->setCompileFlags(flags);
                    }

                    // Set polyphony settings
                    faust_proc->setNumVoices(nb::cast<int>(proc_state["num_voices"]));
                    faust_proc->setGroupVoices(nb::cast<bool>(proc_state["group_voices"]));
                    faust_proc->setDynamicVoices(nb::cast<bool>(proc_state["dynamic_voices"]));
                    faust_proc->setLLVMOpt(nb::cast<int>(proc_state["llvm_opt_level"]));
                    faust_proc->setReleaseLength(nb::cast<double>(proc_state["release_length"]));

                    // Compile the Faust code
                    faust_proc->compile();

                    // Restore soundfiles
                    if (proc_state.contains("soundfiles"))
                    {
                        faust_proc->setSoundfiles(nb::cast<nb::dict>(proc_state["soundfiles"]));
                    }

                    // Save parameters for later restoration (after graph is loaded)
                    if (proc_state.contains("parameters"))
                    {
                        processor_params_to_restore[proc_name] =
                            nb::cast<nb::dict>(proc_state["parameters"]);
                    }

                    new_proc = faust_proc;
                }
                else
#endif
                    if (proc_type == "PlaybackProcessor")
                {
                    // For PlaybackProcessor, we can create it directly from state
                    std::string name = nb::cast<std::string>(proc_state["unique_name"]);
                    nb::ndarray<float> audio_data =
                        nb::cast<nb::ndarray<float>>(proc_state["audio_data"]);
                    new_proc = makePlaybackProcessor(name, audio_data);
                }

                if (new_proc)
                {
                    std::string proc_name = nb::cast<std::string>(proc_state["unique_name"]);
                    name_to_processor[proc_name] = new_proc;
                }
            }
        }

        // Rebuild the graph
        if (state.contains("dag"))
        {
            nb::list dag_list = nb::cast<nb::list>(state["dag"]);
            nb::list graph_arg;

            for (nb::handle dag_node_handle : dag_list)
            {
                nb::dict dag_node = nb::cast<nb::dict>(dag_node_handle);
                std::string proc_name = nb::cast<std::string>(dag_node["processor_name"]);
                nb::list inputs_list = nb::cast<nb::list>(dag_node["inputs"]);

                auto it = name_to_processor.find(proc_name);
                if (it != name_to_processor.end())
                {
                    ProcessorBase* proc = it->second;
                    nb::tuple graph_item = nb::make_tuple(nb::cast(proc), inputs_list);
                    graph_arg.append(graph_item);
                }
            }

            if (nb::len(graph_arg) > 0)
            {
                loadGraphWrapper(graph_arg);
                // Call connectGraph to prepare all processors
                // This ensures prepareToPlay is called before we restore parameters
                connectGraph();
            }
        }

        // Now restore parameters AFTER the graph has been loaded and processors prepared
        // This ensures compile() has been called by prepareToPlay, so parameters won't be reset
        for (const auto& [proc_name, params] : processor_params_to_restore)
        {
            auto it = name_to_processor.find(proc_name);
            if (it != name_to_processor.end())
            {
                ProcessorBase* proc = it->second;
                for (auto item : params)
                {
                    std::string param_name = nb::cast<std::string>(item.first);
                    float param_value = nb::cast<float>(item.second);
                    proc->setAutomationVal(param_name.c_str(), param_value);
                }
            }
        }
    }

    bool loadGraphWrapper(nb::object dagObj);

  protected:
    double mySampleRate;
    int myBufferSize;
    std::unordered_map<std::string, juce::AudioProcessorGraph::NodeID> m_UniqueNameToNodeID;

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

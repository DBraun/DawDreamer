#include "RenderEngineWrapper.h"


PYBIND11_MODULE(dawdreamer, m)
{
    using arg = py::arg;

    py::class_<ProcessorBase, std::shared_ptr<ProcessorBase>>(m, "ProcessorBase");

    py::class_<OscillatorProcessor, std::shared_ptr<OscillatorProcessor>>(m, "OscillatorProcessor", py::base<ProcessorBase>());
    
    py::class_<PlaybackProcessor, std::shared_ptr<PlaybackProcessor>>(m, "PlaybackProcessor", py::base<ProcessorBase>())
        .def("set_data", &PlaybackProcessor::setData);

    py::class_<PannerProcessor, std::shared_ptr<PannerProcessor>>(m, "PannerProcessor", py::base<ProcessorBase>())
        .def_property("rule", &PannerProcessor::getRule, &PannerProcessor::setRule)
        .def_property("pan", &PannerProcessor::getPan, &PannerProcessor::setPan);
    
    py::class_<CompressorProcessor, std::shared_ptr<CompressorProcessor>>(m, "CompressorProcessor", py::base<ProcessorBase>())
        .def_property("threshold", &CompressorProcessor::getThreshold, &CompressorProcessor::setThreshold)
        .def_property("ratio", &CompressorProcessor::getRatio, &CompressorProcessor::setRatio)
        .def_property("attack", &CompressorProcessor::getAttack, &CompressorProcessor::setAttack)
        .def_property("release", &CompressorProcessor::getRelease, &CompressorProcessor::setRelease);
    
    py::class_<FilterProcessor, std::shared_ptr<FilterProcessor>>(m, "FilterProcessor", py::base<ProcessorBase>())
        .def_property("mode", &FilterProcessor::getMode, &FilterProcessor::setMode)
        .def_property("frequency", &FilterProcessor::getFrequency, &FilterProcessor::setFrequency)
        .def_property("q", &FilterProcessor::getQ, &FilterProcessor::setQ)
        .def_property("gain", &FilterProcessor::getGain, &FilterProcessor::setGain);
    
    py::class_<ReverbProcessor, std::shared_ptr<ReverbProcessor>>(m, "ReverbProcessor", py::base<ProcessorBase>())
        .def_property("room_size", &ReverbProcessor::getRoomSize, &ReverbProcessor::setRoomSize)
        .def_property("damping", &ReverbProcessor::getDamping, &ReverbProcessor::setDamping)
        .def_property("wet_level", &ReverbProcessor::getWetLevel, &ReverbProcessor::setWetLevel)
        .def_property("dry_level", &ReverbProcessor::getDryLevel, &ReverbProcessor::setDryLevel)
        .def_property("width", &ReverbProcessor::getWidth, &ReverbProcessor::setWidth);
    
    py::class_<AddProcessor, std::shared_ptr<AddProcessor>>(m, "AddProcessor", py::base<ProcessorBase>())
        .def_property("gain_levels", &AddProcessor::getGainLevels, &AddProcessor::setGainLevels);

    py::class_<PluginProcessorWrapper, std::shared_ptr<PluginProcessorWrapper>>(m, "PluginProcessor", py::base<ProcessorBase>())
        .def("load_preset", &PluginProcessorWrapper::loadPreset)
        .def("get_patch", &PluginProcessorWrapper::wrapperGetPatch)
        .def("set_patch", &PluginProcessorWrapper::wrapperSetPatch)
        .def("get_parameter", &PluginProcessorWrapper::wrapperGetParameter)
        .def("get_parameter_text", &PluginProcessorWrapper::getParameterAsText)
        .def("set_parameter", &PluginProcessorWrapper::wrapperSetParameter)
        .def("get_plugin_parameter_size", &PluginProcessorWrapper::wrapperGetPluginParameterSize)
        .def("get_plugin_parameters_description", &PluginProcessorWrapper::getPluginParametersDescription)
        .def("override_plugin_parameter", &PluginProcessorWrapper::overridePluginParameter)
        .def("remove_overriden_plugin_parameter", &PluginProcessorWrapper::removeOverridenParameter)
        .def_property_readonly("n_midi_events", &PluginProcessorWrapper::getNumMidiEvents)
        .def("load_midi", &PluginProcessorWrapper::loadMidi)
        .def("clear_midi", &PluginProcessorWrapper::clearMidi)
        .def("add_midi_note", &PluginProcessorWrapper::addMidiNote);

    std::vector<float> defaultGain;

    py::return_value_policy returnPolicy = py::return_value_policy::take_ownership;

    py::class_<RenderEngineWrapper>(m, "RenderEngine")
        .def(py::init<double, int>())
        .def("hello", &RenderEngineWrapper::hello)
        .def("render", &RenderEngineWrapper::render)
        .def("get_audio", &RenderEngineWrapper::wrapperGetAudioFrames)
        .def("load_graph", &RenderEngineWrapper::loadGraphWrapper, arg("dagObj"), arg("numInputAudioChans") = 2, arg("numOutAudioChans") = 2)
        .def("make_oscillator_processor", &RenderEngineWrapper::makeOscillatorProcessor, returnPolicy)
        .def("make_plugin_processor", &RenderEngineWrapper::makePluginProcessor, returnPolicy)
        .def("make_playback_processor", &RenderEngineWrapper::makePlaybackProcessor, returnPolicy)
        .def("make_filter_processor", &RenderEngineWrapper::makeFilterProcessor, returnPolicy,
            arg("name"), arg("mode")="high", arg("freq")=1000.f, arg("q")=.707107f, arg("gain")=1.f)
        .def("make_reverb_processor", &RenderEngineWrapper::makeReverbProcessor, returnPolicy,
            arg("name"), arg("roomSize")= 0.5f, arg("damping")= 0.5f, arg("wetLevel") = 0.33f,
            arg("dryLevel")= 0.4f, arg("width") = 1.0f)
        .def("make_add_processor", &RenderEngineWrapper::makeAddProcessor, returnPolicy,
            arg("name"), arg("gainLevels") = defaultGain)
        .def("make_panner_processor", &RenderEngineWrapper::makePannerProcessor, returnPolicy,
            arg("name"), arg("rule") = "linear", arg("pan")=0.f)
        .def("make_compressor_processor", &RenderEngineWrapper::makeCompressorProcessor, returnPolicy,
            arg("name"), arg("threshold")=0.f, arg("ratio")=2.f, arg("attack")=2.0f, arg("release")=50.f);
}

#include "RenderEngineWrapper.h"


PYBIND11_MODULE(dawdreamer, m)
{

    py::class_<ProcessorBase, std::shared_ptr<ProcessorBase>>(m, "ProcessorBase");

    py::class_<OscillatorProcessor, std::shared_ptr<OscillatorProcessor>>(m, "OscillatorProcessor", py::base<ProcessorBase>());
    py::class_<PlaybackProcessor, std::shared_ptr<PlaybackProcessor>>(m, "PlaybackProcessor", py::base<ProcessorBase>());
    py::class_<CompressorProcessor, std::shared_ptr<CompressorProcessor>>(m, "CompressorProcessor", py::base<ProcessorBase>());
    py::class_<FilterProcessor, std::shared_ptr<FilterProcessor>>(m, "FilterProcessor", py::base<ProcessorBase>());
    py::class_<ReverbProcessor, std::shared_ptr<ReverbProcessor>>(m, "ReverbProcessor", py::base<ProcessorBase>());
    py::class_<AddProcessor, std::shared_ptr<AddProcessor>>(m, "AddProcessor", py::base<ProcessorBase>());

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

    py::class_<RenderEngineWrapper>(m, "RenderEngine")
        .def(py::init<double, int>())
        .def("hello", &RenderEngineWrapper::hello)
        .def("render", &RenderEngineWrapper::render)
        .def("get_audio", &RenderEngineWrapper::wrapperGetAudioFrames)
        .def("load_graph", &RenderEngineWrapper::loadGraphWrapper, py::arg("dagObj"), py::arg("numInputAudioChans") = 2, py::arg("numOutAudioChans") = 2)
        .def("make_oscillator_processor", &RenderEngineWrapper::makeOscillatorProcessor, py::return_value_policy::take_ownership)
        .def("make_plugin_processor", &RenderEngineWrapper::makePluginProcessor, py::return_value_policy::take_ownership)
        .def("make_playback_processor", &RenderEngineWrapper::makePlaybackProcessor, py::return_value_policy::take_ownership)
        .def("make_filter_processor", &RenderEngineWrapper::makeFilterProcessor, py::return_value_policy::take_ownership)
        .def("make_reverb_processor", &RenderEngineWrapper::makeReverbProcessor, py::return_value_policy::take_ownership)
        .def("make_add_processor", &RenderEngineWrapper::makeAddProcessor, py::return_value_policy::take_ownership,
            py::arg("name"), py::arg("gainLevels") = defaultGain)
        .def("make_compressor_processor", &RenderEngineWrapper::makeCompressorProcessor, py::return_value_policy::take_ownership,
            py::arg("name"), py::arg("threshold")=0.f, py::arg("ratio")=2.f, py::arg("attack")=2.0f, py::arg("release")=50.f);
}

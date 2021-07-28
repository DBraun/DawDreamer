#include "RenderEngineWrapper.h"

PYBIND11_MODULE(dawdreamer, m)
{
    using arg = py::arg;

    m.doc() = R"pbdoc(
        dawdreamer
        -----------------------
    
        .. currentmodule:: dawdreamer
      
        .. autosummary::
           :toctree: _generate
      
           RenderEngine
           ProcessorBase
           AddProcessor
           CompressorProcessor
           DelayProcessor
           FaustProcessor
           FilterProcessor
           PannerProcessor
           PlaybackProcessor
           PlaybackWarpProcessor
           PluginProcessor
           ReverbProcessor
           SamplerProcessor
    )pbdoc";

    py::class_<ProcessorBase, std::shared_ptr<ProcessorBase>>(m, "ProcessorBase")
        .def("set_automation", &ProcessorBase::setAutomation, arg("parameter_name"), arg("data"), R"pbdoc(
    Set a parameter's automation with a numpy array.

    Parameters
    ----------
    parameter_name : str
        The name of the parameter.
    data : str
        An array of data for the parameter automation.

    Returns
    -------
    None

    """
    ----------
)pbdoc")
        .def("get_automation", &ProcessorBase::getAutomationNumpy, arg("parameter_name"), R"pbdoc(
    Get a parameter's automation as a numpy array.

    Parameters
    ----------
    parameter_name : str
        The name of the parameter.

    Returns
    -------
    np.array
        The parameter's automation.

    """
    ----------

)pbdoc")
        .def_property("record", &ProcessorBase::getRecordEnable, &ProcessorBase::setRecordEnable, "Whether recording of this processor is enabled." )
        .def("get_audio", &ProcessorBase::getAudioFrames, "Get the audio data of the processor after a render, assuming recording was enabled.")
        .def("get_name", &ProcessorBase::getUniqueName, "Get the user-defined name of a processor instance.").doc() = R"pbdoc(
    The abstract Processor Base class, which all processors subclass.
)pbdoc";

    py::class_<OscillatorProcessor, std::shared_ptr<OscillatorProcessor>, ProcessorBase>(m, "OscillatorProcessor");

    py::class_<PlaybackProcessor, std::shared_ptr<PlaybackProcessor>, ProcessorBase>(m, "PlaybackProcessor")
        .def("set_data", &PlaybackProcessor::setData, arg("data"), "Set the audio as a 2xN numpy array.")
        .doc() = "The Playback Processor can play audio data provided as an argument.";

#ifdef BUILD_DAWDREAMER_RUBBERBAND
    py::class_<PlaybackWarpProcessor, std::shared_ptr<PlaybackWarpProcessor>, ProcessorBase>(m, "PlaybackWarpProcessor")
        .def("set_time_ratio", &PlaybackWarpProcessor::setTimeRatio, arg("time_ratio"), 
            "Set the time ratio, which will have an effect if an Ableton ASD file hasn't been loaded. A value of 2.0 for the time ratio will \
play the audio in double the amount of time, so it will sound slowed down.")
        .def_property("transpose", &PlaybackWarpProcessor::getTranspose, &PlaybackWarpProcessor::setTranspose, "The pitch transposition in semitones")
        .def_property("warp_on", &PlaybackWarpProcessor::getWarpOn, &PlaybackWarpProcessor::setWarpOn, "Whether warping is enabled.")
        .def_property("loop_on", &PlaybackWarpProcessor::getLoopOn, &PlaybackWarpProcessor::setLoopOn, "Whether looping is enabled")
        .def_property("loop_start", &PlaybackWarpProcessor::getLoopStart, &PlaybackWarpProcessor::setLoopStart, "The loop start position in beats (typically quarter notes) relative to 1.1.1")
        .def_property("loop_end", &PlaybackWarpProcessor::getLoopEnd, &PlaybackWarpProcessor::setLoopEnd, "The loop end position in beats (typically quarter notes) relative to 1.1.1")
        .def_property("start_marker", &PlaybackWarpProcessor::getStartMarker, &PlaybackWarpProcessor::setStartMarker, "The start position in beats (typically quarter notes) relative to 1.1.1")
        .def_property("end_marker", &PlaybackWarpProcessor::getEndMarker, &PlaybackWarpProcessor::setEndMarker, "The end position in beats (typically quarter notes) relative to 1.1.1")
        .def("set_clip_file", &PlaybackWarpProcessor::loadAbletonClipInfo, arg("asd_file_path"), "Load an Ableton Live file with an \".asd\" extension")
        .def_property_readonly("warp_markers", &PlaybackWarpProcessor::getWarpMarkers, "Get the warp markers as a 2D array of time positions in seconds and positions in beats.")
        .def("set_data", &PlaybackWarpProcessor::setData, arg("data"), "Set the audio as a 2xN numpy array")
        .def("set_clip_positions", &PlaybackWarpProcessor::setClipPositions, arg("clip_positions"), R"pbdoc(
    Set one or more positions at which the clip should play.

    Parameters
    ----------

    clip_positions : list
        A list of length-3 lists. Each length-3 list consists of a global clip start time, a global clip start end time, and a local start offset.

    Returns
    -------
    None 
)pbdoc").doc() = "The Playback Warp Processor can play audio data while time-stretching and pitch-shifting it thanks to the Rubberband library \
(https://github.com/breakfastquay/rubberband). This processor can load Ableton Live \".asd\" files to do beat-matching.";
#endif

    py::class_<PannerProcessor, std::shared_ptr<PannerProcessor>, ProcessorBase>(m, "PannerProcessor")
        .def_property("rule", &PannerProcessor::getRule, &PannerProcessor::setRule,
            "The rule must be among \"linear\", \"balanced\", \"sin3dB\", \"sin4p5dB\", \"sin6dB\", \"squareRoot3dB\", \"squareRoot4p5dB.\"")
        .def_property("pan", &PannerProcessor::getPan, &PannerProcessor::setPan, "The pan value between -1.0 and 1.0.").doc() = "The Panner Processor class";

    py::class_<CompressorProcessor, std::shared_ptr<CompressorProcessor>, ProcessorBase>(m, "CompressorProcessor")
        .def_property("threshold", &CompressorProcessor::getThreshold, &CompressorProcessor::setThreshold,
            "The compressor's threshold in decibels.")
        .def_property("ratio", &CompressorProcessor::getRatio, &CompressorProcessor::setRatio,
            "The ratio of the compressor. It must be greater than or equal to 1.0.")
        .def_property("attack", &CompressorProcessor::getAttack, &CompressorProcessor::setAttack,
            "The compressor's attack in millisecods.")
        .def_property("release", &CompressorProcessor::getRelease, &CompressorProcessor::setRelease,
            "The compressor's release in millisecods.")
        .doc() = "A compressor from JUCE.";

    py::class_<DelayProcessor, std::shared_ptr<DelayProcessor>, ProcessorBase>(m, "DelayProcessor")
        .def_property("delay", &DelayProcessor::getDelay, &DelayProcessor::setDelay, "The delay in milliseconds.")
        .def_property("wet", &DelayProcessor::getWet, &DelayProcessor::setWet, "A wet level between 0.0 and 1.0.")
        .doc() = "A delay from JUCE.";

    py::class_<FilterProcessor, std::shared_ptr<FilterProcessor>, ProcessorBase>(m, "FilterProcessor")
        .def_property("mode", &FilterProcessor::getMode, &FilterProcessor::setMode,
            "Choose from \"low\", \"high\", \"band\", \"low_shelf\", \"high_shelf\", \"notch\"." )
        .def_property("frequency", &FilterProcessor::getFrequency, &FilterProcessor::setFrequency,
            "The frequency cutoff in Hz.")
        .def_property("q", &FilterProcessor::getQ, &FilterProcessor::setQ,
            "The Q-value. A safe choice is 1./rad(2)=0.707107." )
        .def_property("gain", &FilterProcessor::getGain, &FilterProcessor::setGain,
            "The gain parameter only matters when the mode is low_shelf or high_shelf. A value of 1.0 has no effect.")
        .doc() = "A Filter Processor applies one of several kinds of filters. The filter cutoff, Q-value and gain can be adjusted, \
but the filter mode cannot under automation.";

    py::class_<ReverbProcessor, std::shared_ptr<ReverbProcessor>, ProcessorBase>(m, "ReverbProcessor")
        .def_property("room_size", &ReverbProcessor::getRoomSize, &ReverbProcessor::setRoomSize, "The room size between 0.0 and 1.0.")
        .def_property("damping", &ReverbProcessor::getDamping, &ReverbProcessor::setDamping, "The damping amount between 0.0 and 1.0.")
        .def_property("wet_level", &ReverbProcessor::getWetLevel, &ReverbProcessor::setWetLevel, "A wet level between 0.0 and 1.0.")
        .def_property("dry_level", &ReverbProcessor::getDryLevel, &ReverbProcessor::setDryLevel, "A dry level between 0.0 and 1.0.")
        .def_property("width", &ReverbProcessor::getWidth, &ReverbProcessor::setWidth, "The stereo width from 0.0 to 1.0.")
        .doc() = "A Reverb Processor applies reverb with the FreeVerb algorithm.";

    py::class_<AddProcessor, std::shared_ptr<AddProcessor>, ProcessorBase>(m, "AddProcessor")
        .def_property("gain_levels", &AddProcessor::getGainLevels, &AddProcessor::setGainLevels,
            "A list of gain levels to apply to the corresponding inputs.")
        .doc() = "An Add Processor adds one or more stereo inputs with corresponding gain parameters.";

    py::class_<PluginProcessorWrapper, std::shared_ptr<PluginProcessorWrapper>, ProcessorBase>(m, "PluginProcessor")
        .def("load_preset", &PluginProcessorWrapper::loadPreset, arg("filepath"), "Load an FXP preset.")
        .def("load_vst3_preset", &PluginProcessorWrapper::loadVST3Preset, arg("filepath"), "Load a VST3 preset.")
        .def("get_patch", &PluginProcessorWrapper::wrapperGetPatch)
        .def("set_patch", &PluginProcessorWrapper::wrapperSetPatch, arg("patch"))
        .def("get_parameter", &PluginProcessorWrapper::wrapperGetParameter, arg("index"), "Get a parameter's value.")
        .def("get_parameter_name", &PluginProcessorWrapper::wrapperGetParameterName, arg("index"), "Get a parameter's name.")
        .def("get_parameter_text", &PluginProcessorWrapper::getParameterAsText, arg("index"), "Get a parameter's value as text.")
        .def("set_parameter", &PluginProcessorWrapper::wrapperSetParameter, arg("index"), arg("value"),
            "Set a parameter's value to a constant.")
        .def("get_plugin_parameter_size", &PluginProcessorWrapper::wrapperGetPluginParameterSize, "Get the number of parameters.")
        .def("get_plugin_parameters_description", &PluginProcessorWrapper::getPluginParametersDescription,
            "Get a list of dictionaries describing the plugin's parameters.")
        .def_property_readonly("n_midi_events", &PluginProcessorWrapper::getNumMidiEvents, "The number of MIDI events stored in the buffer. \
Note that note-ons and note-offs are counted separately.")
        .def("load_midi", &PluginProcessorWrapper::loadMidi, arg("filepath"), "Load MIDI from a file.")
        .def("clear_midi", &PluginProcessorWrapper::clearMidi, "Remove all MIDI notes.")
        .def("add_midi_note", &PluginProcessorWrapper::addMidiNote,
            arg("note"), arg("velocity"), arg("start_time"), arg("duration"),
            "Add a single MIDI note whose note and velocity are integers between 0 and 127.")
        .doc() = "A Plugin Processor can load VST \".dll\" files on Windows and \".vst\" files on macOS. The files can be for either instruments \
or effects. Some plugins such as ones that do sidechain compression can accept two inputs when loading a graph.";

    py::class_<SamplerProcessor, std::shared_ptr<SamplerProcessor>, ProcessorBase>(m, "SamplerProcessor")
        .def("set_data", &SamplerProcessor::setData, arg("data"), "Set an audio sample.")
        .def("get_parameter", &SamplerProcessor::wrapperGetParameter, arg("index"), "Get a parameter's value.")
        .def("get_parameter_name", &SamplerProcessor::wrapperGetParameterName, arg("index"), "Get a parameter's name.")
        .def("get_parameter_text", &SamplerProcessor::wrapperGetParameterAsText, arg("index"), "Get a parameter's value as text.")
        .def("set_parameter", &SamplerProcessor::wrapperSetParameter, arg("index"), arg("value"),
            "Set a parameter's value to a constant.")
        .def("get_parameter_size", &SamplerProcessor::wrapperGetPluginParameterSize, "Get the number of parameters.")
        .def("get_parameters_description", &SamplerProcessor::getParametersDescription,
            "Get a list of dictionaries describing the plugin's parameters.")
        .def_property_readonly("n_midi_events", &SamplerProcessor::getNumMidiEvents, "The number of MIDI events stored in the buffer. \
Note that note-ons and note-offs are counted separately.")
        .def("load_midi", &SamplerProcessor::loadMidi, arg("filepath"), "Load MIDI from a file.")
        .def("clear_midi", &SamplerProcessor::clearMidi, "Remove all MIDI notes.")
        .def("add_midi_note", &SamplerProcessor::addMidiNote,
            arg("note"), arg("velocity"), arg("start_time"), arg("duration"),
            "Add a single MIDI note whose note and velocity are integers between 0 and 127.")
        .doc() = "The Sampler Processor works like a basic Sampler instrument. It takes a typically short audio sample and can play it back \
at different pitches and speeds. It has parameters for an ADSR envelope controlling the amplitude and another for controlling a low-pass filter cutoff. \
Unlike a VST, the parameters don't need to be between 0 and 1. For example, you can set an envelope attack parameter to 50 to represent 50 milliseconds.";

#ifdef BUILD_DAWDREAMER_FAUST
    py::class_<FaustProcessor, std::shared_ptr<FaustProcessor>, ProcessorBase>(m, "FaustProcessor")
        .def("set_dsp", &FaustProcessor::compileFromFile, arg("filepath"), "Set the FAUST signal process with a file.")
        .def("set_dsp_string", &FaustProcessor::compileFromString, arg("faust_code"), 
            "Set the FAUST signal process with a string containing FAUST code.")
        .def("get_parameters_description", &FaustProcessor::getPluginParametersDescription,
            "Get a list of dictionaries describing the parameters of the most recently compiled FAUST code.")
        .def("get_parameter", &FaustProcessor::getParamWithIndex, arg("param_index"))
        .def("get_parameter", &FaustProcessor::getParamWithPath, arg("parameter_path"))
        .def("set_parameter", &FaustProcessor::setParamWithIndex, arg("parameter_index"), arg("value"))
        .def("set_parameter", &FaustProcessor::setAutomationVal, arg("parameter_path"), arg("value"))
        .def_property_readonly("compiled", &FaustProcessor::isCompiled, "Did the most recent DSP code compile?")
        .def_property_readonly("code", &FaustProcessor::code, "Get the most recently compiled Faust DSP code.")
        .def_property("num_voices", &FaustProcessor::getNumVoices, &FaustProcessor::setNumVoices, "The number of voices for polyphony. Set to zero to disable polyphony. One or more enables polyphony.")
        .def_property_readonly("n_midi_events", &FaustProcessor::getNumMidiEvents, "The number of MIDI events stored in the buffer. \
Note that note-ons and note-offs are counted separately.")
        .def("load_midi", &FaustProcessor::loadMidi, arg("filepath"), "Load MIDI from a file.")
        .def("clear_midi", &FaustProcessor::clearMidi, "Remove all MIDI notes.")
        .def("add_midi_note", &FaustProcessor::addMidiNote, arg("note"), arg("velocity"), arg("start_time"), arg("duration"),
    "Add a single MIDI note whose note and velocity are integers between 0 and 127.")
        .doc() = "A Faust Processor can compile and execute FAUST code. See https://faust.grame.fr for more information.";
#endif

    std::vector<float> defaultGain;

    py::return_value_policy returnPolicy = py::return_value_policy::take_ownership;

    py::class_<RenderEngineWrapper>(m, "RenderEngine", "A Render Engine loads and runs a graph of audio processors.")
        .def(py::init<double, int>(), arg("sample_rate"), arg("block_size"))
        .def("render", &RenderEngineWrapper::render, arg("seconds"), "Render the most recently loaded graph.")
        .def("set_bpm", &RenderEngineWrapper::setBPM, arg("bpm"), "Set the beats-per-minute of the engine.")
        .def("get_audio", &RenderEngine::getAudioFrames, "Get the most recently rendered audio as a numpy array.")
        .def("get_audio", &RenderEngine::getAudioFramesForName, arg("name"), "Get the most recently rendered audio for a specific processor.")
        .def("load_graph", &RenderEngineWrapper::loadGraphWrapper, arg("dag"), arg("num_input_audio_chans") = 2, arg("num_out_audio_chans") = 2,
            "Load a directed acyclic graph of processors.")
        .def("make_oscillator_processor", &RenderEngineWrapper::makeOscillatorProcessor, arg("name"), arg("frequency"),
            "Make an Oscillator Processor", returnPolicy)
        .def("make_plugin_processor", &RenderEngineWrapper::makePluginProcessor, arg("name"), arg("plugin_path"),
            "Make a Plugin Processor", returnPolicy)
        .def("make_sampler_processor", &RenderEngineWrapper::makeSamplerProcessor, arg("name"), arg("data"),
            "Make a Sampler Processor with audio data to be used as the sample.", returnPolicy)
#ifdef BUILD_DAWDREAMER_FAUST
        .def("make_faust_processor", &RenderEngineWrapper::makeFaustProcessor, arg("name"), arg("dsp_path"), "Make a FAUST Processor", returnPolicy)
#endif
        .def("make_playback_processor", &RenderEngineWrapper::makePlaybackProcessor, arg("name"), arg("data"), returnPolicy, "Make a Playback Processor")
#ifdef BUILD_DAWDREAMER_RUBBERBAND
        .def("make_playbackwarp_processor", &RenderEngineWrapper::makePlaybackWarpProcessor, arg("name"), arg("data"),
            "Make a Playback Processor that can do time-stretching and pitch-shifting.", returnPolicy)
#endif
        .def("make_filter_processor", &RenderEngineWrapper::makeFilterProcessor, returnPolicy,
            arg("name"), arg("mode") = "high", arg("freq") = 1000.f, arg("q") = .707107f, arg("gain") = 1.f, "Make a Filter Processor")
        .def("make_reverb_processor", &RenderEngineWrapper::makeReverbProcessor, returnPolicy,
            arg("name"), arg("roomSize") = 0.5f, arg("damping") = 0.5f, arg("wetLevel") = 0.33f,
            arg("dryLevel") = 0.4f, arg("width") = 1.0f, "Make a Reverb Processor")
        .def("make_add_processor", &RenderEngineWrapper::makeAddProcessor, returnPolicy,
            arg("name"), arg("gain_levels") = defaultGain, "Make an Add Processor with an optional list of gain levels")
        .def("make_delay_processor", &RenderEngineWrapper::makeDelayProcessor, returnPolicy,
            arg("name"), arg("rule") = "linear", arg("delay") = 10.f, arg("wet") = .1f, "Make a Delay Processor")
        .def("make_panner_processor", &RenderEngineWrapper::makePannerProcessor, returnPolicy,
            arg("name"), arg("rule") = "linear", arg("pan") = 0.f, "Make a Panner Processor")
        .def("make_compressor_processor", &RenderEngineWrapper::makeCompressorProcessor, returnPolicy,
            arg("name"), arg("threshold") = 0.f, arg("ratio") = 2.f, arg("attack") = 2.0f, arg("release") = 50.f, "Make a Compressor Processor");
}

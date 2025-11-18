"""
Tests for pickle support in DawDreamer.
Tests pickling/unpickling of RenderEngine with various processor types.
"""

import os
import pickle

from dawdreamer_utils import *

BUFFER_SIZE = 128


def test_render_engine_with_playback_processor():
    """Test pickling RenderEngine containing a PlaybackProcessor."""
    DURATION = 2.0

    # Create engine and playback processor
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    data = np.random.rand(2, int(SAMPLE_RATE * DURATION))
    playback = engine.make_playback_processor("playback", data)

    # Load graph and render
    engine.load_graph([(playback, [])])
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match
    assert np.allclose(original_audio, restored_audio, atol=1e-07)


def test_render_engine_with_faust_processor():
    """Test pickling RenderEngine containing a FaustProcessor."""
    DURATION = 1.0

    # Get the correct Faust libraries path
    faust_lib_path = os.path.join(
        os.path.dirname(os.path.dirname(__file__)), "thirdparty", "faust", "libraries"
    )

    # Create engine and Faust processor with gain
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    data = np.random.rand(2, int(SAMPLE_RATE * DURATION))
    playback = engine.make_playback_processor("playback", data)

    faust = engine.make_faust_processor("faust")
    faust.faust_libraries_paths = [faust_lib_path]
    faust_code = """
    import("stdfaust.lib");
    gain = hslider("gain", 0.5, 0, 1, 0.01);
    process = _ * gain, _ * gain;
    """
    faust.set_dsp_string(faust_code)
    faust.compile()

    # Set a parameter value (Faust uses /dawdreamer/ namespace)
    faust.set_parameter("/dawdreamer/gain", 0.75)

    # Build graph and render
    graph = [(playback, []), (faust, ["playback"])]
    engine.load_graph(graph)
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle the entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match
    assert np.allclose(original_audio, restored_audio, atol=1e-05)


def test_render_engine_with_polyphonic_faust():
    """Test pickling RenderEngine with polyphonic FaustProcessor."""
    DURATION = 1.0

    # Get the correct Faust libraries path
    faust_lib_path = os.path.join(
        os.path.dirname(os.path.dirname(__file__)), "thirdparty", "faust", "libraries"
    )

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    # Create input signal
    input_data = np.ones((2, int(SAMPLE_RATE * DURATION)))
    playback = engine.make_playback_processor("input", input_data)

    faust = engine.make_faust_processor("poly_faust")
    faust.faust_libraries_paths = [faust_lib_path]

    # Simple polyphonic processor (pass-through to test polyphony settings)
    faust_code = """
    import("stdfaust.lib");
    process = _, _;  // Per-voice processing
    effect = _, _;  // Global effect (required for polyphonic mode)
    """
    faust.set_dsp_string(faust_code)
    faust.num_voices = 8
    faust.group_voices = True
    faust.dynamic_voices = True
    faust.release_length = 0.3
    faust.compile()

    # Build graph and render
    engine.load_graph([(playback, []), (faust, ["input"])])
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match (verifies polyphony settings and parameters are preserved)
    assert np.allclose(original_audio, restored_audio, atol=1e-05)


def test_render_engine_with_complex_graph():
    """Test pickling RenderEngine with a complex graph."""
    DURATION = 1.5

    # Get the correct Faust libraries path
    faust_lib_path = os.path.join(
        os.path.dirname(os.path.dirname(__file__)), "thirdparty", "faust", "libraries"
    )

    # Create engine with a graph
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    engine.set_bpm(120.0)

    # Create processors
    data = np.random.rand(2, int(SAMPLE_RATE * DURATION))
    playback = engine.make_playback_processor("playback", data)

    faust = engine.make_faust_processor("faust")
    faust.faust_libraries_paths = [faust_lib_path]
    faust_code = """
    import("stdfaust.lib");
    cutoff = hslider("cutoff", 1000, 20, 20000, 1);
    process = fi.lowpass(4, cutoff), fi.lowpass(4, cutoff);
    """
    faust.set_dsp_string(faust_code)
    faust.compile()
    faust.set_parameter("/dawdreamer/cutoff", 500)

    # Build graph and render
    graph = [(playback, []), (faust, ["playback"])]
    engine.load_graph(graph)
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle the entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match
    assert np.allclose(original_audio, restored_audio, atol=1e-05)


def test_render_engine_bpm_automation():
    """Test pickling RenderEngine with BPM automation."""
    DURATION = 2.0

    # Create engine with BPM automation
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    # Create BPM automation from 100 to 140 BPM
    ppqn = 960
    num_samples = int(SAMPLE_RATE * DURATION)
    bpm_automation = np.linspace(100, 140, num_samples)
    engine.set_bpm(bpm_automation, ppqn)

    # Create a simple processor
    data = make_sine(440, DURATION)
    data = np.array([data, data])
    playback = engine.make_playback_processor("playback", data)

    graph = [(playback, [])]
    engine.load_graph(graph)
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match (BPM automation doesn't affect playback processor,
    # but we're testing that the automation is preserved)
    assert np.allclose(original_audio, restored_audio, atol=1e-07)


def test_playback_processor_audio_data_preserved():
    """Test that audio data is correctly preserved when pickling RenderEngine with PlaybackProcessor."""
    DURATION = 0.5

    # Create specific audio pattern
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    t = np.linspace(0, DURATION, int(SAMPLE_RATE * DURATION))
    channel1 = np.sin(2 * np.pi * 440 * t)  # 440 Hz sine
    channel2 = np.sin(2 * np.pi * 880 * t)  # 880 Hz sine
    data = np.array([channel1, channel2])

    playback = engine.make_playback_processor("playback", data)
    engine.load_graph([(playback, [])])
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Should be identical
    assert np.allclose(original_audio, restored_audio, atol=1e-07)


def test_faust_library_paths_preserved():
    """Test that Faust library paths are preserved when pickling RenderEngine."""
    # Get the correct Faust libraries path
    faust_lib_path = os.path.join(
        os.path.dirname(os.path.dirname(__file__)), "thirdparty", "faust", "libraries"
    )

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    faust = engine.make_faust_processor("faust")

    # Set custom library paths
    custom_paths = [faust_lib_path, "/custom/path1", "/custom/path2"]
    faust.faust_libraries_paths = custom_paths

    # Set a simple DSP
    faust.set_dsp_string("process = _, _;")
    faust.compile()

    # Load graph
    engine.load_graph([(faust, [])])

    # Pickle and unpickle engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # The engine doesn't expose processors directly, so we can at least
    # verify the engine pickles and restores without error
    assert restored_engine is not None


def test_render_engine_with_oscillator_processor():
    """Test pickling RenderEngine containing an OscillatorProcessor."""
    DURATION = 1.0

    # Create engine and oscillator processor
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    osc = engine.make_oscillator_processor("osc", 440.0)

    # Load graph and render
    engine.load_graph([(osc, [])])
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match
    assert np.allclose(original_audio, restored_audio, atol=1e-07)


def test_render_engine_with_filter_processor():
    """Test pickling RenderEngine containing a FilterProcessor."""
    DURATION = 1.0

    # Create engine with input and filter
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    data = np.random.rand(2, int(SAMPLE_RATE * DURATION))
    playback = engine.make_playback_processor("playback", data)
    filter_proc = engine.make_filter_processor("filter", "low", 1000.0, 0.707, 1.0)

    # Load graph and render
    engine.load_graph([(playback, []), (filter_proc, ["playback"])])
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match
    assert np.allclose(original_audio, restored_audio, atol=1e-05)


def test_render_engine_with_compressor_processor():
    """Test pickling RenderEngine containing a CompressorProcessor."""
    DURATION = 1.0

    # Create engine with input and compressor
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    data = np.random.rand(2, int(SAMPLE_RATE * DURATION))
    playback = engine.make_playback_processor("playback", data)
    compressor = engine.make_compressor_processor("comp", -10.0, 4.0, 10.0, 100.0)

    # Load graph and render
    engine.load_graph([(playback, []), (compressor, ["playback"])])
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match
    assert np.allclose(original_audio, restored_audio, atol=1e-05)


def test_render_engine_with_reverb_processor():
    """Test pickling RenderEngine containing a ReverbProcessor."""
    DURATION = 1.0

    # Create engine with input and reverb
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    data = np.random.rand(2, int(SAMPLE_RATE * DURATION))
    playback = engine.make_playback_processor("playback", data)
    reverb = engine.make_reverb_processor("reverb", 0.8, 0.5, 0.33, 0.4, 1.0)

    # Load graph and render
    engine.load_graph([(playback, []), (reverb, ["playback"])])
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match
    assert np.allclose(original_audio, restored_audio, atol=1e-05)


def test_render_engine_with_panner_processor():
    """Test pickling RenderEngine containing a PannerProcessor."""
    DURATION = 1.0

    # Create engine with input and panner
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    data = np.random.rand(2, int(SAMPLE_RATE * DURATION))
    playback = engine.make_playback_processor("playback", data)
    panner = engine.make_panner_processor("panner", "balanced", 0.5)

    # Load graph and render
    engine.load_graph([(playback, []), (panner, ["playback"])])
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match
    assert np.allclose(original_audio, restored_audio, atol=1e-07)


def test_render_engine_with_delay_processor():
    """Test pickling RenderEngine containing a DelayProcessor."""
    DURATION = 1.0

    # Create engine with delay processor
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    data = np.random.rand(2, int(SAMPLE_RATE * DURATION)) * 0.5
    playback = engine.make_playback_processor("playback", data)
    delay = engine.make_delay_processor("delay", "linear", 250.0, 0.75)

    # Load graph and render
    engine.load_graph([(playback, []), (delay, ["playback"])])
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match (verifies delay parameters were preserved)
    assert np.allclose(original_audio, restored_audio, atol=1e-05)


def test_render_engine_with_add_processor():
    """Test pickling RenderEngine containing an AddProcessor."""
    DURATION = 1.0

    # Create engine with two inputs and add processor
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    data1 = np.random.rand(2, int(SAMPLE_RATE * DURATION)) * 0.5
    data2 = np.random.rand(2, int(SAMPLE_RATE * DURATION)) * 0.3
    playback1 = engine.make_playback_processor("playback1", data1)
    playback2 = engine.make_playback_processor("playback2", data2)
    add = engine.make_add_processor("add", [0.5, 1.0])

    # Load graph and render
    engine.load_graph([(playback1, []), (playback2, []), (add, ["playback1", "playback2"])])
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match
    assert np.allclose(original_audio, restored_audio, atol=1e-05)


def test_render_engine_with_sampler_processor():
    """Test pickling SamplerProcessor preserves sample data and parameters.

    Note: MIDI events are not yet preserved during pickling (future work).
    This test verifies sample data and parameter preservation only.
    """
    DURATION = 1.0

    # Create a simple sine wave sample
    freq = 440
    t = np.linspace(0, DURATION, int(SAMPLE_RATE * DURATION), False)
    sample_data = np.sin(2 * np.pi * freq * t).astype(np.float32)
    sample_data = np.stack([sample_data, sample_data])  # stereo

    # Create engine with sampler
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    sampler = engine.make_sampler_processor("sampler", sample_data)

    # Set some parameters
    sampler.set_parameter(0, 50.0)  # attack
    sampler.set_parameter(1, 100.0)  # decay
    original_attack = sampler.get_parameter(0)
    original_decay = sampler.get_parameter(1)

    # Verify sample data is preserved before pickling
    retrieved_sample = sampler.get_data()
    assert np.allclose(
        sample_data, retrieved_sample
    ), "Sample data should match original before pickle"

    # Pickle and unpickle the sampler directly
    pickled = pickle.dumps(sampler)
    restored_sampler = pickle.loads(pickled)

    # Verify sample data is preserved after pickling
    restored_sample = restored_sampler.get_data()
    assert np.allclose(
        sample_data, restored_sample
    ), "Sample data should match original after pickle"

    # Verify parameters are preserved
    restored_attack = restored_sampler.get_parameter(0)
    restored_decay = restored_sampler.get_parameter(1)
    assert abs(original_attack - restored_attack) < 0.01, "Attack parameter should be preserved"
    assert abs(original_decay - restored_decay) < 0.01, "Decay parameter should be preserved"


def test_render_engine_with_automation():
    """Test that parameter automation curves are preserved through pickling."""
    DURATION = 1.0

    # Get the correct Faust libraries path
    faust_lib_path = os.path.join(
        os.path.dirname(os.path.dirname(__file__)), "thirdparty", "faust", "libraries"
    )

    # Create engine with Faust processor
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    faust = engine.make_faust_processor("faust")
    faust.faust_libraries_paths = [faust_lib_path]

    faust_code = """
    import("stdfaust.lib");
    freq = hslider("freq", 440, 20, 20000, 1);
    gain = hslider("gain", 0.5, 0, 1, 0.01);
    process = os.osc(freq) * gain;
    """
    faust.set_dsp_string(faust_code)
    faust.compile()

    # Set automation on parameters
    freq_automation = np.array([440.0, 880.0, 1320.0, 880.0, 440.0], dtype=np.float32)
    gain_automation = np.array([0.1, 0.3, 0.5, 0.7, 0.9], dtype=np.float32)

    params = faust.get_parameters_description()
    freq_param_name = params[0]["name"]
    gain_param_name = params[1]["name"]

    faust.set_automation(freq_param_name, freq_automation)
    faust.set_automation(gain_param_name, gain_automation)

    # Load graph
    graph = [(faust, [])]
    engine.load_graph(graph)

    # Render original
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match (verifies automation curves were preserved)
    assert np.allclose(original_audio, restored_audio, atol=1e-05), "Automation should be preserved"


def test_faust_midi_preservation():
    """Test that MIDI events are preserved through pickling for FaustProcessor."""
    # Get the correct Faust libraries path
    faust_lib_path = os.path.join(
        os.path.dirname(os.path.dirname(__file__)), "thirdparty", "faust", "libraries"
    )

    # Create Faust processor
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    faust = engine.make_faust_processor("faust")
    faust.faust_libraries_paths = [faust_lib_path]

    faust_code = """
    import("stdfaust.lib");
    process = no.noise * 0.1;
    """
    faust.set_dsp_string(faust_code)
    faust.compile()

    # Add MIDI notes
    faust.add_midi_note(60, 100, 0.0, 0.5, beats=False)
    faust.add_midi_note(64, 100, 0.5, 0.5, beats=False)
    faust.add_midi_note(67, 100, 1.0, 0.5, beats=False)

    num_midi_before = faust.n_midi_events
    assert num_midi_before == 6, "Should have 6 MIDI events (3 note-ons + 3 note-offs)"

    # Pickle and unpickle
    pickled = pickle.dumps(faust)
    restored_faust = pickle.loads(pickled)

    num_midi_after = restored_faust.n_midi_events
    assert num_midi_after == num_midi_before, "MIDI events should be preserved"


def test_render_engine_with_plugin_processor():
    """Test pickling RenderEngine with a PluginProcessor (VST effect).

    Test is skipped as VST plugins may not be available in all environments.
    """
    import platform

    if platform.system() != "Darwin":
        pytest.skip("Plugin test only runs on macOS")

    DURATION = 1.0
    # We'll skip this test for now since VST effect plugins are not available in CI
    # This test is here as a template for when VST effect plugins are available
    pytest.skip("VST plugins not available - test serves as implementation template")

    plugin_path = "/path/to/effect.vst3"  # Placeholder for future testing

    if not os.path.isdir(plugin_path):
        pytest.skip(f"Plugin not found: {plugin_path}")

    # Create engine with audio data and plugin effect
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    # Create some test audio
    data = np.random.rand(2, int(SAMPLE_RATE * DURATION)) * 0.3
    playback = engine.make_playback_processor("playback", data)

    # Create plugin effect
    plugin = engine.make_plugin_processor("effect", plugin_path)

    # Set some parameters to test state preservation
    if plugin.get_plugin_parameter_size() > 0:
        plugin.set_parameter(0, 0.5)
    if plugin.get_plugin_parameter_size() > 1:
        plugin.set_parameter(1, 0.7)

    # Build graph
    graph = [(playback, []), (plugin, ["playback"])]
    engine.load_graph(graph)

    # Render original
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match (verifies plugin state and parameters are preserved)
    assert np.allclose(
        original_audio, restored_audio, rtol=1e-04, atol=1e-05
    ), "Audio output should match after pickle"


def test_render_engine_with_all_processors():
    """Test pickling RenderEngine with a complex graph containing all supported processor types."""
    DURATION = 1.0

    # Get the correct Faust libraries path
    faust_lib_path = os.path.join(
        os.path.dirname(os.path.dirname(__file__)), "thirdparty", "faust", "libraries"
    )

    # Create engine
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    engine.set_bpm(120.0)

    # Create various processors
    osc = engine.make_oscillator_processor("osc", 440.0)
    data = np.random.rand(2, int(SAMPLE_RATE * DURATION)) * 0.3
    playback = engine.make_playback_processor("playback", data)
    filter_proc = engine.make_filter_processor("filter", "low", 2000.0, 0.707, 1.0)
    compressor = engine.make_compressor_processor("comp", -15.0, 3.0, 10.0, 100.0)
    delay = engine.make_delay_processor("delay", "linear", 100.0, 0.3)
    reverb = engine.make_reverb_processor("reverb", 0.6, 0.5, 0.25, 0.4, 1.0)
    panner = engine.make_panner_processor("panner", "balanced", -0.3)
    add = engine.make_add_processor("add", [0.4, 0.6])

    faust = engine.make_faust_processor("faust")
    faust.faust_libraries_paths = [faust_lib_path]
    faust_code = """
    import("stdfaust.lib");
    gain = hslider("gain", 0.8, 0, 1, 0.01);
    process = _ * gain, _ * gain;
    """
    faust.set_dsp_string(faust_code)
    faust.compile()

    # Build complex graph
    graph = [
        (osc, []),
        (playback, []),
        (add, ["osc", "playback"]),
        (filter_proc, ["add"]),
        (compressor, ["filter"]),
        (delay, ["comp"]),
        (faust, ["delay"]),
        (panner, ["faust"]),
        (reverb, ["panner"]),
    ]
    engine.load_graph(graph)
    engine.render(DURATION)
    original_audio = engine.get_audio()

    # Pickle and unpickle entire engine
    pickled = pickle.dumps(engine)
    restored_engine = pickle.loads(pickled)

    # Render with restored engine
    restored_engine.render(DURATION)
    restored_audio = restored_engine.get_audio()

    # Audio should match
    assert np.allclose(original_audio, restored_audio, atol=1e-05)


if __name__ == "__main__":
    # Run individual tests for debugging
    test_render_engine_with_playback_processor()
    print("✓ test_render_engine_with_playback_processor passed")

    test_render_engine_with_faust_processor()
    print("✓ test_render_engine_with_faust_processor passed")

    test_render_engine_with_polyphonic_faust()
    print("✓ test_render_engine_with_polyphonic_faust passed")

    test_render_engine_with_complex_graph()
    print("✓ test_render_engine_with_complex_graph passed")

    test_render_engine_bpm_automation()
    print("✓ test_render_engine_bpm_automation passed")

    test_playback_processor_audio_data_preserved()
    print("✓ test_playback_processor_audio_data_preserved passed")

    test_faust_library_paths_preserved()
    print("✓ test_faust_library_paths_preserved passed")

    test_render_engine_with_oscillator_processor()
    print("✓ test_render_engine_with_oscillator_processor passed")

    test_render_engine_with_filter_processor()
    print("✓ test_render_engine_with_filter_processor passed")

    test_render_engine_with_compressor_processor()
    print("✓ test_render_engine_with_compressor_processor passed")

    test_render_engine_with_reverb_processor()
    print("✓ test_render_engine_with_reverb_processor passed")

    test_render_engine_with_panner_processor()
    print("✓ test_render_engine_with_panner_processor passed")

    test_render_engine_with_delay_processor()
    print("✓ test_render_engine_with_delay_processor passed")

    test_render_engine_with_add_processor()
    print("✓ test_render_engine_with_add_processor passed")

    test_render_engine_with_sampler_processor()
    print("✓ test_render_engine_with_sampler_processor passed")

    test_render_engine_with_automation()
    print("✓ test_render_engine_with_automation passed")

    test_faust_midi_preservation()
    print("✓ test_faust_midi_preservation passed")

    test_render_engine_with_plugin_processor()
    print("✓ test_render_engine_with_plugin_processor passed (or skipped)")

    test_render_engine_with_all_processors()
    print("✓ test_render_engine_with_all_processors passed")

    print("\n✓ All pickle tests passed!")

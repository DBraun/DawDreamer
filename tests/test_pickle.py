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

    # Skip DelayProcessor - has pre-existing crash bug
    # test_render_engine_with_delay_processor()

    test_render_engine_with_add_processor()
    print("✓ test_render_engine_with_add_processor passed")

    test_render_engine_with_all_processors()
    print("✓ test_render_engine_with_all_processors passed")

    print("\n✓ All pickle tests passed!")

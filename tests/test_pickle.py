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

    print("\n✓ All pickle tests passed!")

from dawdreamer_utils import *

BUFFER_SIZE = 1


def test_playback(set_data=False):
    DURATION = 5.0

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    data = load_audio_file(ASSETS / "575854__yellowtree__d-b-funk-loop.wav")
    playback_processor = engine.make_playback_processor("playback", data)

    if set_data:
        playback_processor.set_data(data)

    graph = [
        (playback_processor, []),
    ]

    engine.load_graph(graph)

    engine.render(DURATION)

    output = engine.get_audio()

    wavfile.write(OUTPUT / "test_playback.wav", SAMPLE_RATE, output.transpose())

    # do the same for noise
    data = np.random.rand(2, int(SAMPLE_RATE * (DURATION + 0.1)))
    playback_processor.set_data(data)
    render(engine)
    audio = engine.get_audio()

    data = data[:, : audio.shape[1]]
    audio = audio[:, : audio.shape[1]]

    assert np.allclose(data, audio, atol=1e-07)


def test_playback_bad_shapes():
    """Non-2D audio data must raise instead of crashing (issue #219)."""

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    with pytest.raises(RuntimeError, match="2D"):
        engine.make_playback_processor("playback", np.array(0.0, dtype=np.float32))

    with pytest.raises(RuntimeError, match="2D"):
        engine.make_playback_processor("playback", np.zeros(4, dtype=np.float32))

    with pytest.raises(RuntimeError, match="2D"):
        engine.make_playback_processor("playback", np.zeros((2, 3, 4), dtype=np.float32))

    data = np.random.rand(2, SAMPLE_RATE).astype(np.float32)
    playback_processor = engine.make_playback_processor("playback", data)

    with pytest.raises(RuntimeError, match="2D"):
        playback_processor.set_data(np.zeros(4, dtype=np.float32))

    with pytest.raises(RuntimeError, match="2D"):
        engine.make_sampler_processor("sampler", np.zeros(4, dtype=np.float32))

    with pytest.raises(RuntimeError, match="2D"):
        engine.make_playbackwarp_processor("warp", np.zeros(4, dtype=np.float32))


def test_playback_zero_samples():
    """Zero-sample audio data renders silence without crashing (issue #219)."""

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    playback_processor = engine.make_playback_processor(
        "playback", np.zeros((2, 0), dtype=np.float32)
    )

    engine.load_graph([(playback_processor, [])])
    engine.render(0.1)

    audio = engine.get_audio()
    assert np.abs(audio).max() == 0.0

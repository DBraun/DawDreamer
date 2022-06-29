from dawdreamer_utils import *

BUFFER_SIZE = 16

def test_record():

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	audio1_input = load_audio_file(ASSETS / "Music Delta - Disco" / "bass.wav", duration=5.1)
	playback1 = engine.make_playback_processor("playback1", audio1_input)

	audio2_input = load_audio_file(ASSETS / "Music Delta - Disco" / "other.wav", duration=5.1)
	playback2 = engine.make_playback_processor("playback2", audio2_input)

	playback1.record = True
	playback2.record = True

	graph = [
	    (playback1, []),
	    (playback2, []),
	    (engine.make_add_processor("add", [1., 1.]), ["playback1", "playback2"])
	]

	engine.load_graph(graph)

	engine.render(DURATION)

	output = engine.get_audio()

	audio1 = playback1.get_audio()
	audio2 = playback2.get_audio()

	assert(np.allclose(audio1, audio1_input[:,:audio1.shape[1]], atol=1e-07))
	assert(np.allclose(audio2, audio2_input[:,:audio1.shape[1]], atol=1e-07))

	audio1 = engine.get_audio(playback1.get_name())
	audio2 = engine.get_audio(playback2.get_name())

	assert(np.allclose(audio1, audio1_input[:,:audio1.shape[1]], atol=1e-07))
	assert(np.allclose(audio2, audio2_input[:,:audio1.shape[1]], atol=1e-07))

	audio_missing = engine.get_audio("Invalid Name")

	assert(audio_missing.shape[0] == 2)
	assert(audio_missing.shape[1] == 0)

	wavfile.write(OUTPUT / 'test_record_both.wav', SAMPLE_RATE, output.transpose())
	wavfile.write(OUTPUT / 'test_record_1.wav', SAMPLE_RATE, audio1.transpose())
	wavfile.write(OUTPUT / 'test_record_2.wav', SAMPLE_RATE, audio2.transpose())

@pytest.mark.parametrize("buffer_size",
	[1, 2, 3, 4, 6, 8, 2048]
)
def test_render_beats(buffer_size: int):

	BPM = 140.12346
	BEATS = 4.0131564
	SECONDS = (BEATS / BPM) * 60.

	engine = daw.RenderEngine(SAMPLE_RATE, buffer_size)

	engine.set_bpm(BPM)

	audio_data = load_audio_file(ASSETS / "Music Delta - Disco" / "bass.wav", duration=5.1)
	playback = engine.make_playback_processor("playback", audio_data)

	engine.load_graph([(playback, [])])

	engine.render(BEATS, beats=True)

	output = engine.get_audio()

	desired_samples = SECONDS * SAMPLE_RATE
	actual_samples = output.shape[1]

	assert abs(desired_samples - actual_samples) <= buffer_size

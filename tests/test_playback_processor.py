from dawdreamer_utils import *

BUFFER_SIZE = 1

def test_playback(set_data=False):

	DURATION = 5.

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

	wavfile.write(OUTPUT / 'test_playback.wav', SAMPLE_RATE, output.transpose())

	# do the same for noise
	data = np.random.rand(2, int(SAMPLE_RATE*(DURATION+.1)))
	playback_processor.set_data(data)
	render(engine)
	audio = engine.get_audio()

	data = data[:,:audio.shape[1]]
	audio = audio[:,:audio.shape[1]]

	assert(np.allclose(data, audio, atol=1e-07))

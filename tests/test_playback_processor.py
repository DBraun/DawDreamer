from utils import *

BUFFER_SIZE = 16

def test_playback(set_data=False):

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav")
	playback_processor = engine.make_playback_processor("playback", data)

	if set_data:
		playback_processor.set_data(data)

	graph = [
	    (playback_processor, []),
	]

	assert(engine.load_graph(graph))

	engine.render(DURATION)

	output = engine.get_audio()

	wavfile.write('output/test_playback.wav', SAMPLE_RATE, output.transpose())

from dawdreamer_utils import *

from itertools import product

BUFFER_SIZE = 16

@pytest.mark.parametrize("duration,buffer_size,set_data",
	product(
		[16./SAMPLE_RATE, 16.5/SAMPLE_RATE, 16.9/SAMPLE_RATE, .1, 1.,1.23535],
		[1, 2, 4, 8, 16, 128, 2048],
		[True, False]
		))
def test_impulse(duration, buffer_size, set_data):

	engine = daw.RenderEngine(SAMPLE_RATE, buffer_size)

	num_samples = int(duration*SAMPLE_RATE)
	impulse_input = np.random.uniform(low=-1, high=1, size=(2, num_samples))
	playback_processor = engine.make_playback_processor("playback", impulse_input)

	if set_data:
		playback_processor.set_data(impulse_input)

	graph = [
	    (playback_processor, []),
	]

	engine.load_graph(graph)

	engine.render(duration)

	output = engine.get_audio()

	assert(np.allclose(impulse_input, output, atol=1e-07))

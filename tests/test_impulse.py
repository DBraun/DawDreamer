import numpy as np
import pytest

import dawdreamer as daw

SAMPLE_RATE = 44100
BUFFER_SIZE = 16

def make_impulse(duration):

    num_samples = int(duration*SAMPLE_RATE)

    y = np.zeros((2, num_samples), np.float32)
    y[0, 0:5] = np.arange(5)
    y[1, 0:5] = 5+np.arange(5)

    import random
    for _ in range(20):

        ind = random.randint(1, int(num_samples)-1)
        y[:, ind] = random.random()

    return y

def _test_impulse(set_data=False):

	DURATION = .0007

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	impulse_input = make_impulse(DURATION)
	playback_processor = engine.make_playback_processor("playback", impulse_input)

	if set_data:
		playback_processor.set_data(impulse_input)

	graph = [
	    (playback_processor, []),
	]

	engine.load_graph(graph)

	engine.render(DURATION)

	output = engine.get_audio()

	assert(np.allclose(impulse_input, output, atol=1e-07))


def test_impulse1():
	_test_impulse(False)

def test_impulse2():
	_test_impulse(True)

import pytest
from scipy.io import wavfile

from utils import *
import dawdreamer as daw

BUFFER_SIZE = 16
SAMPLE_RATE = 44100

def test_record():

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	audio1_input = load_audio_file("assets/Music Delta - Disco/bass.wav", duration=5.1)
	playback1 = engine.make_playback_processor("playback1", audio1_input)

	audio2_input = load_audio_file("assets/Music Delta - Disco/other.wav", duration=5.1)
	playback2 = engine.make_playback_processor("playback2", audio2_input)

	playback1.record = True
	playback2.record = True

	graph = [
	    (playback1, []),
	    (playback2, []),
	    (engine.make_add_processor("add", [1., 1.]), ["playback1", "playback2"])
	]

	assert(engine.load_graph(graph))

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

	wavfile.write('output/test_record_both.wav', SAMPLE_RATE, output.transpose())
	wavfile.write('output/test_record_1.wav', SAMPLE_RATE, audio1.transpose())
	wavfile.write('output/test_record_2.wav', SAMPLE_RATE, audio2.transpose())

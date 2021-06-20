import pytest
import librosa

import dawdreamer as daw

SAMPLE_RATE = 44100
BUFFER_SIZE = 16

def load_audio_file(file_path, duration=None):
	sig, rate = librosa.load(file_path, duration=duration, mono=False, sr=SAMPLE_RATE)
	assert(rate == SAMPLE_RATE)
	return sig

def test_playback1(set_data=False):

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav")
	playback_processor = engine.make_playback_processor("playback", data)

	if set_data:
		playback_processor.set_data(data)

	graph = [
	    (playback_processor, []),
	]

	engine.load_graph(graph)

	engine.render(DURATION)

	output = engine.get_audio()

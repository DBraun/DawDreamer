import pytest
import librosa
from scipy.io import wavfile
from os.path import abspath

from utils import *
import dawdreamer as daw

BUFFER_SIZE = 16

def test_plugin_effect(set_data=False):

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav", DURATION+.1)
	playback_processor = engine.make_playback_processor("playback", data)

	if set_data:
		playback_processor.set_data(data)

	effect = engine.make_plugin_processor("effect", abspath("plugins/Dimension Expander_x64.dll"))

	graph = [
	    (playback_processor, []),
	    (effect, ["playback"])
	]

	engine.load_graph(graph)

	render(engine, file_path='output/test_plugin_effect.wav', duration=DURATION)

def test_plugin_instrument(set_data=False):

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	synth = engine.make_plugin_processor("synth", abspath("plugins/TAL-NoiseMaker-64.dll"))

	 # (MIDI note, velocity, start sec, duration sec)
	synth.add_midi_note(60, 60, 0.0, .25)
	synth.add_midi_note(64, 80, 0.5, .5)
	synth.add_midi_note(67, 127, 0.75, .5)

	graph = [
	    (synth, []),
	]

	engine.load_graph(graph)

	render(engine, file_path='output/test_plugin_instrument.wav', duration=DURATION)

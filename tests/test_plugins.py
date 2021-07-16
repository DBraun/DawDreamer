import pytest
from os.path import abspath
import platform

from utils import *
import dawdreamer as daw

BUFFER_SIZE = 16

MY_SYSTEM = platform.system()
# "Darwin" is macOS. "Windows" is Windows.

def test_plugin_effect(set_data=False):

	if MY_SYSTEM not in ["Darwin", "Windows"]:
		# We don't test LV2 plugins on Linux yet.
		return

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav", DURATION+.1)
	playback_processor = engine.make_playback_processor("playback", data)

	if set_data:
		playback_processor.set_data(data)

	plugin_name = "DimensionExpander.vst" if MY_SYSTEM == "Darwin" else "Dimension Expander_x64.dll"

	effect = engine.make_plugin_processor("effect", abspath("plugins/"+plugin_name))

	graph = [
	    (playback_processor, []),
	    (effect, ["playback"])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_plugin_effect.wav', duration=DURATION)

def test_plugin_instrument(set_data=False):

	if MY_SYSTEM not in ["Darwin", "Windows"]:
		# We don't test LV2 plugins on Linux yet.
		return

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	plugin_name = "TAL-NoiseMaker.vst" if MY_SYSTEM == "Darwin" else "TAL-NoiseMaker-64.dll"

	synth = engine.make_plugin_processor("synth", abspath("plugins/"+plugin_name))

	 # (MIDI note, velocity, start sec, duration sec)
	synth.add_midi_note(60, 60, 0.0, .25)
	synth.add_midi_note(64, 80, 0.5, .5)
	synth.add_midi_note(67, 127, 0.75, .5)

	assert(synth.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	graph = [
	    (synth, []),
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_plugin_instrument.wav', duration=DURATION)

	assert(not synth.load_preset('bogus_path.fxp'))

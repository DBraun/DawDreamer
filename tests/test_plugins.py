from utils import *
import platform

MY_SYSTEM = platform.system()
# "Darwin" is macOS. "Windows" is Windows.

BUFFER_SIZE = 16

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

	# print(effect.get_plugin_parameters_description())

	graph = [
	    (playback_processor, []),
	    (effect, ["playback"])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_plugin_effect.wav', duration=DURATION)

def test_plugin_instrument():

	if MY_SYSTEM not in ["Darwin", "Windows"]:
		# We don't test LV2 plugins on Linux yet.
		return

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	plugin_name = "TAL-NoiseMaker.vst" if MY_SYSTEM == "Darwin" else "TAL-NoiseMaker-64.dll"

	synth = engine.make_plugin_processor("synth", abspath("plugins/"+plugin_name))

	# print(synth.get_plugin_parameters_description())

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

def test_plugin_serum():

	if MY_SYSTEM not in ["Windows"]:
		# We don't Serum on platforms other than Windows.
		return

	plugin_path = "C:/VSTPlugIns/Serum_x64.dll"

	if not isfile(plugin_path):
		return

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	synth = engine.make_plugin_processor("synth", plugin_path)

	# print(synth.get_plugin_parameters_description())

	synth.get_parameter(0)
	assert(synth.set_parameter(0, synth.get_parameter(0)))
	assert(synth.set_automation(0, np.array([synth.get_parameter(0)])))

	 # (MIDI note, velocity, start sec, duration sec)
	synth.add_midi_note(60, 60, 0.0, .25)
	synth.add_midi_note(64, 80, 0.5, .5)
	synth.add_midi_note(67, 127, 0.75, .5)

	assert(synth.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	graph = [
	    (synth, []),
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_plugin_serum.wav', duration=DURATION)

	audio = engine.get_audio()
	assert(not np.allclose(audio*0., audio, atol=1e-07))

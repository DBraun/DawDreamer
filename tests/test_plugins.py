from utils import *
import platform
import os.path

MY_SYSTEM = platform.system()
# "Darwin" is macOS. "Windows" is Windows.

BUFFER_SIZE = 16

def _test_stereo_plugin_effect(plugin_path):

	if MY_SYSTEM == 'Darwin':
		# macOS treats .component and .vst3 as directories
		assert(os.path.isdir(plugin_path))
	else:
		assert(os.path.isfile(plugin_path))

	DURATION = 5.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav", DURATION+.1)
	playback_processor = engine.make_playback_processor("playback", data)

	effect = engine.make_plugin_processor("effect", plugin_path)

	# print(effect.get_plugin_parameters_description())
	assert(effect.get_num_input_channels() == 2)
	assert(effect.get_num_output_channels() == 2)

	graph = [
	    (playback_processor, []),
	    (effect, ["playback"])
	]

	assert(engine.load_graph(graph))

	plugin_basename = os.path.basename(plugin_path)

	render(engine, file_path=f'output/test_plugin_effect_{plugin_basename}.wav', duration=DURATION)

	# check that it's non-silent
	audio = engine.get_audio()
	assert(np.mean(np.abs(audio)) > .05)

def test_stereo_plugin_effects():

	if MY_SYSTEM not in ["Darwin", "Windows"]:
		# todo: we should test LV2 plugins on Linux.
		return

	plugin_paths = []

	if MY_SYSTEM == 'Darwin':
		plugin_paths.append(abspath("plugins/ValhallaFreqEcho.vst"))
		plugin_paths.append(abspath("plugins/ValhallaFreqEcho.vst3"))
		plugin_paths.append(abspath("plugins/ValhallaFreqEcho.component"))
	elif MY_SYSTEM == 'Windows':
		plugin_paths.append(abspath("plugins/Dimension Expander_x64.dll"))

	for plugin_path in plugin_paths:
		_test_stereo_plugin_effect(plugin_path)

def test_plugin_instrument():

	if MY_SYSTEM not in ["Darwin", "Windows"]:
		# todo: we should test LV2 plugins on Linux.
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

	assert(synth.get_num_input_channels() == 0)
	assert(synth.get_num_output_channels() == 2)

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


def _test_plugin_goodhertz_sidechain(do_sidechain=True):

	if MY_SYSTEM not in ["Windows"]:
		# We don't Goodhertz on platforms other than Windows.
		return

	plugin_path = "C:/VSTPlugIns/Goodhertz/Ghz Vulf Compressor 3.vst3"

	if not isfile(plugin_path):
		return

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	DURATION = 5.1

	vocals = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav", duration=DURATION)
	drums = load_audio_file("assets/60988__folktelemetry__crash-fast-14.wav", duration=DURATION)

	drums *= .1

	vocals_processor = engine.make_playback_processor("vocals", vocals)
	drums_processor = engine.make_playback_processor("drums", drums)

	plugin = engine.make_plugin_processor("plugin", plugin_path)

	# plugin.set_parameter(2, 1.)
	# plugin.set_parameter(5, .1)
	# plugin.set_parameter(13, 1.)
	# plugin.set_parameter(15, 1.)
	# plugin.set_parameter(18, 1.)

	if do_sidechain:

		plugin.set_parameter(19, 0.5)
		# parameter 19 is the "External Sidechain" for Vulf Compressor. In the UI, click the three dots, which opens the panel
		# Then look for "External Sidechain" and set it to 50%.

		graph = [
			(vocals_processor, []),
			(drums_processor, []),
			(plugin, ["vocals", "drums"])
		]
	else:
		graph = [
			(vocals_processor, []),
			(plugin, ["vocals"])
		]

	assert(engine.load_graph(graph))

	sidechain_on = "on" if do_sidechain else "off"

	file_path = f'output/test_plugin_goodhertz_sidechain_{sidechain_on}.wav'

	render(engine, file_path=file_path, duration=DURATION)

	audio = engine.get_audio()
	assert(not np.allclose(audio*0., audio, atol=1e-07))


def test_plugin_goodhertz_sidechain():
	_test_plugin_goodhertz_sidechain(do_sidechain=True)
	_test_plugin_goodhertz_sidechain(do_sidechain=False)


# def test_plugin_effect_ambisonics(set_data=False):

# 	if MY_SYSTEM != "Windows":
# 		return

# 	DURATION = 5.

# 	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

# 	data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav", DURATION+.1)
# 	playback_processor = engine.make_playback_processor("playback", data)

# 	data = data.mean(axis=0, keepdims=True)

# 	if set_data:
# 		playback_processor.set_data(data)

# 	plugin_name = "sparta_ambiENC.vst" if MY_SYSTEM == "Darwin" else "sparta_ambiENC.dll"

# 	plugin_path = abspath("plugins/"+plugin_name)
# 	if not isfile(plugin_path):
# 		return

# 	effect = engine.make_plugin_processor("effect", plugin_path)

# 	effect.set_parameter(0, 1)

# 	assert(effect.get_num_input_channels() == 64)
# 	assert(effect.get_num_output_channels() == 64)

# 	# for par in effect.get_plugin_parameters_description():
# 	# 	print(par)

# 	graph = [
# 	    (playback_processor, []),
# 	    (effect, ["playback"])
# 	]

# 	assert(engine.load_graph(graph))

# 	render(engine, file_path='output/test_plugin_effect_ambisonics.wav', duration=DURATION)

# 	audio = engine.get_audio()

# 	assert(effect.get_num_output_channels() == audio.shape[0])

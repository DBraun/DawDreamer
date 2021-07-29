from utils import *

BUFFER_SIZE = 1

def test_faust_passthrough():

	DURATION = 5.1

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav", duration=DURATION)
	playback_processor = engine.make_playback_processor("playback", data)

	faust_processor = engine.make_faust_processor("faust")
	assert(faust_processor.set_dsp_string('process = _, _;'))
	assert(faust_processor.compile())

	print(faust_processor.get_parameters_description())

	graph = [
	    (playback_processor, []),
	    (faust_processor, ["playback"])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_faust_passthrough.wav')

	audio = engine.get_audio()

	# Todo: the last sample is inaccurate by a little bit
	# So we trim the last sample and compare
	data = data[:,:audio.shape[1]-1]
	audio = audio[:,:audio.shape[1]-1]

	assert(np.allclose(data, audio, atol=1e-07))

def test_faust_sidechain():

	"""Have the volume of the drums attenuate the volume of the bass."""

	DURATION = 5.1

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	drums = engine.make_playback_processor("drums",
		load_audio_file("assets/Music Delta - Disco/drums.wav", duration=DURATION))

	bass = engine.make_playback_processor("bass",
		load_audio_file("assets/Music Delta - Disco/bass.wav", duration=DURATION))

	dsp_path = abspath("faust_dsp/sidechain.dsp")
	faust_processor = engine.make_faust_processor("faust")
	faust_processor.set_dsp(dsp_path)
	assert(faust_processor.compile())

	print(faust_processor.get_parameters_description())

	graph = [
	    (drums, []),
	    (bass, []),
	    (faust_processor, [bass.get_name(), drums.get_name()]),
	    (engine.make_add_processor("add", [1., 1.]), ["faust", "drums"])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_sidechain_on.wav')

	graph = [
	    (drums, []),
	    (bass, []),
	    (engine.make_add_processor("add", [1., 1.]), ["bass", "drums"])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_sidechain_off.wav')

def test_faust_zita_rev1(set_data=False):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav")
	playback_processor = engine.make_playback_processor("playback", data)

	if set_data:
		playback_processor.set_data(data)

	dsp_path = abspath("faust_dsp/dm.zita_rev1.dsp")
	faust_processor = engine.make_faust_processor("faust")
	faust_processor.set_dsp(dsp_path)
	assert(faust_processor.set_dsp(dsp_path))
	assert(faust_processor.compile())

	print(faust_processor.get_parameters_description())

	graph = [
	    (playback_processor, []),
	    (faust_processor, ["playback"])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_faust_dm.zita_rev1.wav')

def test_faust_automation():

	DURATION = 5.1

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	drums = engine.make_playback_processor("drums",
		load_audio_file("assets/Music Delta - Disco/drums.wav", duration=DURATION))

	other = engine.make_playback_processor("other",
		load_audio_file("assets/Music Delta - Disco/other.wav", duration=DURATION))

	dsp_path = abspath("faust_dsp/two_stereo_inputs_filter.dsp")
	faust_processor = engine.make_faust_processor("faust")
	faust_processor.set_dsp(dsp_path)
	assert(faust_processor.set_dsp(dsp_path))
	assert(faust_processor.compile())

	print(faust_processor.get_parameters_description())

	faust_processor.set_parameter("/MyEffect/cutoff", 7000.0)  # Change the cutoff frequency.
	assert(faust_processor.get_parameter("/MyEffect/cutoff") == 7000.)
	# or set automation like this
	faust_processor.set_automation("/MyEffect/cutoff", 10000+9000*make_sine(2, DURATION))

	graph = [
	    (drums, []),
	    (other, []),
	    (faust_processor, [drums.get_name(), other.get_name()])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_faust_automation.wav')

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

	# print(faust_processor.get_parameters_description())

	graph = [
	    (playback_processor, []),
	    (faust_processor, ["playback"])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_faust_passthrough.wav')

	audio = engine.get_audio()

	# Todo: the last sample is inaccurate by a little bit
	# So we trim the last sample and compare
	data = data[:,:audio.shape[1]]
	audio = audio[:,:audio.shape[1]]

	assert(np.allclose(data, audio, atol=1e-07))

	# do the same for noise
	data = np.random.rand(2, int(SAMPLE_RATE*(DURATION+.1)))
	playback_processor.set_data(data)
	render(engine)
	audio = engine.get_audio()

	data = data[:,:audio.shape[1]]
	audio = audio[:,:audio.shape[1]]

	assert(np.allclose(data, audio, atol=1e-07))


def test_faust_multichannel_in_out():

	DURATION = 5.1

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	numChannels = 9
	underscores = ",".join('_'*numChannels)

	data = np.sin(np.linspace(0, 4000, num=int(44100*(DURATION+.1))))
	data = np.stack([data for _ in range(numChannels)])

	playback_processor = engine.make_playback_processor("playback", data)

	faust_processor = engine.make_faust_processor("faust")
	assert(faust_processor.set_dsp_string(f'process = {underscores};'))
	assert(faust_processor.compile())

	# print(faust_processor.get_parameters_description())

	graph = [
	    (playback_processor, []),
	    (faust_processor, ["playback"])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_faust_multichannel_in_out.wav')

	audio = engine.get_audio()

	# Todo: the last sample is inaccurate by a little bit
	# So we trim the last sample and compare
	data = data[:,:audio.shape[1]]
	audio = audio[:,:audio.shape[1]]

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

	# print(faust_processor.get_parameters_description())

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

	# print(faust_processor.get_parameters_description())

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
	assert(faust_processor.set_dsp(dsp_path))
	assert(faust_processor.compile())

	# print(faust_processor.get_parameters_description())

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

def test_faust_ambisonics_encoding(ambisonics_order=2, set_data=False):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav")

	data = data.mean(axis=0, keepdims=True)

	assert(data.ndim == 2)
	assert(data.shape[0] == 1)

	playback_processor = engine.make_playback_processor("playback", data)

	if set_data:
		playback_processor.set_data(data)

	faust_processor = engine.make_faust_processor("faust")

	dsp_code = f"""

	import("stdfaust.lib");

	L = {ambisonics_order};

	encoder3Dxyz(n, x, tx, ty, tz) = ho.encoder3D(n, signal, angle, elevation)
	with {{
		max_gain = 10.; // todo: user can pick this.
		// 10 indicates don't multiply the signal by more than 10 even if it's very close in 3D
		xz_square = tx*tx+tz*tz;
	    signal = x / max(1./max_gain, sqrt(xz_square+ty*ty));

	    angle = atan2(tx, -tz);
	    elevation = atan2(ty, sqrt(xz_square));
	}};

	// todo: ma.EPSILON doesn't work on Linux?
	eps = .00001;
	//eps = ma.EPSILON;
	process(sig) = encoder3Dxyz(L, sig, 
	    hslider("tx", 1., -10000., 10000., eps),
	    hslider("ty", 1., -10000., 10000., eps),
	    hslider("tz", 1., -10000., 10000., eps)
	    );

	"""

	faust_processor.set_dsp_string(dsp_code)
	assert(faust_processor.compile())

	# print(faust_processor.get_parameters_description())

	assert(faust_processor.get_num_input_channels() == 1)
	assert(faust_processor.get_num_output_channels() == (ambisonics_order+1)**2)

	graph = [
	    (playback_processor, []),
	    (faust_processor, ["playback"])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_faust_ambisonics_encoding.wav')
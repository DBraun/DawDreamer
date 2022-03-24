from dawdreamer_utils import *

BUFFER_SIZE = 1

def _test_faust_poly_wavetable(wavecycle, output_path, lagrange_order=4):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	dsp_path = abspath(FAUST_DSP / "polyphonic_wavetable.dsp")
	faust_processor = engine.make_faust_processor("faust")
	faust_processor.num_voices = 8

	dsp_code = open(dsp_path).read()

	if wavecycle.ndim == 1:
		wavecycle = wavecycle.reshape(1, -1)

	dsp_code = """
LAGRANGE_ORDER = {LAGRANGE_ORDER}; // lagrange order. [2-4] are good choices.
""".format(LAGRANGE_ORDER=lagrange_order) + dsp_code

	# set_soundfiles
	soundfiles = {
		'myCycle': [wavecycle]
	}
	faust_processor.set_soundfiles(soundfiles)

	faust_processor.set_dsp_string(dsp_code)
	faust_processor.compile()

	desc = faust_processor.get_parameters_description()

	 # (MIDI note, velocity, start sec, duration sec)
	faust_processor.add_midi_note(60, 60, 0.0, .25)
	faust_processor.add_midi_note(64, 80, 0.5, .5)
	faust_processor.add_midi_note(67, 127, 0.75, .5)

	assert(faust_processor.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	graph = [
	    (faust_processor, [])
	]

	engine.load_graph(graph)

	render(engine, file_path=OUTPUT / output_path, duration=3.)

	# check that it's non-silent
	audio = engine.get_audio()
	assert(np.mean(np.abs(audio)) > .01)

def test_faust_poly_wavetable_sine():

	"""First make a very low resolution sine wave and use Lagrange interpolation in Faust.
	Then use a very high resolution sine wave which will sound perfect."""

	waveform_length = 4
	wavecycle = np.sin(np.pi*2.*np.arange(waveform_length)/float(waveform_length))
	output_path = 'test_faust_poly_wavetable_sine_4.wav'
	# this won't sound like a perfect sine wave, but it only used 4 samples!
	_test_faust_poly_wavetable(wavecycle, output_path)

	waveform_length = 4096
	wavecycle = np.sin(np.pi*2.*np.arange(waveform_length)/float(waveform_length))
	output_path = 'test_faust_poly_wavetable_sine_4096.wav'
	# this should sound perfect.
	_test_faust_poly_wavetable(wavecycle, output_path)

def test_faust_poly_wavetable_saw():

	waveform_length = 4
	wavecycle = -1.+2.*np.arange(waveform_length)/float(waveform_length)
	output_path = 'test_faust_poly_wavetable_saw.wav'
	_test_faust_poly_wavetable(wavecycle, output_path)
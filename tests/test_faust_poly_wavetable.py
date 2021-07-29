from utils import *

BUFFER_SIZE = 1

def _test_faust_poly_wavetable(wavecycle, output_path, lagrange_order=4):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	dsp_path = abspath("faust_dsp/polyphonic_wavetable.dsp")
	faust_processor = engine.make_faust_processor("faust")
	faust_processor.num_voices = 8

	dsp_code = open(dsp_path).read()

	waveform_length = wavecycle.shape[0]
	wavecycle = ",".join([str(num) for num in wavecycle.tolist()])
	# print(wavecycle)

	dsp_code = """
LAGRANGE_ORDER = {LAGRANGE_ORDER}; // lagrange order. [2-4] are good choices.
CYCLE_SEQ = waveform{{{CYCLE_SEQ}}} : !, _;
CYCLE_LENGTH = {CYCLE_LENGTH};
""".format(LAGRANGE_ORDER=lagrange_order,
	CYCLE_LENGTH=waveform_length,
	CYCLE_SEQ=wavecycle) + dsp_code
	# print('dsp code: ')
	# print(dsp_code)

	assert(faust_processor.set_dsp_string(dsp_code))
	assert(faust_processor.compile())

	desc = faust_processor.get_parameters_description()

	 # (MIDI note, velocity, start sec, duration sec)
	faust_processor.add_midi_note(60, 60, 0.0, .25)
	faust_processor.add_midi_note(64, 80, 0.5, .5)
	faust_processor.add_midi_note(67, 127, 0.75, .5)

	assert(faust_processor.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	graph = [
	    (faust_processor, [])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/'+output_path, duration=3.)

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
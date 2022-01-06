from utils import *

BUFFER_SIZE = 1024

# todo: this example is bad because it turns a waveform into a very long string,
#  which takes a long time to compile as Faust code. It would be better to
#  find a way to use the soundfile primitive with a wavecycle.
def _test_faust_poly_sampler(sample_seq, output_path, lagrange_order=4):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	faust_processor = engine.make_faust_processor("faust")
	faust_processor.num_voices = 8

	dsp_path = abspath("faust_dsp/polyphonic_sampler.dsp")
	dsp_code = open(dsp_path).read()

	waveform_length = sample_seq.shape[1]
	sample_l_seq = ",".join([str(num) for num in sample_seq[0].tolist()])
	sample_r_seq = ",".join([str(num) for num in sample_seq[1].tolist()])

	dsp_code = """
LAGRANGE_ORDER = {LAGRANGE_ORDER}; // lagrange order. [2-4] are good choices.
SAMPLE_L_SEQ = waveform{{{SAMPLE_L_SEQ}}} : !, _;
SAMPLE_R_SEQ = waveform{{{SAMPLE_R_SEQ}}} : !, _;
SAMPLE_LENGTH = {SAMPLE_LENGTH};
""".format(LAGRANGE_ORDER=lagrange_order,
	SAMPLE_LENGTH=waveform_length,
	SAMPLE_L_SEQ=sample_l_seq,
	SAMPLE_R_SEQ=sample_r_seq) + dsp_code
	# print('dsp code: ')
	# print(dsp_code)

	assert(faust_processor.set_dsp_string(dsp_code))
	assert(faust_processor.compile())

	desc = faust_processor.get_parameters_description()
	# for par in desc:
	# 	print(par)

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

def test_faust_poly_sampler_cymbal():
	# Load a stereo audio sample and pass it to Faust
	sample_seq = load_audio_file("assets/60988__folktelemetry__crash-fast-14.wav")
	_test_faust_poly_sampler(sample_seq, 'test_faust_poly_sampler_cymbal.wav', lagrange_order=4)
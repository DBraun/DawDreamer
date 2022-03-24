from dawdreamer_utils import *

BUFFER_SIZE = 1024

def _test_faust_poly_sampler(sample_seq, output_path, lagrange_order=4):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	faust_processor = engine.make_faust_processor("faust")
	faust_processor.num_voices = 8

	# set_soundfiles
	if sample_seq.ndim == 1:
		sample_seq = sample_seq.reshape(1, -1)
	soundfiles = {
		'mySample': [sample_seq]
	}
	faust_processor.set_soundfiles(soundfiles)

	dsp_path = abspath(FAUST_DSP / "polyphonic_sampler.dsp")
	dsp_code = open(dsp_path).read()

	dsp_code = """
LAGRANGE_ORDER = {LAGRANGE_ORDER}; // lagrange order. [2-4] are good choices.
""".format(LAGRANGE_ORDER=lagrange_order) + dsp_code
	# print('dsp code: ')
	# print(dsp_code)

	faust_processor.set_dsp_string(dsp_code)
	faust_processor.compile()

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

	engine.load_graph(graph)

	render(engine, file_path=OUTPUT / output_path, duration=3.)

	# check that it's non-silent
	audio = engine.get_audio()
	assert(np.mean(np.abs(audio)) > .01)

def test_faust_poly_sampler_cymbal():
	# Load a stereo audio sample and pass it to Faust
	sample_seq = load_audio_file(ASSETS / "60988__folktelemetry__crash-fast-14.wav")
	_test_faust_poly_sampler(sample_seq, 'test_faust_poly_sampler_cymbal.wav', lagrange_order=4)
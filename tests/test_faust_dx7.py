from dawdreamer_utils import *


def _test_faust_dx7(algorithm=0, num_voices=8, buffer_size=2048):

	# There are 32 algorithms.

	engine = daw.RenderEngine(SAMPLE_RATE, buffer_size)

	faust_processor = engine.make_faust_processor("faust")
	faust_processor.num_voices = num_voices
	faust_processor.group_voices = True

	dsp_path = abspath(FAUST_DSP / "dx7.dsp")
	dsp_code = open(dsp_path).read()
	dsp_code = 'ALGORITHM = {0};\n'.format(algorithm) + dsp_code

	assert(faust_processor.set_dsp_string(dsp_code))
	assert(faust_processor.compile())

	desc = faust_processor.get_parameters_description()
	if algorithm == 0:
		for par in desc:
			print(par)

	for i, par in enumerate(desc):
		par_min = par['min']
		par_max = par['max']
		value = par_min + (par_max-par_min)*random.random()
		assert(faust_processor.set_automation(par['name'], np.array([value])))
		assert(faust_processor.set_parameter(par['index'], value))
		if random.random() < .5:
			assert(faust_processor.set_parameter(par['name'], value))

	 # (MIDI note, velocity, start sec, duration sec)
	faust_processor.add_midi_note(60, 60, 0.0, .25)
	faust_processor.add_midi_note(64, 80, 0.5, .5)
	faust_processor.add_midi_note(67, 127, 0.75, .5)

	assert(faust_processor.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	graph = [
	    (faust_processor, [])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path=OUTPUT / 'test_faust_dx7_algo_{0}.wav'.format(algorithm), duration=3.)

def test_faust_dx7():

	# just test 2 out of 32 to save some time
	for i in range(2):
		_test_faust_dx7(algorithm=i, num_voices=8, buffer_size=2048)

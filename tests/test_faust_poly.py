from utils import *

def _test_faust_poly(file_path, group_voices=True, num_voices=8, buffer_size=1, cutoff=None,
	automation=False, decay=None):

	engine = daw.RenderEngine(SAMPLE_RATE, buffer_size)

	dsp_path = abspath(FAUST_DSP / "polyphonic.dsp")
	faust_processor = engine.make_faust_processor("faust")
	assert(faust_processor.set_dsp(dsp_path))
	
	# Group voices will affect the number of parameters.
	# True will result in fewer parameters because all voices will
	# share the same parameters.
	faust_processor.group_voices = group_voices
	faust_processor.num_voices = num_voices
	
	assert(faust_processor.compile())

	# for par in faust_processor.get_parameters_description():
	# 	print(par)

	 # (MIDI note, velocity, start sec, duration sec)
	faust_processor.add_midi_note(60, 60, 0.0, .25)
	faust_processor.add_midi_note(64, 80, 0.5, .5)
	faust_processor.add_midi_note(67, 127, 0.75, .5)

	assert(faust_processor.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	if cutoff is not None:
		assert(faust_processor.set_parameter("/Sequencer/DSP2/MyInstrument/cutoff", cutoff))
	elif automation:
		assert(faust_processor.set_automation("/Sequencer/DSP2/MyInstrument/cutoff", 5000+4900*make_sine(30, 10.)))

	if decay is not None:
		if group_voices:
			assert(faust_processor.set_parameter("/Sequencer/DSP1/Polyphonic/Voices/MyInstrument/decay", decay))
		else:
			for i in range(1, num_voices+1):
				assert(faust_processor.set_parameter(f"/Sequencer/DSP1/Polyphonic/V{i}/MyInstrument/decay", decay))

	# for par in faust_processor.get_parameters_description():
	# 	print(par)

	graph = [
	    (faust_processor, [])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path=file_path, duration=3.)

	return engine.get_audio()

def test_faust_poly():
	audio1 = _test_faust_poly(OUTPUT / 'test_faust_poly_grouped.wav', group_voices=True)
	audio2 = _test_faust_poly(OUTPUT / 'test_faust_poly_ungrouped.wav', group_voices=False)

	assert(np.allclose(audio1, audio2))

	audio1 = _test_faust_poly(OUTPUT / 'test_faust_poly_2k_cutoff_grouped.wav', group_voices=True, cutoff=2000)
	audio2 = _test_faust_poly(OUTPUT / 'test_faust_poly_2k_cutoff_ungrouped.wav', group_voices=False, cutoff=2000)

	assert(np.allclose(audio1[:,:-1], audio2[:,:-1]))  # todo: don't drop last sample

	audio1 = _test_faust_poly(OUTPUT / 'test_faust_poly_automation_cutoff_grouped.wav', group_voices=True, automation=True)
	audio2 = _test_faust_poly(OUTPUT / 'test_faust_poly_automation_cutoff_ungrouped.wav', group_voices=False, automation=True)

	assert(np.allclose(audio1[:,:-1], audio2[:,:-1]))  # todo: don't drop last sample

	audio1 = _test_faust_poly(OUTPUT / 'test_faust_poly_automation_decay_grouped.wav', group_voices=True, decay=.5)
	audio2 = _test_faust_poly(OUTPUT / 'test_faust_poly_automation_decay_ungrouped.wav', group_voices=False, decay=.5)

	assert(np.allclose(audio1[:,:-1], audio2[:,:-1]))  # todo: don't drop last sample

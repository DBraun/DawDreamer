from utils import *

BUFFER_SIZE = 512

def test_sampler(set_data=False):

	def get_par_index(desc, par_name):
		for parDict in desc:
			if parDict['name'] == par_name:
				return parDict['index']
		raise ValueError(f"Parameter '{par_name}' not found.")

	DURATION = 1.5

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file(ASSETS / "60988__folktelemetry__crash-fast-14.wav")
	sampler_processor = engine.make_sampler_processor("playback", data)

	if set_data:
		sampler_processor.set_data(data)

	assert(sampler_processor.get_num_output_channels() == 2)

	desc = sampler_processor.get_parameters_description()
	# print(desc)

	sampler_processor.set_parameter(get_par_index(desc, 'Center Note'), 60.)  # set the center frequency to middle C (60)

	# (MIDI note, velocity, start sec, duration sec)
	sampler_processor.add_midi_note(60, 60, 0.0, .25)
	sampler_processor.add_midi_note(64, 80, 0.5, .5)
	sampler_processor.add_midi_note(67, 127, 0.75, .1)

	sampler_processor.set_parameter(get_par_index(desc, 'Amp Env Attack'), 1.)  # set attack in milliseconds
	sampler_processor.set_parameter(get_par_index(desc, 'Amp Env Decay'), 0.)  # set decay in milliseconds
	sampler_processor.set_parameter(get_par_index(desc, 'Amp Env Sustain'), 1.)  # set sustain
	sampler_processor.set_parameter(get_par_index(desc, 'Amp Env Release'), 100.)  # set release in milliseconds


	amp_index = get_par_index(desc, 'Amp Active')
	sampler_processor.set_parameter(amp_index, 1.)

	assert(sampler_processor.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	graph = [
	    (sampler_processor, [])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path=OUTPUT / 'test_sampler_with_amp.wav', duration=DURATION)

	sampler_processor.set_parameter(amp_index, 0.)

	assert(sampler_processor.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	render(engine, file_path=OUTPUT / 'test_sampler_without_amp.wav', duration=DURATION)
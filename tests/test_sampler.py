import pytest
from os.path import abspath
import platform

from utils import *
import dawdreamer as daw

BUFFER_SIZE = 4096*4

def test_sampler(set_data=False):

	DURATION = 1.5

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file("assets/60988__folktelemetry__crash-fast-14.wav")
	sampler_processor = engine.make_sampler_processor("playback", data)

	if set_data:
		sampler_processor.set_data(data)

	# (MIDI note, velocity, start sec, duration sec)
	sampler_processor.add_midi_note(60, 60, 0.0, .25)
	sampler_processor.add_midi_note(64, 80, 0.5, .5)
	sampler_processor.add_midi_note(67, 127, 0.75, .1)

	desc = sampler_processor.get_parameters_description()
	# print(desc)
	amp_index = [parDict['index'] for parDict in desc if parDict['name'] == 'Amp Active'][0]
	sampler_processor.set_parameter(amp_index, 1.)

	assert(sampler_processor.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	graph = [
	    (sampler_processor, [])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_sampler_with_amp.wav', duration=DURATION)

	sampler_processor.set_parameter(amp_index, 0.)

	assert(sampler_processor.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	render(engine, file_path='output/test_sampler_without_amp.wav', duration=DURATION)
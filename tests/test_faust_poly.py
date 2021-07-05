import pytest
from scipy.io import wavfile
from os.path import abspath

from utils import *
import dawdreamer as daw

BUFFER_SIZE = 1

def test_faust_poly(set_data=False):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	dsp_path = abspath("faust_dsp/polyphonic.dsp")
	faust_processor = engine.make_faust_processor("faust", "")
	faust_processor.num_voices = 8
	assert(faust_processor.set_dsp(dsp_path))
	assert(faust_processor.compiled)

	desc = faust_processor.get_parameters_description()
	for par in desc:
		print(par)

	 # (MIDI note, velocity, start sec, duration sec)
	faust_processor.add_midi_note(60, 60, 0.0, .25)
	faust_processor.add_midi_note(64, 80, 0.5, .5)
	faust_processor.add_midi_note(67, 127, 0.75, .5)

	assert(faust_processor.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

	# faust_processor.set_automation("/Sequencer/DSP2/MyInstrument/cutoff", 5000+4900*make_sine(10, 10.))

	graph = [
	    (faust_processor, [])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_faust_poly.wav', duration=3.)

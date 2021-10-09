from utils import *

BUFFER_SIZE = 1024

def _test_faust_soundfile(sample_seq, output_path):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	faust_processor = engine.make_faust_processor("faust")
	faust_processor.num_voices = 8

	dsp_path = abspath("faust_dsp/soundfile.dsp")

	faust_processor.set_data('mySound', sample_seq)

	assert(faust_processor.set_dsp(dsp_path))
	assert(faust_processor.compile())
	# desc = faust_processor.get_parameters_description()
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

def test_faust_soundfile_cymbal():
	# Load a stereo audio sample and pass it to Faust
	sample_seq = load_audio_file("assets/60988__folktelemetry__crash-fast-14.wav")
	_test_faust_soundfile(sample_seq, 'test_faust_soundfile.wav')
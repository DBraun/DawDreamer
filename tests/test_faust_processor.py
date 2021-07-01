import pytest
from scipy.io import wavfile
from os.path import abspath

from utils import *
import dawdreamer as daw

BUFFER_SIZE = 1

def test_faust_zita_rev1(set_data=False):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav")
	playback_processor = engine.make_playback_processor("playback", data)

	if set_data:
		playback_processor.set_data(data)

	dsp_path = abspath("faust_dsp/dm.zita_rev1.dsp")
	faust_processor = engine.make_faust_processor("faust", dsp_path)
	assert(faust_processor.set_dsp(dsp_path))

	print(faust_processor.get_parameters_description())

	graph = [
	    (playback_processor, []),
	    (faust_processor, ["playback"])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_faust_dm.zita_rev1.wav')

def test_faust_automation():

	DURATION = 5.1

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	drums = engine.make_playback_processor("drums",
		load_audio_file("assets/Music Delta - Disco/drums.wav", duration=DURATION))

	other = engine.make_playback_processor("other",
		load_audio_file("assets/Music Delta - Disco/other.wav", duration=DURATION))

	dsp_path = abspath("faust_dsp/two_stereo_inputs_filter.dsp")
	faust_processor = engine.make_faust_processor("faust", dsp_path)
	assert(faust_processor.set_dsp(dsp_path))

	print(faust_processor.get_parameters_description())

	faust_processor.set_parameter("/MyEffect/cutoff", 7000.0)  # Change the cutoff frequency.
	# or set automation like this
	faust_processor.set_automation("/MyEffect/cutoff", 10000+9000*make_sine(2, DURATION))

	graph = [
	    (drums, []),
	    (other, []),
	    (faust_processor, [drums.get_name(), other.get_name()])
	]

	assert(engine.load_graph(graph))

	render(engine, file_path='output/test_faust_automation.wav')

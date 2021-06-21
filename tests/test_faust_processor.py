import pytest
import librosa
import numpy as np
from scipy.io import wavfile
from os.path import abspath

import dawdreamer as daw

SAMPLE_RATE = 44100
BUFFER_SIZE = 1

def make_sine(freq: float, duration: float, sr=SAMPLE_RATE):
	"""Return sine wave based on freq in Hz and duration in seconds"""
	N = int(duration * sr) # Number of samples 
	return np.sin(np.pi*2.*freq*np.arange(N)/sr)

def load_audio_file(file_path, duration=None):
	sig, rate = librosa.load(file_path, duration=duration, mono=False, sr=SAMPLE_RATE)
	assert(rate == SAMPLE_RATE)
	return sig

def render(engine, file_path=None, duration=5.):

	engine.render(duration)

	output = engine.get_audio()

	if file_path is not None:

		wavfile.write(file_path, SAMPLE_RATE, output.transpose())

	return True


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

	engine.load_graph(graph)

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
	faust_processor.set_automation("/MyEffect/cutoff", 5000+4000*make_sine(2, DURATION))

	graph = [
	    (drums, []),
	    (other, []),
	    (faust_processor, ["drums", "other"])
	]

	engine.load_graph(graph)

	render(engine, file_path='output/test_faust_automation.wav')

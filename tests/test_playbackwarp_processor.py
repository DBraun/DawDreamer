import pytest
import librosa
import numpy as np
from scipy.io import wavfile
from os.path import abspath

from utils import *
import dawdreamer as daw

BUFFER_SIZE = 16

def test_faust_automation():

	DURATION = 10.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	engine.set_bpm(140.)

	drums = engine.make_playbackwarp_processor("drums",
		load_audio_file("assets/Music Delta - Disco/drums.wav", duration=DURATION))

	assert(drums.set_clip_file(abspath("assets/Music Delta - Disco/drums.wav.asd")))

	other = engine.make_playbackwarp_processor("other",
		load_audio_file("assets/Music Delta - Disco/other.wav", duration=DURATION))

	assert(other.set_clip_file(abspath("assets/Music Delta - Disco/other.wav.asd")))

	print('drums.start_marker: ', drums.start_marker)
	print('drums.end_marker: ', drums.end_marker)
	print('drums.loop_on: ', drums.loop_on)
	print('drums.loop_start: ', drums.loop_start)
	print('drums.loop_end: ', drums.loop_end)
	print('drums.warp_on: ', drums.warp_on)

	graph = [
	    (drums, []),
	    (other, []),
	    (engine.make_add_processor("add", [1., 1.]), ["drums", "other"])
	]

	engine.load_graph(graph)

	render(engine, file_path='output/test_playbackwarp_processor.wav')

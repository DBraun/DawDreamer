from utils import *

BUFFER_SIZE = 1

def test_playbackwarp_processor1():

	DURATION = 10.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	engine.set_bpm(140.)

	drums = engine.make_playbackwarp_processor("drums",
		load_audio_file(ASSETS / "Music Delta - Disco" / "drums.wav", duration=DURATION))

	assert(drums.set_clip_file(abspath(ASSETS / "Music Delta - Disco" / "drums.wav.asd")))

	other = engine.make_playbackwarp_processor("other",
		load_audio_file(ASSETS / "Music Delta - Disco" / "other.wav", duration=DURATION))

	assert(other.set_clip_file(abspath(ASSETS / "Music Delta - Disco" / "other.wav.asd")))

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

	assert(engine.load_graph(graph))

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor1a.wav')

	other.transpose = 2.

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor1b.wav')

	other.set_automation('transpose', make_sine(1., DURATION))

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor1c.wav')

def test_playbackwarp_processor2():

	DURATION = 10.

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	# Pick 120 because it's easy to analyze for timing in Audacity.
	# 1 second is two quarter notes.
	engine.set_bpm(120.)

	drums = engine.make_playbackwarp_processor("drums",
		load_audio_file(ASSETS / "Music Delta - Disco" / "drums.wav", duration=DURATION))

	assert(drums.set_clip_file(abspath(ASSETS / "Music Delta - Disco" / "drums.wav.asd")))

	drums.start_marker = 0.
	drums.loop_on = True
	assert(drums.loop_on)

	graph = [
	    (drums, []),
	]

	assert(engine.load_graph(graph))

	drums.set_clip_positions([[0., 4., 0.], [5., 9., 0.]])

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor2a.wav')

	drums.set_clip_positions([[0., 4., 0.]])
	drums.start_marker = 0.
	drums.loop_start = 1.
	drums.loop_end = 2.

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor2b.wav')

	drums.start_marker = 0.
	drums.loop_start = 3.
	drums.loop_end = 4.
	drums.set_clip_positions([[0., 2., 0.], [3., 9., 3.]])

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor2c.wav')
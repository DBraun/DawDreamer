from dawdreamer_utils import *


@pytest.mark.parametrize("buffer_size", [1, 2048])
def test_playbackwarp_processor1(buffer_size: int):

	DURATION = 15.

	engine = daw.RenderEngine(SAMPLE_RATE, buffer_size)

	ppqn = 960

	full_measure = ppqn * 4  # four beats is a measure
	bpm = np.concatenate([140*np.ones(full_measure), 70.*np.ones(full_measure)])
	bpm = np.tile(bpm, (100))
	engine.set_bpm(bpm, ppqn=ppqn)

	drums = engine.make_playbackwarp_processor("drums",
		load_audio_file(ASSETS / "Music Delta - Disco" / "drums.wav", duration=DURATION))

	assert(drums.set_clip_file(abspath(ASSETS / "Music Delta - Disco" / "drums.wav.asd")))

	other = engine.make_playbackwarp_processor("other",
		load_audio_file(ASSETS / "Music Delta - Disco" / "other.wav", duration=DURATION))

	other.set_clip_file(abspath(ASSETS / "Music Delta - Disco" / "other.wav.asd"))

	print('drums.start_marker: ', drums.start_marker)
	print('drums.end_marker: ', drums.end_marker)
	print('drums.loop_on: ', drums.loop_on)
	print('drums.loop_start: ', drums.loop_start)
	print('drums.loop_end: ', drums.loop_end)
	print('drums.warp_on: ', drums.warp_on)

	warp_markers = drums.warp_markers

	# re-assign and test that it stayed the same
	drums.warp_markers = warp_markers
	assert (drums.warp_markers == warp_markers).all()

	# add one interpolated warp marker in the middle
	warp1 = warp_markers[0]
	warp3 = warp_markers[-1]
	warp2 = 0.5*(warp1+warp3)

	warp_markers = np.stack([warp1, warp2, warp3], axis=0)

	assert warp_markers.shape[0] == 3
	assert warp_markers.shape[1] == 2

	drums.warp_markers = warp_markers

	assert (drums.warp_markers == warp_markers).all()

	graph = [
	    (drums, []),
	    (other, []),
	    (engine.make_add_processor("add", [1., 1.]), ["drums", "other"])
	]

	engine.load_graph(graph)
	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor1a.wav', duration=DURATION)

	assert(np.mean(np.abs(engine.get_audio())) > .01)

	other.transpose = 2.
	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor1b.wav', duration=DURATION)

	assert(np.mean(np.abs(engine.get_audio())) > .01)

	other.set_automation('transpose', make_sine(1., DURATION))
	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor1c.wav', duration=DURATION)

	assert(np.mean(np.abs(engine.get_audio())) > .01)

@pytest.mark.parametrize("buffer_size", [1])
def test_playbackwarp_processor2(buffer_size: int):

	DURATION = 10.

	engine = daw.RenderEngine(SAMPLE_RATE, buffer_size)

	# Pick 120 because it's easy to analyze for timing in Audacity.
	# 1 second is two quarter notes.
	engine.set_bpm(120.)

	drums = engine.make_playbackwarp_processor("drums",
		load_audio_file(ASSETS / "Music Delta - Disco" / "drums.wav", duration=DURATION))

	drums.set_clip_file(abspath(ASSETS / "Music Delta - Disco" / "drums.wav.asd"))

	drums.start_marker = 0.
	drums.loop_on = True
	assert(drums.loop_on)

	graph = [
	    (drums, []),
	]

	engine.load_graph(graph)

	drums.set_clip_positions([[0., 4., 0.], [5., 9., 0.]])

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor2a.wav')

	assert(np.mean(np.abs(engine.get_audio())) > .01)

	drums.set_clip_positions([[0., 4., 0.]])
	drums.start_marker = 0.
	drums.loop_start = 1.
	drums.loop_end = 2.

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor2b.wav')

	assert(np.mean(np.abs(engine.get_audio())) > .01)

	drums.start_marker = 0.
	drums.loop_start = 3.
	drums.loop_end = 4.
	drums.set_clip_positions([[0., 2., 0.], [3., 9., 3.]])

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor2c.wav')

	assert(np.mean(np.abs(engine.get_audio())) > .01)

@pytest.mark.parametrize("buffer_size", [1])
def test_playbackwarp_processor3(buffer_size: int):

	"""
	Test using the playback warp processor without a clip file and therefore without warping.
	The BPM of the engine should not affect the output audio.
	Looping should work.
	The time ratio should have an effect.
	"""

	DURATION = 3.

	engine = daw.RenderEngine(SAMPLE_RATE, buffer_size)

	drum_audio = load_audio_file(ASSETS / "575854__yellowtree__d-b-funk-loop.wav")
	drum_audio = drum_audio[:, int(SAMPLE_RATE*.267):]  # manually trim to beginning

	drums = engine.make_playbackwarp_processor("drums", drum_audio)

	drums.time_ratio = .7
	assert drums.time_ratio == .7

	drums.loop_on = True
	assert(drums.loop_on)

	assert not drums.warp_on

	actual_bpm = 108.
	num_beats = 2.
	drums.loop_end = num_beats*60./actual_bpm

	graph = [
	    (drums, []),
	]

	engine.load_graph(graph)

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor3a.wav', duration=DURATION)

	audio = engine.get_audio()

	assert audio.ndim == 2

	num_samples = audio.shape[1]

	assert num_samples > 100

	assert(np.mean(np.abs(audio)) > .01)

@pytest.mark.parametrize("buffer_size", [1])
def test_playbackwarp_processor4(buffer_size: int):

	"""
	Test using the playback warp processor without a clip file and therefore without warping.
	The BPM of the engine should not affect the output audio.
	Test that `reset_warp_markers` works with a BPM argument.
	"""

	DURATION = 10.

	engine = daw.RenderEngine(SAMPLE_RATE, buffer_size)

	drum_audio = load_audio_file(ASSETS / "Music Delta - Disco" / "drums.wav")
	drum_audio = drum_audio[:, int(SAMPLE_RATE*.267):]  # manually trim to beginning

	drums = engine.make_playbackwarp_processor("drums", drum_audio)

	assert drums.time_ratio == 1.

	drums.loop_on = True
	assert(drums.loop_on)

	assert not drums.warp_on

	actual_bpm = 110.
	num_beats = 2.

	drums.reset_warp_markers(actual_bpm)
	drums.loop_end = num_beats

	graph = [
	    (drums, []),
	]

	engine.load_graph(graph)

	render(engine, file_path=OUTPUT / 'test_playbackwarp_processor4a.wav', duration=DURATION)

	audio = engine.get_audio()

	assert audio.ndim == 2

	num_samples = audio.shape[1]

	assert num_samples > 100

	assert(np.mean(np.abs(audio)) > .01)

@pytest.mark.parametrize("warp_on", [True, False])
def test_playbackwarp_sample_rate_diff(warp_on: bool):

	"""
	Use the playback warp processor on source audio with a high sample rate (96 kHz).
	Whether or not warp_on is on, the output should sound like the input.
	"""

	if not USE_LIBROSA:
		return

	DURATION = 10.

	engine = daw.RenderEngine(SAMPLE_RATE, 128)

	engine.set_bpm(110.)  # we know that this file is 110 BPM.

	upsampled_rate = 96_000
	# nb: sr=None makes it load at the native sample rate (96000 for this file)
	audio, rate = librosa.load(abspath(ASSETS / "Music Delta - Disco" / "drums_96kHz.wav"),
		duration=DURATION, mono=False, sr=None)
	assert rate == upsampled_rate

	drums = engine.make_playbackwarp_processor("drums", audio, sr=upsampled_rate)

	drums.set_clip_file(abspath(ASSETS / "Music Delta - Disco" / "drums.wav.asd"))

	drums.warp_on = warp_on
	drums.start_marker = 0.
	drums.loop_on = True
	assert(drums.loop_on)

	graph = [
	    (drums, []),
	]

	engine.load_graph(graph)

	warp_str = '_warp_on' if warp_on else ''
	render(engine, file_path=OUTPUT / f'test_playbackwarp_sample_rate_diff{warp_str}.wav')

from utils import *

BUFFER_SIZE = 1024

def _test_faust_soundfile(sample_seq, output_path, sound_choice=0):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	faust_processor = engine.make_faust_processor("faust")
	faust_processor.num_voices = 8

	dsp_path = abspath("faust_dsp/soundfile.dsp")

	if sample_seq.ndim == 1:
		sample_seq = sample_seq.reshape(1, -1)

	reversed_audio = np.flip(sample_seq[:,:int(44100*.5)], axis=-1)

	# set_soundfiles
	soundfiles = {
		'mySound': [sample_seq, reversed_audio, sample_seq]
	}
	faust_processor.set_soundfiles(soundfiles)

	assert(faust_processor.set_dsp(dsp_path))
	assert(faust_processor.compile())
	# desc = faust_processor.get_parameters_description()
	# for par in desc:
	# 	print(par)

	faust_processor.set_parameter('/Sequencer/DSP1/Polyphonic/Voices/MyInstrument/soundChoice', sound_choice)

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

	audio = engine.get_audio()
	assert(np.mean(np.abs(audio)) > .01)

def _test_faust_soundfile_multichannel(output_path):

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	faust_processor = engine.make_faust_processor("faust")

	dsp_path = abspath("faust_dsp/soundfile.dsp")

	numChannels = 9

	sample_seq = np.sin(np.linspace(0, 4000, num=int(44100*5.0)))
	sample_seq = np.stack([sample_seq for _ in range(numChannels)])

	# set_soundfiles
	soundfiles = {
		'mySound': [sample_seq]
	}
	faust_processor.set_soundfiles(soundfiles)

	underscores = ",".join('_'*numChannels)
	dsp_string = f'process = 0,_~+(1):soundfile("mySound",{numChannels}):!,!,{underscores};'
	assert(faust_processor.set_dsp_string(dsp_string))
	assert(faust_processor.compile())
	# desc = faust_processor.get_parameters_description()
	# for par in desc:
	# 	print(par)

	graph = [
	    (faust_processor, [])
	]

	assert(engine.load_graph(graph))
	render(engine, file_path='output/'+output_path, duration=3.)

	audio = engine.get_audio()
	assert(np.mean(np.abs(audio)) > .01)

def download_grand_piano():

	"""Download the dataset if it's missing"""

	try:

		file_paths = [f"assets/bitKlavierGrand_PianoBar/{i}v8.wav" for i in range(88)]

		import os.path

		if os.path.isfile(file_paths[0]):
			return

		bitKlavierURL = 'https://ccrma.stanford.edu/~braun/assets/bitKlavierGrand_PianoBar.zip'
		import requests
		 
		# download the file contents in binary format
		print(f'Downloading: {bitKlavierURL}')
		r = requests.get(bitKlavierURL)

		path_to_zip_file = abspath("assets/bitKlavierGrand_PianoBar.zip")
		with open(path_to_zip_file, "wb") as zip:
		    zip.write(r.content)

		import zipfile
		with zipfile.ZipFile(path_to_zip_file, 'r') as zip_ref:
		    zip_ref.extractall("assets")

		os.remove(path_to_zip_file)

	except Exception as e:
		print('Something went wrong downloading the bitKlavier Grand Piano data.')
		raise e


def test_faust_soundfile_piano():

	download_grand_piano()

	engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

	faust_processor = engine.make_faust_processor("faust")
	faust_processor.num_voices = 16
	faust_processor.group_voices = True

	dsp_path = abspath("faust_dsp/soundfile_piano.dsp")

	# set_soundfiles
	soundfiles = {
		'mySound': [load_audio_file(f"assets/bitKlavierGrand_PianoBar/{i}v8.wav") for i in range(88)]
	}
	faust_processor.set_soundfiles(soundfiles)

	assert(faust_processor.set_dsp(dsp_path))
	assert(faust_processor.compile())
	# desc = faust_processor.get_parameters_description()
	# for par in desc:
	# 	print(par)

	midi_path = 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'
	faust_processor.load_midi(abspath(f'assets/{midi_path}'))

	graph = [
	    (faust_processor, [])
	]

	assert(engine.load_graph(graph))
	render(engine, file_path='output/test_sound_file_piano.wav', duration=10.)

	audio = engine.get_audio()
	assert(np.mean(np.abs(audio)) > .0001)


def test_faust_soundfile_cymbal():
	# Load a stereo audio sample and pass it to Faust
	sample_seq = load_audio_file("assets/60988__folktelemetry__crash-fast-14.wav")
	_test_faust_soundfile(sample_seq, 'test_faust_soundfile_0.wav', sound_choice=0)
	_test_faust_soundfile(sample_seq, 'test_faust_soundfile_1.wav', sound_choice=1)
	_test_faust_soundfile(sample_seq, 'test_faust_soundfile_2.wav', sound_choice=2)
	_test_faust_soundfile_multichannel('test_faust_soundfile_3.wav')
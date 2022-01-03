
import pytest
from scipy.io import wavfile
from os.path import abspath, isfile
import numpy as np
import pathlib
import random

import dawdreamer as daw

SAMPLE_RATE = 44100

def make_sine(freq: float, duration: float, sr=SAMPLE_RATE):
	"""Return sine wave based on freq in Hz and duration in seconds"""
	N = int(duration * sr) # Number of samples 
	return np.sin(np.pi*2.*freq*np.arange(N)/sr)

def load_audio_file(file_path, duration=None):

	try:
		import librosa

		sig, rate = librosa.load(file_path, duration=duration, mono=False, sr=SAMPLE_RATE)
		assert(rate == SAMPLE_RATE)
		
	except ModuleNotFoundError as e:
		import soundfile
		# todo: soundfile doesn't allow you to specify the duration or sample rate, unless the file is RAW
		sig, rate = soundfile.read(file_path, always_2d=True)
		sig = sig.T

	return sig


def render(engine, file_path=None, duration=5.):

	engine.render(duration)

	output = engine.get_audio()

	if file_path is not None:

		wavfile.write(file_path, SAMPLE_RATE, output.transpose())

	return True

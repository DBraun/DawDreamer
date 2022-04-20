from collections import defaultdict
from os import getenv
from os.path import abspath, isfile, isdir, basename, splitext
from pathlib import Path
import pathlib
import platform
import random
import warnings

import numpy as np
import pytest
from scipy.io import wavfile

USE_LIBROSA = True
try:
	import librosa
except ModuleNotFoundError as e:
	import soundfile
	USE_LIBROSA = False

import dawdreamer as daw

SAMPLE_RATE = 44100

DIR = Path(__file__).parent.resolve()
ASSETS = DIR / 'assets'
FAUST_DSP = DIR / 'faust_dsp'
PLUGINS = DIR / 'plugins'
OUTPUT = DIR / 'output'

def make_sine(freq: float, duration: float, sr=SAMPLE_RATE):
	"""Return sine wave based on freq in Hz and duration in seconds"""
	N = int(duration * sr) # Number of samples 
	return np.sin(np.pi*2.*freq*np.arange(N)/sr)

def load_audio_file(file_path, duration=None):

	if USE_LIBROSA:

		sig, rate = librosa.load(file_path, duration=duration, mono=False, sr=SAMPLE_RATE)
		assert(rate == SAMPLE_RATE)
	
	else:
		# todo: soundfile doesn't allow you to specify the duration or sample rate, unless the file is RAW
		sig, rate = soundfile.read(file_path, always_2d=True)
		sig = sig.T

	return sig


def render(engine, file_path=None, duration=5.):

	assert(engine.render(duration))

	output = engine.get_audio()

	if file_path is not None:

		wavfile.write(file_path, SAMPLE_RATE, output.transpose())


def append_if_exists(some_list, filepath):

    filepath = abspath(filepath)
    filepath = filepath.replace("\\", "/")

    if platform.system() == 'Darwin' and splitext(filepath)[-1] in [".component", ".vst3", ".vst"]:
        # macOS treats .component .vst and .vst3 as directories
        if isdir(filepath):
            some_list.append(filepath)
    elif isfile(filepath):
        some_list.append(filepath)


def is_pytesting():
	return bool(getenv("PYTEST_CURRENT_TEST"))


ALL_PLUGIN_INSTRUMENTS = []
ALL_PLUGIN_EFFECTS = []

if platform.system() == "Darwin":

    # append_if_exists(ALL_PLUGIN_EFFECTS, PLUGINS / "DimensionExpander.component")   # todo: enable
    # append_if_exists(ALL_PLUGIN_EFFECTS, PLUGINS / "DimensionExpander.vst")         # todo: enable

    append_if_exists(ALL_PLUGIN_EFFECTS, PLUGINS / "RoughRider3.vst")
    append_if_exists(ALL_PLUGIN_EFFECTS, PLUGINS / "RoughRider3.vst3")
    append_if_exists(ALL_PLUGIN_EFFECTS, PLUGINS / "RoughRider3.component")

    # todo: the Valhalla Freq Echo plugins sometimes work and sometimes just output NAN.
    # append_if_exists(ALL_PLUGIN_EFFECTS, PLUGINS / "ValhallaFreqEcho.vst")
    # append_if_exists(ALL_PLUGIN_EFFECTS, PLUGINS / "ValhallaFreqEcho.vst3")
    append_if_exists(ALL_PLUGIN_EFFECTS, PLUGINS / "ValhallaFreqEcho.component")

    append_if_exists(ALL_PLUGIN_INSTRUMENTS, PLUGINS / "TAL-NoiseMaker.vst")
    append_if_exists(ALL_PLUGIN_INSTRUMENTS, PLUGINS / "TAL-NoiseMaker.vst3")
    # append_if_exists(ALL_PLUGIN_INSTRUMENTS, PLUGINS / "TAL-NoiseMaker.component")  # todo: enable
    append_if_exists(ALL_PLUGIN_INSTRUMENTS, "/Library/Audio/Plug-Ins/VST3/Dexed.vst3")  # todo: only DBraun has this
    # append_if_exists(ALL_PLUGIN_INSTRUMENTS, "/Library/Audio/Plug-Ins/Components/Dexed.component")  # todo: enable, only DBraun has this

    # append_if_exists(ALL_PLUGIN_INSTRUMENTS, "/Library/Audio/Plug-Ins/Components/helm.component")  # todo: enable, only DBraun has this
    # append_if_exists(ALL_PLUGIN_INSTRUMENTS, "/Library/Audio/Plug-Ins/VST/helm.vst")  # todo: enable, only DBraun has this
    # append_if_exists(ALL_PLUGIN_INSTRUMENTS, "/Library/Audio/Plug-Ins/VST3/helm.vst3")  # todo: enable, only DBraun has this
    
elif platform.system() == "Windows":
    append_if_exists(ALL_PLUGIN_EFFECTS, PLUGINS / "Dimension Expander_x64.dll")
    append_if_exists(ALL_PLUGIN_EFFECTS, "C:/VSTPlugIns/Bloom.dll")                   # todo: only DBraun has this
    append_if_exists(ALL_PLUGIN_EFFECTS, "C:/VSTPlugIns/CamelCrusher.dll")            # todo: only DBraun has this
    append_if_exists(ALL_PLUGIN_EFFECTS, "C:/VSTPlugIns/CHOWTapeModel.dll")           # todo: only DBraun has this
    append_if_exists(ALL_PLUGIN_EFFECTS, "C:/VSTPlugIns/Szechuan Saturator.vst3")     # todo: only DBraun has this
    append_if_exists(ALL_PLUGIN_EFFECTS, "C:/VSTPlugIns/TAL-Chorus-LX.vst3")          # todo: only DBraun has this
    append_if_exists(ALL_PLUGIN_EFFECTS, "C:/VSTPlugIns/TAL-Chorus-LX-64.dll")        # todo: only DBraun has this

    append_if_exists(ALL_PLUGIN_INSTRUMENTS, PLUGINS / "TAL-NoiseMaker-64.dll")
    append_if_exists(ALL_PLUGIN_INSTRUMENTS, "C:/VSTPlugIns/Dexed.dll")               # todo: only DBraun has this
    append_if_exists(ALL_PLUGIN_INSTRUMENTS, "C:/VSTPlugIns/Serum_x64.dll")           # todo: only DBraun has this
    append_if_exists(ALL_PLUGIN_INSTRUMENTS, "C:/VSTPlugIns/Surge.vst3")              # todo: only DBraun has this
    append_if_exists(ALL_PLUGIN_INSTRUMENTS, "C:/VSTPlugIns/TAL-NoiseMaker-64.vst3")  # todo: only DBraun has this

else:
    # todo: test LV2 plugins on Linux
    pass

if getenv("CIBW_TEST_REQUIRES"):
    # Skip .component plugins on GitHub Actions workflows
    func = lambda plugin_path: not plugin_path.endswith('.component')
    ALL_PLUGIN_EFFECTS     = list(filter(func, ALL_PLUGIN_EFFECTS))
    ALL_PLUGIN_INSTRUMENTS = list(filter(func, ALL_PLUGIN_INSTRUMENTS))


PLUGIN_INPUT_CHANNELS = defaultdict(lambda: 2)
PLUGIN_INST_INPUT_CHANNELS = defaultdict(lambda: 0)
PLUGIN_OUTPUT_CHANNELS = defaultdict(lambda: 2)

PLUGIN_INPUT_CHANNELS["RoughRider3"] = 3  # RoughRider has an optional mono sidechain input.
PLUGIN_INST_INPUT_CHANNELS["TAL-NoiseMaker-64"] = 2
PLUGIN_INST_INPUT_CHANNELS["TAL-NoiseMaker"] = 2
PLUGIN_INST_INPUT_CHANNELS["Surge"] = 2
PLUGIN_OUTPUT_CHANNELS["Surge"] = 6

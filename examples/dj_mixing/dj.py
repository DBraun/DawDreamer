import dawdreamer as daw

from pathlib import Path

import librosa
import numpy as np
import pytest
from scipy.io import wavfile

SAMPLE_RATE = 44100
PPQN = 960*4

DIR = Path(__file__).parent.resolve()
OUTPUT = DIR / 'output'


def keyframe(start: float, stop: float, beats: float):
	return np.linspace(start, stop, num=int(PPQN*beats), endpoint=True)

def automate(*args):
	return np.concatenate([keyframe(*triplet) for triplet in args])


class MyEngine(daw.RenderEngine):

	def __init__(self, sr):
		super(MyEngine, self).__init__(sr, 1)

		self.playback1 = self.make_playbackwarp_processor("playback1", np.zeros((2, 0)))
		self.playback2 = self.make_playbackwarp_processor("playback2", np.zeros((2, 0)))

		mixer = self.make_faust_processor("mixer")

		mixer.set_dsp_string(f"""
			declare name "MyMixer";
			import("stdfaust.lib");
			mixer_slider = hslider("Mixer", 0., 0., 1., 0.0001) : si.smoo;
			mixer = _, _ : it.interpolate_linear(mixer_slider);
			stereo_mixer = par(i, 2, mixer);

			process = ro.interleave(2,2) : stereo_mixer :> si.bus(2);
			""")
		# print(mixer.get_parameters_description())

		self.mixer = mixer

		graph = [
			(self.playback1, []),
			(self.playback2, []),
			(self.mixer, ["playback1", "playback2"])
		]

		self.load_graph(graph)

	def load_audio(self, i: int, audio_path: str, duration=None):

		audio_path = str(audio_path)

		audio, rate = librosa.load(audio_path, duration=duration, mono=False, sr=None)

		if i == 0:
			playback = self.playback1
		else:
			playback = self.playback2

		playback.set_data(audio, sr=rate)
		playback.set_clip_file(audio_path + ".asd")

	def render(self, duration: float, file_path=None):

		super(MyEngine, self).render(duration, convert_to_sec=False)

		output = self.get_audio()

		if file_path is not None:

			wavfile.write(file_path, SAMPLE_RATE, output.transpose())

		return output

def main():

	# Put your own paths and BPM here	
	audio1_path = ""
	audio2_path = ""
	bpm1 = 120
	bpm2 = 126
	# bpm2 = bpm1 = (bpm1 + bpm2)*.5 # debug with this

	if audio1_path == '' or audio2_path == '':
		raise ValueError("You must customize the audio paths.")

	engine = MyEngine(SAMPLE_RATE)

	bpm_automation = automate(
		(bpm1, bpm1, 8),
		(bpm1, bpm2, 32),
		(bpm2, bpm2, 8),
		(bpm2, bpm1, 32),
		(bpm1, bpm1, 8),
		)

	engine.set_bpm(bpm_automation, ppqn=PPQN)

	mixer_automation = automate(
		(0, 0, 8),
		(0, 1, 32),
		(1, 1, 8),
		(1, 0, 32),
		(0, 0, 8),
		)

	engine.mixer.set_automation('/MyMixer/Mixer', mixer_automation, ppqn=PPQN)
	# engine.mixer.set_parameter('/MyMixer/Mixer', 0.5)  # debug with this
	# engine.mixer.set_parameter('/MyMixer/Mixer', 0.)  # debug with this
	# engine.mixer.set_parameter('/MyMixer/Mixer', 1.)  # debug with this

	engine.load_audio(0, audio1_path)
	engine.load_audio(1, audio2_path)

	num_beats = 88.
	engine.render(num_beats, file_path=OUTPUT / 'dj_transition_output.wav')
	# It's possible to make more calls to load_audio and re-do automation


if __name__ == "__main__":
	main()
	print('All done!')

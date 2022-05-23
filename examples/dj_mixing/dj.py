import dawdreamer as daw

from pathlib import Path
from os.path import isfile
import librosa
import numpy as np
import pytest
from scipy.io import wavfile

SAMPLE_RATE = 44100
PPQN = 960*4

DIR = Path(__file__).parent.resolve()
OUTPUT = DIR / 'output'


def keyframe(start: float, stop: float, beats: float, smooth=True):

    def smoothclamp(x, mi, mx):
        return mi + (mx-mi)*(lambda t: np.where(t < 0 , 0, np.where( t <= 1 , 3*t**2-2*t**3, 1 ) ) )( (x-mi)/(mx-mi) )

    y = np.linspace(0, 1, num=int(PPQN*beats), endpoint=True)
    if smooth:
        y = smoothclamp(y, 0., 1.)  # any easing function can go here
    y = start + (stop-start)*y
    return y

def automate(*args):
    return np.concatenate([keyframe(*triplet) for triplet in args])


class MyEngine(daw.RenderEngine):

    def __init__(self, sr):
        super(MyEngine, self).__init__(sr, 1)

        self.playback1 = self.make_playbackwarp_processor("playback1", np.zeros((2, 0)))
        self.playback2 = self.make_playbackwarp_processor("playback2", np.zeros((2, 0)))

        # Optionally enable recording on the individual playbacks
        # self.playback1.record = self.playback2.record = True

        mixer = self.make_faust_processor("mixer")

        # NB: Check out this code in the Faust IDE: https://faustide.grame.fr/
        mixer.set_dsp_string(f"""
            declare name "MyMixer";
            import("stdfaust.lib");
            mixer_slider = hslider("Mixer", 0., 0., 1., 0.0001) : si.smoo;
            mixer = _, _ : it.interpolate_linear(mixer_slider);
            process = ro.interleave(2,2) : par(i, 2, mixer);
            """)
        # print(mixer.get_parameters_description())

        self.mixer = mixer

        graph = [
            (self.playback1, []),
            (self.playback2, []),
            (self.mixer, ["playback1", "playback2"])
        ]

        self.load_graph(graph)

    def load_audio(self, i: int, audio_path: str, bpm=120., start_sec=0.):

        """
        Load an audio file into a playback warp processor. If the `.asd` file doesn't
        exist, you should specify a BPM kwarg to fall-back on. The `start_sec` kwarg
        should also be the time in seconds of the first down-beat.
        """

        audio_path = str(audio_path)

        audio, rate = librosa.load(audio_path, duration=None, mono=False, sr=None)

        if i == 0:
            playback = self.playback1
        else:
            playback = self.playback2

        # The PlaybackWarp Processor can take audio of higher sample rate,
        # as long as you specify it with the sr kwarg.
        playback.set_data(audio, sr=rate)
        asd_path = audio_path + ".asd"
        if isfile(asd_path):
            playback.set_clip_file(asd_path)
        else:
            print('Warning: ASD file not found: ', asd_path)

            # If you don't have warp markers, you should turn warping on.
            playback.warp_on = True
            playback.loop_on = False  # todo: more intelligently set this
            playback.loop_start = 0
            playback.loop_end = 8192  # todo: more intelligently set this
            playback.start_marker = 0
            playback.end_marker = 8192  # todo: more intelligently set this

            beats_in_one_sec_audio = bpm/60.
            playback.warp_markers = np.array([[start_sec, 0.], [start_sec+1., beats_in_one_sec_audio]], dtype=np.float32)

    def render(self, duration: float, file_path=None):

        super(MyEngine, self).render(duration, beats=True)

        output = self.get_audio()

        # If before the render we had enabled recording of the playbacks.
        # playback1_audio = self.playback1.get_audio()
        # playback2_audio = self.playback2.get_audio()

        if file_path is not None:

            wavfile.write(file_path, SAMPLE_RATE, output.transpose())

        return output


def main():

    # Put your own paths, BPM, and start seconds here
    audio1_path = ""
    audio2_path = ""
    bpm1 = 128
    bpm2 = 132
    start_sec1 = 0  # the time in seconds of the first downbeat
    start_sec2 = 0  # the time in seconds of the first downbeat
    # bpm1 = bpm2 = (bpm1 + bpm2)*.5 # Debug with this

    if audio1_path == '' or audio2_path == '':
        raise ValueError("You must customize the audio paths.")

    engine = MyEngine(SAMPLE_RATE)

    bpm_automation = automate(
        (bpm1, bpm1, 8),  # Hold bpm1 for 8 beats
        (bpm1, bpm2, 32), # Transition from bpm1 to bpm2 over 32 beats
        (bpm2, bpm2, 8),  # Hold bpm2 for 8 beats
        (bpm2, bpm1, 32), # Transition from bpm2 to bpm1 over 32 beats
        (bpm1, bpm1, 8),  # Hold bpm1 for 8 beats
        )

    engine.set_bpm(bpm_automation, ppqn=PPQN)

    mixer_automation = automate(
        (0, 0, 8),  # Listen to the first audio for 8 beats
        (0, 1, 32), # Fade to the second audio over 32 beats
        (1, 1, 8),  # Listen to the second audio for 8 beats
        (1, 0, 32), # Fade to the first audio over 32 beats
        (0, 0, 8),  # Listen to the first audio for 8 beats
        )

    engine.mixer.set_automation('/MyMixer/Mixer', mixer_automation, ppqn=PPQN)
    # engine.mixer.set_parameter('/MyMixer/Mixer', 0.5)  # Listen with even mixing.
    # engine.mixer.set_parameter('/MyMixer/Mixer', 0.)  # Listen to the first audio
    # engine.mixer.set_parameter('/MyMixer/Mixer', 1.)  # Listen to the second audio

    # Load the audio files into the playback processors.
    engine.load_audio(0, audio1_path, bpm=bpm1, start_sec=start_sec1)
    engine.load_audio(1, audio2_path, bpm=bpm2, start_sec=start_sec2)

    # Start the clips at 0 beats and have them play for 8192 beats.
    # engine.playback1.set_clip_positions([[0., 8192., 0.]])
    # engine.playback2.set_clip_positions([[0., 8192., 0.]])

    num_beats = 88.
    engine.render(num_beats, file_path=OUTPUT / 'dj_transition_output.wav')
    # It's possible to make more calls to load_audio, re-do automation, and re-render.
    # The rest is up to you!

if __name__ == "__main__":
    main()
    print('All done!')

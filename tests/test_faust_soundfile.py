from dawdreamer_utils import *

BUFFER_SIZE = 1024


# # Load a stereo audio sample and pass it to Faust
@pytest.mark.parametrize(
    "audio_path,output_path,sound_choice",
    [
        (ASSETS / "60988__folktelemetry__crash-fast-14.wav", "test_faust_soundfile_0.wav", 0),
        (ASSETS / "60988__folktelemetry__crash-fast-14.wav", "test_faust_soundfile_1.wav", 1),
        (ASSETS / "60988__folktelemetry__crash-fast-14.wav", "test_faust_soundfile_2.wav", 2),
    ],
)
def test_faust_soundfile(audio_path: str, output_path, sound_choice):
    sample_seq = load_audio_file(str(audio_path))

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    faust_processor = engine.make_faust_processor("faust")
    faust_processor.num_voices = 8

    dsp_path = abspath(FAUST_DSP / "soundfile.dsp")

    if sample_seq.ndim == 1:
        sample_seq = sample_seq.reshape(1, -1)

    reversed_audio = np.flip(sample_seq[:, : int(44100 * 0.5)], axis=-1)

    # set_soundfiles
    soundfiles = {"mySound": [sample_seq, reversed_audio, sample_seq]}
    faust_processor.set_soundfiles(soundfiles)

    faust_processor.set_dsp(dsp_path)
    faust_processor.compile()
    # desc = faust_processor.get_parameters_description()
    # for par in desc:
    #   print(par)

    faust_processor.set_parameter(
        "/Sequencer/DSP1/Polyphonic/Voices/MyInstrument/soundChoice", sound_choice
    )

    # (MIDI note, velocity, start sec, duration sec)
    faust_processor.add_midi_note(60, 60, 0.0, 0.25)
    faust_processor.add_midi_note(64, 80, 0.5, 0.5)
    faust_processor.add_midi_note(67, 127, 0.75, 0.5)

    assert faust_processor.n_midi_events == 3 * 2  # multiply by 2 because of the off-notes.

    engine.load_graph([(faust_processor, [])])
    render(engine, file_path=OUTPUT / output_path, duration=3.0)

    audio = engine.get_audio()
    assert np.mean(np.abs(audio)) > 0.01


@pytest.mark.parametrize("output_path", ["test_faust_soundfile_multichannel.wav"])
def test_faust_soundfile_multichannel(output_path):
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    faust_processor = engine.make_faust_processor("faust")

    abspath(FAUST_DSP / "soundfile.dsp")

    numChannels = 9

    sample_seq = np.sin(np.linspace(0, 4000, num=int(44100 * 5.0)))
    sample_seq = np.stack([sample_seq for _ in range(numChannels)])

    # set_soundfiles
    soundfiles = {"mySound": [sample_seq]}
    faust_processor.set_soundfiles(soundfiles)

    dsp_string = f'process = 0,_~+(1):soundfile("mySound",{numChannels}):!,!,si.bus({numChannels});'
    faust_processor.set_dsp_string(dsp_string)
    faust_processor.compile()
    # desc = faust_processor.get_parameters_description()
    # for par in desc:
    #   print(par)

    engine.load_graph([(faust_processor, [])])
    render(engine, file_path=OUTPUT / output_path, duration=3.0)

    audio = engine.get_audio()
    assert np.mean(np.abs(audio)) > 0.01


def generate_piano_sample(key_index: int, duration: float = 3.0) -> np.ndarray:
    """Generate a synthetic piano-like sample for a given piano key (0-87).

    Args:
        key_index: Piano key index (0 = A0, 87 = C8)
        duration: Duration in seconds

    Returns:
        Stereo audio array with shape (2, num_samples)
    """
    # Piano key frequencies: A0 (27.5 Hz) to C8 (4186 Hz)
    # Formula: f = 27.5 * 2^(n/12) where n is the key index
    frequency = 27.5 * (2 ** (key_index / 12.0))

    num_samples = int(SAMPLE_RATE * duration)
    t = np.arange(num_samples) / SAMPLE_RATE

    # Generate harmonics for a more piano-like sound
    signal = np.sin(2 * np.pi * frequency * t)
    signal += 0.5 * np.sin(2 * np.pi * frequency * 2 * t)
    signal += 0.25 * np.sin(2 * np.pi * frequency * 3 * t)

    # Apply exponential decay envelope
    decay_rate = 2.0
    envelope = np.exp(-decay_rate * t)
    signal = signal * envelope

    # Normalize
    signal = signal / np.max(np.abs(signal) + 1e-10) * 0.8

    # Create stereo by duplicating with slight variation
    left = signal
    right = signal * 0.95

    return np.stack([left, right])


def generate_grand_piano_samples() -> dict:
    """Generate all 88 synthetic piano samples for testing.

    Returns:
        Dictionary mapping key index to audio array
    """
    return {i: generate_piano_sample(i, duration=3.0) for i in range(88)}


def test_faust_soundfile_piano():
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    faust_processor = engine.make_faust_processor("faust")
    faust_processor.num_voices = 16
    faust_processor.group_voices = True
    faust_processor.release_length = 0.5

    dsp_path = abspath(FAUST_DSP / "soundfile_piano.dsp")

    # Generate synthetic piano samples instead of downloading 148MB of audio
    piano_samples = generate_grand_piano_samples()
    soundfiles = {"mySound": [piano_samples[i] for i in range(88)]}
    faust_processor.set_soundfiles(soundfiles)

    faust_processor.set_dsp(dsp_path)
    faust_processor.compile()

    midi_path = (
        "MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi"
    )
    faust_processor.load_midi(abspath(ASSETS / midi_path))

    engine.load_graph([(faust_processor, [])])
    render(engine, file_path=OUTPUT / "test_sound_file_piano.wav", duration=10.0)

    audio = engine.get_audio()
    assert np.mean(np.abs(audio)) > 0.0001


@pytest.mark.parametrize(
    "audio_path,output_path",
    [(ASSETS / "60988__folktelemetry__crash-fast-14.wav", "test_faust_soundfile_many_notes.wav")],
)
def test_faust_soundfile_many_notes(audio_path: str, output_path, num_voices=10):
    """Load a stereo audio sample and pass it to Faust"""

    sample_seq = load_audio_file(audio_path)

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    faust_processor = engine.make_faust_processor("faust")
    faust_processor.num_voices = num_voices

    dsp_path = abspath(FAUST_DSP / "soundfile.dsp")

    if sample_seq.ndim == 1:
        sample_seq = sample_seq.reshape(1, -1)

    # set_soundfiles
    soundfiles = {"mySound": [sample_seq]}
    faust_processor.set_soundfiles(soundfiles)

    faust_processor.set_dsp(dsp_path)
    faust_processor.compile()

    # Notice how if the release length is too long then you'll see warnings about voice stealing.
    # Note that you should actually edit your DSP to use this release length (i.e., use ADSR to modulate
    # the gain and use this release length)
    faust_processor.release_length = 0.125

    faust_processor.set_parameter("/Sequencer/DSP1/Polyphonic/Voices/MyInstrument/soundChoice", 0)

    # (MIDI note, velocity, start sec, duration sec)
    for i in range(100):
        faust_processor.add_midi_note(60 + (i % 12), 30, i * 0.125 * 0.25, 0.125)

    engine.load_graph([(faust_processor, [])])
    render(engine, file_path=OUTPUT / output_path, duration=3.0)

    audio = engine.get_audio()
    assert np.mean(np.abs(audio)) > 0.001


# if __name__ == '__main__':
#   test_faust_soundfile_many_notes()

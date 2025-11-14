from dawdreamer_utils import *

BUFFER_SIZE = 128


@pytest.mark.parametrize(
    "audio_file_path,output_path,beats",
    [
        (
            ASSETS / "60988__folktelemetry__crash-fast-14.wav",
            "test_faust_poly_sampler_cymbal.wav",
            False,
        ),
        (
            ASSETS / "60988__folktelemetry__crash-fast-14.wav",
            "test_faust_poly_sampler_cymbal_beats.wav",
            True,
        ),
    ],
)
def test_faust_poly_sampler(audio_file_path: str, output_path: str, beats: bool, lagrange_order=4):
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    faust_processor = engine.make_faust_processor("faust")
    faust_processor.num_voices = 8

    sample_seq = load_audio_file(ASSETS / "60988__folktelemetry__crash-fast-14.wav")

    # set_soundfiles
    if sample_seq.ndim == 1:
        sample_seq = sample_seq.reshape(1, -1)
    soundfiles = {"mySample": [sample_seq]}
    faust_processor.set_soundfiles(soundfiles)

    dsp_path = abspath(FAUST_DSP / "polyphonic_sampler.dsp")
    dsp_code = open(dsp_path).read()

    dsp_code = (
        f"""
LAGRANGE_ORDER = {lagrange_order}; // lagrange order. [2-4] are good choices.
"""
        + dsp_code
    )
    # print('dsp code: ')
    # print(dsp_code)

    faust_processor.set_dsp_string(dsp_code)
    faust_processor.compile()

    faust_processor.get_parameters_description()
    # for par in desc:
    # 	print(par)

    # (MIDI note, velocity, start sec, duration sec)
    faust_processor.add_midi_note(60, 60, 0.0, 0.25, beats=beats)
    faust_processor.add_midi_note(64, 80, 0.5, 0.5, beats=beats)
    faust_processor.add_midi_note(67, 127, 0.75, 0.5, beats=beats)

    graph = [(faust_processor, [])]

    engine.load_graph(graph)
    render(engine, file_path=OUTPUT / output_path, duration=3.0)
    # check that it's non-silent
    audio = engine.get_audio()
    assert np.mean(np.abs(audio)) > 0.001

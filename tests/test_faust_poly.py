from dawdreamer_utils import *

def _test_faust_poly(file_path, group_voices=True, num_voices=8, buffer_size=1, cutoff=None,
    automation=False, decay=None):

    engine = daw.RenderEngine(SAMPLE_RATE, buffer_size)

    dsp_path = abspath(FAUST_DSP / "polyphonic.dsp")
    faust_processor = engine.make_faust_processor("faust")
    faust_processor.set_dsp(dsp_path)
    
    # Group voices will affect the number of parameters.
    # True will result in fewer parameters because all voices will
    # share the same parameters.
    faust_processor.group_voices = group_voices
    faust_processor.num_voices = num_voices
    
    faust_processor.compile()

    # for par in faust_processor.get_parameters_description():
    #   print(par)

     # (MIDI note, velocity, start sec, duration sec)
    faust_processor.add_midi_note(60, 60, 0.0, .25)
    faust_processor.add_midi_note(64, 80, 0.5, .5)
    faust_processor.add_midi_note(67, 127, 0.75, .5)

    assert(faust_processor.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

    if cutoff is not None:
        faust_processor.set_parameter("/Sequencer/DSP2/MyInstrument/cutoff", cutoff)
    elif automation:
        faust_processor.set_automation("/Sequencer/DSP2/MyInstrument/cutoff", 5000+4900*make_sine(30., 10.))

    if decay is not None:
        if group_voices:
            faust_processor.set_parameter("/Sequencer/DSP1/Polyphonic/Voices/MyInstrument/decay", decay)
        else:
            for i in range(1, num_voices+1):
                faust_processor.set_parameter(f"/Sequencer/DSP1/Polyphonic/V{i}/MyInstrument/decay", decay)

    # for par in faust_processor.get_parameters_description():
    #   print(par)

    graph = [
        (faust_processor, [])
    ]

    engine.load_graph(graph)

    render(engine, file_path=file_path, duration=3.)

    return engine.get_audio()

def test_faust_poly():
    audio1 = _test_faust_poly(OUTPUT / 'test_faust_poly_grouped.wav', group_voices=True)
    audio2 = _test_faust_poly(OUTPUT / 'test_faust_poly_ungrouped.wav', group_voices=False)

    assert np.allclose(audio1, audio2)

    audio1 = _test_faust_poly(OUTPUT / 'test_faust_poly_2k_cutoff_grouped.wav', group_voices=True, cutoff=2000)
    audio2 = _test_faust_poly(OUTPUT / 'test_faust_poly_2k_cutoff_ungrouped.wav', group_voices=False, cutoff=2000)

    assert np.allclose(audio1, audio2)

    audio1 = _test_faust_poly(OUTPUT / 'test_faust_poly_automation_cutoff_grouped.wav', group_voices=True, automation=True)
    audio2 = _test_faust_poly(OUTPUT / 'test_faust_poly_automation_cutoff_ungrouped.wav', group_voices=False, automation=True)

    assert np.allclose(audio1, audio2)

    audio1 = _test_faust_poly(OUTPUT / 'test_faust_poly_decay_grouped.wav', group_voices=True, decay=.5)
    audio2 = _test_faust_poly(OUTPUT / 'test_faust_poly_decay_ungrouped.wav', group_voices=False, decay=.5)

    assert np.allclose(audio1, audio2)


@pytest.mark.parametrize("midi_path,bpm_automation,convert_to_sec,buffer_size",
    product(
        [abspath(ASSETS / 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi')],
        [False, True],
        [False, True],
        [1, 128]
    )
)
def test_faust_sine(midi_path: str, bpm_automation: bool, convert_to_sec: bool, buffer_size: int):

    engine = daw.RenderEngine(SAMPLE_RATE, buffer_size)

    duration = 10.

    ppqn = 960

    if bpm_automation:
        bpm_data = 120.+60.*make_sine(1./3., duration*10., sr=ppqn)
        engine.set_bpm(bpm_data, ppqn=ppqn)

    faust_processor = engine.make_faust_processor("faust")
    faust_processor.num_voices = 16
    faust_processor.group_voices = True
    faust_processor.release_length = .5  # note that this is the maximum of the "release" hslider below

    faust_processor.set_dsp_string(
        f"""
        declare name "MyInstrument";

        declare options "[nvoices:8]"; // FaustProcessor has a property which will override this.
        import("stdfaust.lib");

        freq = hslider("freq",200,50,1000,0.01); // note pitch
        gain = hslider("gain",0.1,0,1,0.01);     // note velocity
        gate = button("gate");                   // note on/off

        attack = hslider("attack", .002, 0.001, 10., 0.);
        decay = hslider("decay", .05, 0.001, 10., 0.);
        sustain = hslider("sustain", 1.0, 0.0, 1., 0.);
        release = hslider("release", .05, 0.001, {faust_processor.release_length}, 0.);

        envVol = 0.35*gain*en.adsr(attack, decay, sustain, release, gate);

        process = os.osc(freq)*envVol <: _, _;
        effect = _, _;
        """
        )
    faust_processor.compile()
    # desc = faust_processor.get_parameters_description()
    # for par in desc:
    #   print(par)

    faust_processor.load_midi(midi_path, convert_to_sec=convert_to_sec, clear_previous=True, all_events=False)

    graph = [
        (faust_processor, [])
    ]

    engine.load_graph(graph)
    file_path = OUTPUT / ''.join([
        'test_faust_sine_',
        'bpm_' if bpm_automation else '',
        'conv_' if convert_to_sec else '',
        f'bs_{buffer_size}_',
        splitext(basename(midi_path))[0],
        '.wav'])
    render(engine, file_path=file_path, duration=duration)

    audio = engine.get_audio()
    assert(np.mean(np.abs(audio)) > .0001)

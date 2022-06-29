from dawdreamer_utils import *


def test_faust_instrument_midi_save():

    """The purpose of this test is to use `processor.save_midi("something.mid")`"""

    DURATION = 10.

    # The choices here affects the fidelity of the MIDI at the end
    BUFFER_SIZE = 1
    PPQN = 960*4

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    bpm_automation = make_sine(1./2., DURATION*10, sr=PPQN)
    bpm_automation = 60.+120.*(bpm_automation > 0).astype(np.float32)
    # bpm_automation = 30.+180.*(bpm_automation > 0).astype(np.float32)
    # bpm_automation = 150. + 30*bpm_automation
    engine.set_bpm(bpm_automation, ppqn=PPQN)

    faust_processor = engine.make_faust_processor("faust")
    faust_processor.num_voices = 16
    faust_processor.group_voices = True
    faust_processor.release_length = .5  # note that this is the maximum of the "release" hslider below

    faust_processor.set_dsp_string(
        f"""
        declare name "MyInstrument";

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

    assert faust_processor.get_num_output_channels() == 2

    midi_basename = 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'

    faust_processor.load_midi(abspath(ASSETS / midi_basename), beats=True)

    engine.load_graph([(faust_processor, [])])

    file_path = abspath(OUTPUT / f'test_faust_instrument_midi_save.wav')
    render(engine, file_path=file_path, duration=DURATION)

    # check that it's non-silent
    audio = engine.get_audio()
    assert(np.mean(np.abs(audio)) > .001)

    midi_basename = 'midi_faust_' + splitext(midi_basename)[0] + '.mid'

    faust_processor.save_midi(abspath(OUTPUT / midi_basename))


# note we're just testing one instrument
@pytest.mark.parametrize("plugin_path", ALL_PLUGIN_INSTRUMENTS[:1])
def test_plugin_instrument_midi_save(plugin_path):

    """The purpose of this test is to use `processor.save_midi("something.mid")`"""

    DURATION = 10.

    # The choices here affects the fidelity of the MIDI at the end
    BUFFER_SIZE = 1
    PPQN = 960*16

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    bpm_automation = make_sine(1./2., DURATION*10, sr=PPQN)
    bpm_automation = 60.+120.*(bpm_automation > 0).astype(np.float32)
    engine.set_bpm(bpm_automation, ppqn=PPQN)

    plugin_basename = splitext(basename(plugin_path))[0]

    synth = engine.make_plugin_processor("synth", plugin_path)

    midi_basename = 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'

    synth.load_midi(abspath(ASSETS / midi_basename), beats=True)

    engine.load_graph([(synth, []),])

    file_path = abspath(OUTPUT / f'test_plugin_instrument_midi_save_{plugin_basename}.wav')
    render(engine, file_path=file_path, duration=DURATION)

    # check that it's non-silent
    audio = engine.get_audio()
    assert(np.mean(np.abs(audio)) > .001)

    midi_basename = 'midi_plugin_' + splitext(midi_basename)[0] + '.mid'

    synth.save_midi(abspath(OUTPUT / midi_basename))


def test_automation_record_faust():

    """The purpose of this test is to use these functions:
    * `processor.record_automation = True`
    * `processor.get_automation()`

    This function will save a wav and CSV.
    Open them both in the same Sonic Visualiser project.
    The CSV annoation layer should be imported with these settings:
        Timing is specified: "Implicitly: rows are equally spaced in time"
        Audio sample rate: 44100 (SAMPLE_RATE in this function)
        Frame increment between rows: 1 (BUFFER_SIZE in this function)

    The generated audio is noise going through a low pass filter.
    We are telling the Faust effect to cycle its cutoff frequency 7 times per beat,
    and we are simultaneously changing the BPM via the engine.

    You should confirm that the peaks of the annotation layer align with the loudness
    peaks of the audio waveform.
    The BPM changes instantly back and forth between 120 and 240 BPM, so a quarter note
    should either be 500 ms or 250 ms.
    """

    DURATION = 4.

    # The choices here affects the fidelity of the `get_automation()` at the end
    BUFFER_SIZE = 1
    PPQN = 960*16

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    bpm_automation = make_sine(1./2., DURATION*10, sr=PPQN)
    bpm_automation = 120.+120.*(bpm_automation > 0).astype(np.float32)
    engine.set_bpm(bpm_automation, ppqn=PPQN)

    faust_processor = engine.make_faust_processor("faust")
    faust_processor.set_dsp_string("""
        declare name "MyInstrument";
        import("stdfaust.lib");
        freq = hslider("freq", 20., 20., 20000., .001);
        process = no.noise : _*.1 : fi.lowpass(10, freq) <: si.bus(2);
        """)

    automation = 10000.+9000.*make_sine(7., DURATION*10, sr=PPQN)
    par_name = "/MyInstrument/freq"
    faust_processor.set_automation(par_name, automation, ppqn=PPQN)

    # We will call get_automation() later, so we need to enable this here:
    faust_processor.record_automation = True

    desc = faust_processor.get_parameters_description()
    assert len(desc) > 0

    engine.load_graph([(faust_processor, [])])

    file_path = abspath(OUTPUT / f'test_automation_record_faust.wav')
    render(engine, file_path=file_path, duration=DURATION)

    # check that it's non-silent
    audio = engine.get_audio()
    assert(np.mean(np.abs(audio)) > .001)

    all_automation = faust_processor.get_automation()

    assert len(list(all_automation.keys())) > 0

    assert desc[0]['name'] == par_name
    automation = all_automation[par_name]
    assert automation.ndim == 2
    assert automation.shape[0] == 1
    assert automation.shape[1] == int(DURATION*SAMPLE_RATE)

    automation = automation.reshape(-1)

    np.savetxt(abspath(OUTPUT /'test_automation_record_faust.csv'), automation, delimiter=',', fmt='%f')


def test_automation_record_plugin():

    """The purpose of this test is to use these functions:
    * `processor.record_automation = True`
    * `processor.get_automation()`

    We also set automation with a PPQN, and we set the BPM to change over time
    to complicate the output of the recorded automation.

    Note we're just testing one instrument per OS.
    """

    if platform.system() == "Windows":
        plugin_path = abspath(PLUGINS / "Dimension Expander_x64.dll")
    elif platform.system() == "Darwin":
        plugin_path = abspath(PLUGINS / "TAL-NoiseMaker.vst")
    else:
        # skip on Linux
        return

    DURATION = 10.

    # the choices here affects the fidelity of the `get_automation()` at the end
    BUFFER_SIZE = 16
    PPQN = 960*16

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    bpm_automation = make_sine(1./2., DURATION*4, sr=PPQN)
    bpm_automation = 120.+30*(bpm_automation > 0).astype(np.float32)
    engine.set_bpm(bpm_automation, ppqn=PPQN)

    plugin_basename = splitext(basename(plugin_path))[0]

    synth = engine.make_plugin_processor("synth", plugin_path)

    desc = synth.get_plugin_parameters_description()

    automation = 0.5+.5*make_sine(1./2., DURATION*4, sr=PPQN)
    param_index = 0
    synth.set_automation(param_index, automation, ppqn=PPQN)

    # We will call get_automation() later, so we need to enable this here:
    synth.record_automation = True

    midi_basename = 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'

    synth.load_midi(abspath(ASSETS / midi_basename), beats=True)

    engine.load_graph([(synth, []),])

    file_path = abspath(OUTPUT / f'test_automation_record_plugin_{plugin_basename}.wav')
    render(engine, file_path=file_path, duration=DURATION)

    # check that it's non-silent
    audio = engine.get_audio()
    assert(np.mean(np.abs(audio)) > .001)

    all_automation = synth.get_automation()

    assert len(list(all_automation.keys())) > 0

    automation = all_automation[desc[param_index]['name']]
    assert automation.shape[1] == int(DURATION*SAMPLE_RATE)


if __name__ == "__main__":
    from mido import MidiFile

    filepath = OUTPUT / 'midi_faust_MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.mid'
    # filepath = ASSETS / 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'
    mid = MidiFile(abspath(filepath))
    num_messages = 0
    for i, track in enumerate(mid.tracks):
        print('Track {}: {}'.format(i, track.name))
        for msg in track:
            print(msg)
            num_messages += 1

    print('num events: ', num_messages)

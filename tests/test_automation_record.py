from dawdreamer_utils import *


# note we're just testing one instrument
@pytest.mark.parametrize("plugin_path", ALL_PLUGIN_INSTRUMENTS[:1])
def test_plugin_instrument_midi_record(plugin_path):

    """The purpose of this test is to use these functions:
    * `processor.record_automation = True`
    * `processor.get_automation()`
    * `processor.save_midi("something.mid")`

    We also set automation with a PPQN, and we set the BPM to change over time
    to complicate the output of the recorded automation.
    """

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

    automation = 0.5+.5*make_sine(1./2., DURATION*4, sr=PPQN)
    param_index = 1
    synth.set_automation(param_index, automation, ppqn=PPQN)

    # We will call get_automation() later, so we need to enable this here:
    synth.record_automation = True

    desc = synth.get_plugin_parameters_description()

    midi_basename = 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'

    synth.load_midi(abspath(ASSETS / midi_basename), beats=True)

    engine.load_graph([(synth, []),])

    file_path = abspath(OUTPUT / f'test_plugin_instrument_midi_record_{plugin_basename}.wav')
    render(engine, file_path=file_path, duration=DURATION)

    # check that it's non-silent
    audio = engine.get_audio()
    assert(np.mean(np.abs(audio)) > .001)

    synth.save_midi(abspath(OUTPUT / midi_basename))

    all_automation = synth.get_automation()

    assert len(list(all_automation.keys())) > 0

    automation = all_automation[desc[param_index]['name']]
    assert automation.shape[1] == int(DURATION*SAMPLE_RATE)

    # todo: save the automation with matplotlib or as a sonic visualizer CSV

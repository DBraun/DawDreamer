from dawdreamer_utils import *
import platform
import os.path
import os

MY_SYSTEM = platform.system()
# "Darwin" is macOS. "Windows" is Windows.

BUFFER_SIZE = 16

def _test_stereo_plugin_effect(plugin_path, expected_num_inputs):

    # Skip .component plugins on GitHub Actions workflows
    if os.getenv("CIBW_TEST_REQUIRES") and plugin_path.endswith('.component'):
        return

    if MY_SYSTEM == 'Darwin':
        # macOS treats .component and .vst3 as directories
        assert(os.path.isdir(plugin_path))
    else:
        assert(os.path.isfile(plugin_path))

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    data = load_audio_file(ASSETS / "575854__yellowtree__d-b-funk-loop.wav", DURATION+.1)

    playback_processor = engine.make_playback_processor("playback", data)

    effect = engine.make_plugin_processor("effect", plugin_path)

    # print(effect.get_plugin_parameters_description())
    assert(effect.get_num_input_channels() == expected_num_inputs)
    assert(effect.get_num_output_channels() == 2)

    graph = [
        (playback_processor, []),
        (effect, ["playback"])
    ]

    engine.load_graph(graph)

    plugin_basename = os.path.basename(plugin_path)

    render(engine, file_path=OUTPUT / f'test_plugin_effect_{plugin_basename}.wav', duration=DURATION)

    # check that it's non-silent
    audio = engine.get_audio()
    assert(np.mean(np.abs(audio)) > .05)

def test_stereo_plugin_effects():

    if MY_SYSTEM not in ["Darwin", "Windows"]:
        # todo: we should test LV2 plugins on Linux.
        return

    plugin_paths = []

    if MY_SYSTEM == 'Darwin':
        # todo: the Valhalla Freq Echo plugins sometimes work and sometimes just output NAN.
        # plugin_paths.append((abspath("plugins/ValhallaFreqEcho.vst"), 2))
        # plugin_paths.append((abspath("plugins/ValhallaFreqEcho.vst3"), 2))
        # plugin_paths.append((abspath("plugins/ValhallaFreqEcho.component"), 2))

        # RoughRider has an optional mono sidechain input
        plugin_paths.append((abspath(PLUGINS / "RoughRider3.vst"), 3))
        plugin_paths.append((abspath(PLUGINS / "RoughRider3.vst3"), 3))
        plugin_paths.append((abspath(PLUGINS / "RoughRider3.component"), 3))
    elif MY_SYSTEM == 'Windows':
        plugin_paths.append((abspath(PLUGINS / "Dimension Expander_x64.dll"), 2))

    for plugin_args in plugin_paths:
        _test_stereo_plugin_effect(*plugin_args)

def test_plugin_instrument():

    if MY_SYSTEM not in ["Darwin", "Windows"]:
        # todo: we should test LV2 plugins on Linux.
        return

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    plugin_name = "TAL-NoiseMaker.vst" if MY_SYSTEM == "Darwin" else "TAL-NoiseMaker-64.dll"

    synth = engine.make_plugin_processor("synth", abspath(PLUGINS / plugin_name))

    # print(synth.get_plugin_parameters_description())

    # (MIDI note, velocity, start sec, duration sec)
    # todo: on macOS the note is skipped if it starts at exactly 0.0.
    synth.add_midi_note(60, 60, 0.0, .25)
    synth.add_midi_note(64, 80, 0.5, .5)
    synth.add_midi_note(67, 127, 0.75, .5)

    assert(synth.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

    graph = [
        (synth, []),
    ]

    engine.load_graph(graph)

    render(engine, file_path=OUTPUT / 'test_plugin_instrument.wav', duration=DURATION)

    # check that it's non-silent
    audio = engine.get_audio()
    assert(np.mean(np.abs(audio)) > .01)

    try:
        synth.load_preset('bogus_path.fxp')
        assert False
    except Exception as e:
        print(f"Correctly caught exception: {e}")
        assert True

def test_plugin_instrument_midi():

    if MY_SYSTEM not in ["Darwin", "Windows"]:
        # todo: we should test LV2 plugins on Linux.
        return

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    plugin_name = "TAL-NoiseMaker.vst" if MY_SYSTEM == "Darwin" else "TAL-NoiseMaker-64.dll"

    synth = engine.make_plugin_processor("synth", abspath(PLUGINS / plugin_name))

    # print(synth.get_plugin_parameters_description())

    synth.load_midi(abspath(ASSETS / 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'))
    synth.clear_midi()
    synth.load_midi(abspath(ASSETS / 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'), all_events=True)
    synth.clear_midi()
    synth.load_midi(abspath(ASSETS / 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'), all_events=False)

    graph = [
        (synth, []),
    ]

    engine.load_graph(graph)

    render(engine, file_path=OUTPUT / 'test_plugin_instrument_midi.wav', duration=DURATION)

    # check that it's non-silent
    audio = engine.get_audio()
    assert(np.mean(np.abs(audio)) > .01)

def test_plugin_serum():

    if MY_SYSTEM not in ["Windows"]:
        # We don't Serum on platforms other than Windows.
        return

    plugin_path = "C:/VSTPlugIns/Serum_x64.dll"

    if not isfile(plugin_path):
        return

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    synth = engine.make_plugin_processor("synth", plugin_path)

    # print(synth.get_plugin_parameters_description())

    synth.get_parameter(0)
    synth.set_parameter(0, synth.get_parameter(0))
    synth.set_automation(0, np.array([synth.get_parameter(0)]))

    assert(synth.get_num_input_channels() == 0)
    assert(synth.get_num_output_channels() == 2)

     # (MIDI note, velocity, start sec, duration sec)
    synth.add_midi_note(60, 60, 0.0, .25)
    synth.add_midi_note(64, 80, 0.5, .5)
    synth.add_midi_note(67, 127, 0.75, .5)

    assert(synth.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

    graph = [
        (synth, []),
    ]

    engine.load_graph(graph)

    render(engine, file_path=OUTPUT / 'test_plugin_serum.wav', duration=DURATION)

    audio = engine.get_audio()
    assert(not np.allclose(audio*0., audio, atol=1e-07))


def _test_plugin_goodhertz_sidechain(do_sidechain=True):

    if MY_SYSTEM not in ["Windows"]:
        # We don't Goodhertz on platforms other than Windows.
        return

    plugin_path = "C:/VSTPlugIns/Goodhertz/Ghz Vulf Compressor 3.vst3"

    if not isfile(plugin_path):
        return

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    DURATION = 5.1

    vocals = load_audio_file(ASSETS / "575854__yellowtree__d-b-funk-loop.wav", duration=DURATION)
    drums = load_audio_file(ASSETS / "60988__folktelemetry__crash-fast-14.wav", duration=DURATION)

    drums *= .1

    vocals_processor = engine.make_playback_processor("vocals", vocals)
    drums_processor = engine.make_playback_processor("drums", drums)

    plugin = engine.make_plugin_processor("plugin", plugin_path)

    # plugin.set_parameter(2, 1.)
    # plugin.set_parameter(5, .1)
    # plugin.set_parameter(13, 1.)
    # plugin.set_parameter(15, 1.)
    # plugin.set_parameter(18, 1.)

    if do_sidechain:

        plugin.set_parameter(19, 0.5)
        # parameter 19 is the "External Sidechain" for Vulf Compressor. In the UI, click the three dots, which opens the panel
        # Then look for "External Sidechain" and set it to 50%.

        graph = [
            (vocals_processor, []),
            (drums_processor, []),
            (plugin, ["vocals", "drums"])
        ]
    else:
        graph = [
            (vocals_processor, []),
            (plugin, ["vocals"])
        ]

    engine.load_graph(graph)

    sidechain_on = "on" if do_sidechain else "off"

    file_path = OUTPUT / f'test_plugin_goodhertz_sidechain_{sidechain_on}.wav'

    render(engine, file_path=file_path, duration=DURATION)

    audio = engine.get_audio()
    assert(not np.allclose(audio*0., audio, atol=1e-07))


def test_plugin_goodhertz_sidechain():
    _test_plugin_goodhertz_sidechain(do_sidechain=True)
    _test_plugin_goodhertz_sidechain(do_sidechain=False)


def test_plugin_effect_ambisonics():

    if MY_SYSTEM != "Windows":
        return

    DURATION = 5.

    engine = daw.RenderEngine(48000, 128)

    data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav", DURATION+.1)

    # convert to mono (1, N)
    data = data.mean(axis=0, keepdims=True)

    playback_processor = engine.make_playback_processor("playback", data)

    plugin_path = "C:/VSTPlugIns/sparta/sparta_ambiENC.dll"

    if not isfile(plugin_path):
        return

    proc_encoder = engine.make_plugin_processor("effect", plugin_path)

    # print('inputs: ', proc_encoder.get_num_input_channels(), ' outputs: ', proc_encoder.get_num_output_channels())

    # proc_encoder.open_editor()

    proc_encoder.set_bus(1, 4)

    # print('inputs: ', proc_encoder.get_num_input_channels(), ' outputs: ', proc_encoder.get_num_output_channels())

    assert(proc_encoder.get_num_input_channels() == 1)
    assert(proc_encoder.get_num_output_channels() == 4)

    # for par in proc_encoder.get_plugin_parameters_description():
    #   print(par)

    graph = [
        (playback_processor, []),
        (proc_encoder, ["playback"])
    ]

    assert(engine.load_graph(graph))

    render(engine, file_path='output/test_plugin_effect_ambisonics.wav', duration=DURATION)

    audio = engine.get_audio()

    assert(proc_encoder.get_num_output_channels() == audio.shape[0])


def test_plugin_upright_piano():

    SYNTH_PLUGIN = "C:/VSTPlugIns/Upright Piano VST/Upright Piano.dll"

    if not isfile(SYNTH_PLUGIN):
        return

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    engine.set_bpm(120.) 

    synth = engine.make_plugin_processor("my_synth", SYNTH_PLUGIN)

    synth.add_midi_note(67, 127, 0.5, .25)

    # Basic reverb processor from JUCE.
    room_size = 0.5
    damping = 0.5
    wet_level = 0.33
    dry_level = 0.4
    width = 1.
    reverb_processor = engine.make_reverb_processor("my_reverb", room_size, damping, wet_level, dry_level, width)
    # ReverbProcessor has getters/setters
    reverb_processor.room_size = room_size
    reverb_processor.damping = damping
    reverb_processor.wet_level = wet_level
    reverb_processor.dry_level = dry_level
    reverb_processor.width = width

    graph = [
      (synth, []),
      (reverb_processor, ["my_synth"])
    ]
    engine.load_graph(graph)
    engine.render(5.)
    audio = engine.get_audio()
    audio = np.array(audio, np.float32).transpose()    
    wavfile.write(OUTPUT / 'test_plugin_upright_piano.wav', SAMPLE_RATE, audio)


def test_plugin_editor():

    if MY_SYSTEM not in ["Windows"]:
        # We don't Serum on platforms other than Windows.
        return

    # plugin_path = "C:/VSTPlugIns/Serum_x64.dll"
    plugin_path = "C:/VSTPlugIns/TAL-NoiseMaker-64.vst3"
    # plugin_path = "C:/VSTPlugIns/sparta/sparta_ambiBIN.dll"

    plugin_basename = os.path.splitext(basename(plugin_path))[0]

    if not isfile(plugin_path):
        return

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, 128)

    synth = engine.make_plugin_processor("synth", plugin_path)

    state_file_path = abspath(OUTPUT / (f'state_test_plugin_{plugin_basename}'))

    if isfile(state_file_path):
        synth.load_state(state_file_path)

    # synth.open_editor()

    synth.save_state(state_file_path)

    # print(synth.get_plugin_parameters_description())

    synth.get_parameter(0)
    synth.set_parameter(0, synth.get_parameter(0))
    synth.set_automation(0, np.array([synth.get_parameter(0)]))

    print('inputs: ', synth.get_num_input_channels(), ' outputs: ', synth.get_num_output_channels())

    # assert(synth.get_num_input_channels() == 0)
    # assert(synth.get_num_output_channels() == 2)

     # (MIDI note, velocity, start sec, duration sec)
    synth.add_midi_note(60, 60, 0.0, .25)
    synth.add_midi_note(64, 80, 0.5, .5)
    synth.add_midi_note(67, 127, 0.75, .5)

    assert(synth.n_midi_events == 3*2)  # multiply by 2 because of the off-notes.

    graph = [
        (synth, []),
    ]

    engine.load_graph(graph)

    render(engine, file_path=OUTPUT / (f'test_plugin_{plugin_basename}.wav'), duration=DURATION)

    audio = engine.get_audio()
    assert(not np.allclose(audio*0., audio, atol=1e-07))


def test_plugin_iem(plugin_path="C:/VSTPlugIns/IEMPluginSuite/VST2/IEM/MultiEncoder.dll",
                    plugin_path2="C:/VSTPlugIns/IEMPluginSuite/VST2/IEM/BinauralDecoder.dll"
    ):

    if "PYTEST_CURRENT_TEST" in os.environ:
        # we are not actually using pytest, so open the UI.
        # todo: need a pytest way to test open_editor()
        return
    

    if not isfile(plugin_path):
        return

    plugin_basename = os.path.splitext(basename(plugin_path))[0]

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, 128)

    ambisonics_encoder = engine.make_plugin_processor("ambisonics_encoder", plugin_path)
    ambisonics_decoder = engine.make_plugin_processor("ambisonics_decoder", plugin_path2)
    ambisonics_encoder.record = True
    ambisonics_decoder.record = True

    state_file_path = abspath(OUTPUT / (f'state_test_plugin_{plugin_basename}'))

    if isfile(state_file_path):
        ambisonics_encoder.load_state(state_file_path)

    AMBISONICS_ORDER = 3
    num_inputs = 1
    num_outputs = (AMBISONICS_ORDER+1)**2  # this is a fixed equation

    ambisonics_encoder.set_bus(num_inputs, num_outputs)

    # The UI window will open. In the upper-left, select 1-channel input.
    # In the upper-right select 3rd-order ambisonics (AMBISONICS_ORDER)
    ambisonics_encoder.open_editor()

    ambisonics_encoder.save_state(state_file_path)

    assert ambisonics_encoder.get_num_input_channels() == num_inputs
    assert ambisonics_encoder.get_num_output_channels() == num_outputs

    # print(ambisonics_encoder.get_plugin_parameters_description())
    # print('inputs: ', ambisonics_encoder.get_num_input_channels(), ' outputs: ', ambisonics_encoder.get_num_output_channels())

    ambisonics_decoder.set_bus(num_outputs, 2)

    # Remember to select AMBISONICS_ORDER ambisonics.
    ambisonics_decoder.open_editor()

    assert ambisonics_decoder.get_num_input_channels() == num_outputs
    assert ambisonics_decoder.get_num_output_channels() == 2

    data = load_audio_file("assets/575854__yellowtree__d-b-funk-loop.wav", DURATION+.1)
    # convert to mono (1, N)
    data = data.mean(axis=0, keepdims=True)

    graph = [
        (engine.make_playback_processor("playback", data), []),
        (ambisonics_encoder, ["playback"]),
        (ambisonics_decoder, [ambisonics_encoder.get_name()])
    ]

    engine.load_graph(graph)
    engine.render(DURATION)

    audio = ambisonics_decoder.get_audio()
    assert(not np.allclose(audio*0., audio, atol=1e-07))
    file_path = OUTPUT / f'test_plugin_{plugin_basename}_decoder.wav'
    wavfile.write(file_path, SAMPLE_RATE, audio.transpose())

    audio = ambisonics_encoder.get_audio()
    assert(not np.allclose(audio*0., audio, atol=1e-07))
    file_path = OUTPUT / f'test_plugin_{plugin_basename}_encoder.wav'
    wavfile.write(file_path, SAMPLE_RATE, audio.transpose())


if __name__ == '__main__':
    print('All done!')
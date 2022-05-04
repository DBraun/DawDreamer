from dawdreamer_utils import *


def add_midi(synth):
    # (MIDI note, velocity, start sec, duration sec)
    synth.add_midi_note(60, 60, 0.0, .25)
    synth.add_midi_note(64, 80, 0.5, .5)
    synth.add_midi_note(67, 127, 0.75, .5)


def test_plugin_mem_leak1():

    """test that multiple engines/playback processor/graphs/renders don't leak"""

    DURATION = .05

    data = load_audio_file(ASSETS / "575854__yellowtree__d-b-funk-loop.wav", duration=DURATION)
    
    for _ in range(10000):
        engine = daw.RenderEngine(SAMPLE_RATE, 16)
        playback_processor = engine.make_playback_processor("playback", data)
        engine.load_graph([(playback_processor, [])])
        engine.render(DURATION)


@pytest.mark.parametrize("plugin_path", ALL_PLUGIN_INSTRUMENTS)
def test_plugin_mem_leak2(plugin_path):

    """test that reloading the same graph with a re-used plugin doesn't leak"""

    DURATION = 1.5

    engine = daw.RenderEngine(SAMPLE_RATE, 2048)

    synth = engine.make_plugin_processor("synth", plugin_path)

    add_midi(synth)  

    for _ in range(100):
        graph = [(synth, [])]
        engine.load_graph(graph)
        render(engine, duration=DURATION)

@pytest.mark.parametrize("plugin_path", ALL_PLUGIN_INSTRUMENTS)
def test_plugin_mem_leak3(plugin_path):

    """test that re-creating plugin processors doesn't leak"""

    DURATION = 0.2

    engine = daw.RenderEngine(SAMPLE_RATE, 2048)

    for _ in range(100):
        synth = engine.make_plugin_processor("synth", plugin_path)
        synth.add_midi_note(60, 60, 0.0, .1)
        engine.load_graph([(synth, [])])
        render(engine, duration=DURATION)

# if __name__ == '__main__':
#     test_plugin_mem_leak3()
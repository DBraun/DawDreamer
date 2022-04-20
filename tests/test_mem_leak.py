from dawdreamer_utils import *


def add_midi(synth):
    # (MIDI note, velocity, start sec, duration sec)
    synth.add_midi_note(60, 60, 0.0, .25)
    synth.add_midi_note(64, 80, 0.5, .5)
    synth.add_midi_note(67, 127, 0.75, .5)

@pytest.mark.parametrize("plugin_path", ALL_PLUGIN_INSTRUMENTS)
def test_plugin_mem_leak1(plugin_path):

    """test that multiple renders don't leak"""

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, 2048)

    synth = engine.make_plugin_processor("synth", plugin_path)

    add_midi(synth)  

    graph = [(synth, [])]

    engine.load_graph(graph)

    for _ in range(100):
        render(engine, duration=DURATION)

@pytest.mark.parametrize("plugin_path", ALL_PLUGIN_INSTRUMENTS)
def test_plugin_mem_leak2(plugin_path):

    """test that reloading the same graph doesn't leak"""

    DURATION = 5.

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

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, 2048)

    for _ in range(40):
        synth = engine.make_plugin_processor("synth", plugin_path)
        add_midi(synth)  
        engine.load_graph([(synth, [])])
        render(engine, duration=DURATION)

# if __name__ == '__main__':
#     test_plugin_mem_leak3()
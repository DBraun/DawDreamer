from dawdreamer_utils import *


def add_midi(synth):
    # (MIDI note, velocity, start sec, duration sec)
    synth.add_midi_note(60, 60, 0.0, .25)
    synth.add_midi_note(64, 80, 0.5, .5)
    synth.add_midi_note(67, 127, 0.75, .5)

def test_plugin_mem_leak1():

    """test that multiple renders don't leak"""

    plugin_path = "C:/VSTPlugins/Serum_x64.dll"
    if not isfile(plugin_path):
        return

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, 2048)

    synth = engine.make_plugin_processor("synth", plugin_path)

    add_midi(synth)  

    graph = [(synth, [])]

    assert(engine.load_graph(graph))

    for _ in range(100):
        render(engine, duration=DURATION)

def test_plugin_mem_leak2():

    """test that reloading the same graph doesn't leak"""

    plugin_path = "C:/VSTPlugins/Serum_x64.dll"
    if not isfile(plugin_path):
        return

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, 2048)

    synth = engine.make_plugin_processor("synth", plugin_path)

    add_midi(synth)  

    for _ in range(100):
        graph = [(synth, [])]
        assert(engine.load_graph(graph))
        render(engine, duration=DURATION)

def test_plugin_mem_leak3():

    """test that re-creating plugin processors doesn't leak"""

    plugin_path = "C:/VSTPlugins/Serum_x64.dll"
    if not isfile(plugin_path):
        return

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, 2048)

    for _ in range(40):
        synth = engine.make_plugin_processor("synth", plugin_path)
        add_midi(synth)  
        assert(engine.load_graph([(synth, [])]))
        render(engine, duration=DURATION)

# if __name__ == '__main__':
#     test_plugin_mem_leak3()
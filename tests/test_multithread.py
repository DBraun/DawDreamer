from dawdreamer_utils import *

from scipy.io import wavfile

import numpy as np

import threading
import concurrent.futures

# GLOBAL PARAMETERS
BPM = 160
DURATION = .25

# Static parameters
SAMPLE_RATE = 44100
BLOCK_SIZE = 512

# PLUGIN_INSTRUMENT = "C:/VSTPlugins/Serum_x64.dll"
PLUGIN_INSTRUMENT = "C:/VSTPlugins/Sylenth1.dll"

PLUGIN_EFFECT = "C:/VSTPlugins/Dimension Expander_x64.dll"

def instrument(synthPlugin, name):
    engine = daw.RenderEngine(SAMPLE_RATE, BLOCK_SIZE)
    engine.set_bpm(BPM)

    synth = engine.make_plugin_processor("synth", synthPlugin)

    synth.add_midi_note(60, 100, 0.05, .15)

    graph = [(synth, [])]
    engine.load_graph(graph)

    print("Rendering instrument...")
    engine.render(DURATION)
    print("Finished rendering instrument.")

    output = engine.get_audio()

    assert np.abs(output).mean() > .01

    return output

def playback(audio, name):
    engine = daw.RenderEngine(SAMPLE_RATE, BLOCK_SIZE)
    engine.set_bpm(BPM)

    playbackProcessor = engine.make_playback_processor("playback", audio)

    graph = [(playbackProcessor, [])]
    engine.load_graph(graph)

    print("Rendering playback...")
    engine.render(DURATION)
    print("Finished rendering playback.")

    output = engine.get_audio()

    assert np.abs(output).mean() > .01

    return output

def effect(plugin, audio, name):
    engine = daw.RenderEngine(SAMPLE_RATE, BLOCK_SIZE)
    engine.set_bpm(BPM)

    playbackProcessor = engine.make_playback_processor("playback", audio)
    
    effect = engine.make_plugin_processor("effect", plugin)
    effect.set_parameter(0, 0.2)

    graph = [(playbackProcessor, []), (effect, ["playback"])]
    engine.load_graph(graph)

    print("Rendering effect...")
    engine.render(DURATION)
    print("Finished rendering effect.")

    output = engine.get_audio()

    assert np.abs(output).mean() > .01

    return output

def test_multithread():
    audio_path = ASSETS / "Music Delta - Disco/drums.wav"
    melodyData = load_audio_file(str(audio_path), duration=None)
        
    # Use ThreadPoolExecutor to execute tasks concurrently
    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as executor:
        # Start tasks and get a Future object for each one

        # test playback, no plugins:
        futures = {executor.submit(playback, melodyData, f"melody{i}"): f"melody{i}" for i in range(8)}
        
        # test vst plugin instruments:
        # futures = {executor.submit(instrument, PLUGIN_INSTRUMENT, f"melody{i}"): f"melody{i}" for i in range(8)}

    results = {}
    for future in concurrent.futures.as_completed(futures):
        key = futures[future]
        try:
            # If the task completed without raising an exception, future.result() returns the return value
            results[key] = future.result()
        except Exception as exc:
            # If the task raised an exception, future.result() will raise the same exception
            print(f'Task {key} generated an exception: {exc}')

    print('results', results)

    if "melody0" in results:
        wavfile.write(OUTPUT / "multithread_melody_0.wav", SAMPLE_RATE, results["melody0"].T)
    if "melody5" in results:
        wavfile.write(OUTPUT / "multithread_melody_5.wav", SAMPLE_RATE, results["melody5"].T)

    print('all done!')

if __name__ == "__main__":
    test_multithread()

import multiprocessing

import numpy as np
from dawdreamer_utils import *

from dawdreamer.faust import FaustContext
from dawdreamer.faust.box import boxMul, boxPar, boxReal, boxWire

# GLOBAL PARAMETERS
BPM = 160
DURATION = 0.25

# Static parameters
SAMPLE_RATE = 44100
BLOCK_SIZE = 512

from pytest import fixture


class MockPoolApplyResult:
    def __init__(self, func, args):
        self._func = func
        self._args = args

    def get(self, timeout=0):
        return self._func(*self._args)


@fixture(autouse=True)
def mock_pool_apply_async(monkeypatch):
    monkeypatch.setattr(
        "multiprocessing.pool.Pool.apply_async",
        lambda self,
        func,
        args=(),
        kwds={},
        callback=None,
        error_callback=None: MockPoolApplyResult(func, args),
    )


def instrument(synthPlugin, name):
    engine = daw.RenderEngine(SAMPLE_RATE, BLOCK_SIZE)
    engine.set_bpm(BPM)

    synth = engine.make_plugin_processor("synth", synthPlugin)

    synth.add_midi_note(60, 100, 0.0, 0.15)

    graph = [(synth, [])]
    engine.load_graph(graph)

    print("Rendering instrument...")
    engine.render(DURATION)
    print("Finished rendering instrument.")

    output = engine.get_audio()

    assert np.abs(output).mean() > 0.001

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

    assert np.allclose(audio[:, : output.shape[1]], output)

    return output


def faust_playback(audio, name):
    engine = daw.RenderEngine(SAMPLE_RATE, BLOCK_SIZE)
    engine.set_bpm(BPM)
    playbackProcessor = engine.make_playback_processor("playback", audio)
    faustProcessor = engine.make_faust_processor("faust")
    faustProcessor.set_dsp_string(
        """process = si.bus(2) : sp.stereoize(_*hslider("vol",1,0,1,0.001));"""
    )
    assert faustProcessor.compile()

    graph = [(playbackProcessor, []), (faustProcessor, ["playback"])]
    engine.load_graph(graph)

    print("Rendering faust playback...")
    engine.render(DURATION)
    print("Finished rendering faust playback.")

    output = engine.get_audio()

    assert np.allclose(audio[:, : output.shape[1]], output)

    return output


def faust_boxes(audio, name):
    engine = daw.RenderEngine(SAMPLE_RATE, BLOCK_SIZE)
    engine.set_bpm(BPM)
    playbackProcessor = engine.make_playback_processor("playback", audio)
    faustProcessor = engine.make_faust_processor("faust")

    with FaustContext():
        boxGainHalf = boxMul(boxWire(), boxReal(0.5))
        box = boxPar(boxGainHalf, boxGainHalf)

        faustProcessor.compile_box(box)

    graph = [(playbackProcessor, []), (faustProcessor, ["playback"])]
    engine.load_graph(graph)

    print("Rendering faust box...")
    engine.render(DURATION)
    print("Finished rendering faust box.")

    output = engine.get_audio()

    # confirm that the input was multiplied by 0.5
    minSize = min(output.shape[1], audio.shape[1])
    assert np.allclose(audio[:, :minSize] * 0.5, output[:, :minSize])

    return output


class TestClass:
    @staticmethod
    def get_audio():
        return load_audio_file(
            str(ASSETS / "Music Delta - Disco/drums.wav"), duration=10, offset=0.26
        )

    @staticmethod
    def get_pool():
        return multiprocessing.pool.Pool(processes=multiprocessing.cpu_count())

    # currently skipping this test (by putting underscore in front) because it fails on some plugins
    @pytest.mark.parametrize("plugin_path", ALL_PLUGIN_INSTRUMENTS[0:1])
    def _test_instrument(self, plugin_path):
        self.get_audio()
        with self.get_pool() as pool:
            tasks = [pool.apply_async(instrument, (plugin_path, f"test{i}")) for i in range(8)]
            # Collect tasks:
            [res.get() for res in tasks]

    def test_playback(self):
        audio_data = self.get_audio()
        with self.get_pool() as pool:
            tasks = [pool.apply_async(playback, (audio_data, f"test{i}")) for i in range(100)]
            # Collect tasks:
            [res.get() for res in tasks]

    def test_faust_playback(self):
        audio_data = self.get_audio()
        with self.get_pool() as pool:
            tasks = [pool.apply_async(faust_playback, (audio_data, f"test{i}")) for i in range(100)]
            # Collect tasks:
            [res.get() for res in tasks]

    def test_faust_boxes(self):
        audio_data = self.get_audio()
        with self.get_pool() as pool:
            tasks = [pool.apply_async(faust_boxes, (audio_data, f"test{i}")) for i in range(20)]
            # Collect tasks:
            [res.get() for res in tasks]


if __name__ == "__main__":
    test_class = TestClass()
    test_class._test_instrument(ALL_PLUGIN_INSTRUMENTS[0])
    test_class.test_playback()
    test_class.test_faust_playback()
    test_class.test_faust_boxes()
    print("all done")

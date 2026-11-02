import os

import numpy as np
from scipy.io import wavfile

import dawdreamer as daw

SAMPLE_RATE = 44100
BUFFER_SIZE = 512


def test_coldpiano_renders_chord_progression():
    vst3_path = os.environ.get("COLDPIANO_VST3_PATH")
    assert vst3_path and os.path.exists(vst3_path), f"Missing VST3 at {vst3_path}"

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    coldpiano = engine.make_plugin_processor("coldpiano", vst3_path)

    notes = [
        (48, 95, 0.1, 2.5),
        (55, 85, 0.3, 2.3),
        (62, 90, 0.5, 2.1),
        (63, 100, 0.7, 1.9),
        (67, 88, 0.9, 1.7),
    ]
    for pitch, velocity, start, duration in notes:
        coldpiano.add_midi_note(pitch, velocity, start, duration)

    engine.load_graph([(coldpiano, [])])
    assert engine.render(3.5)

    audio = engine.get_audio()
    assert audio.shape == (2, int(SAMPLE_RATE * 3.5))
    assert np.all(np.isfinite(audio)), "NaN or Inf detected in Coldpiano output!"

    peak = float(np.max(np.abs(audio)))
    print(f"Rendered Coldpiano audio peak amplitude: {peak:.5f}")
    assert peak > 1e-4, "Rendered audio is silent!"

    out_wav = os.path.join(os.path.dirname(__file__), "coldpiano_test_render.wav")
    wavfile.write(out_wav, SAMPLE_RATE, audio.T.astype(np.float32))

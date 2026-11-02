import argparse
from pathlib import Path

import dawdreamer as daw
import numpy as np
import soundfile as sf


def main():
    parser = argparse.ArgumentParser(
        description="Open the Ambience VST3 editor and render a reverberated WAV."
    )
    parser.add_argument("plugin", type=Path, help="Path to Ambience2.0.2.vst3")
    parser.add_argument("input", type=Path, help="Mono or stereo WAV input")
    parser.add_argument("output", type=Path, help="Output WAV path")
    parser.add_argument(
        "--tail-seconds", type=float, default=3.0, help="Reverb tail after the input"
    )
    args = parser.parse_args()

    if not args.plugin.exists():
        parser.error(f"Ambience VST3 plugin not found: {args.plugin}")
    if args.tail_seconds < 0:
        parser.error("--tail-seconds must not be negative")

    audio, sample_rate = sf.read(args.input, dtype="float32", always_2d=True)
    if audio.shape[1] == 1:
        audio = np.repeat(audio, 2, axis=1)
    elif audio.shape[1] != 2:
        parser.error("Input audio must be mono or stereo")

    engine = daw.RenderEngine(sample_rate, 512)
    source = engine.make_playback_processor("input", audio.T.copy())
    ambience = engine.make_plugin_processor("ambience", str(args.plugin))
    engine.load_graph([(source, []), (ambience, ["input"])])

    parameter_count = len(ambience.get_parameters_description())
    print(f"Loaded {ambience.get_name()} with {parameter_count} parameters.")
    ambience.open_editor()

    duration = audio.shape[0] / sample_rate + args.tail_seconds
    engine.render(duration)
    sf.write(args.output, engine.get_audio().T, sample_rate)
    print(f"Rendered {args.output}")


if __name__ == "__main__":
    main()

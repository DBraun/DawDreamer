# Ambience 2.0.2 Reverb

This example hosts the Ambience VST3 effect in DawDreamer, opens its native
editor so you can adjust the reverb, then renders an audio file with the chosen
settings.

Ambience is a separate, Windows-only VST3 plugin licensed under AGPL-3.0. Its
source and releases are available from the
[OTODESK4193/Ambience repository](https://github.com/OTODESK4193/Ambience).
Download the [Ambience 2.0.2 release](https://github.com/OTODESK4193/Ambience/releases/tag/V2.0.2)
and install `Ambience2.0.2.vst3` in `C:\Program Files\Common Files\VST3\`, or
pass its installed path to the example. DawDreamer does not bundle the plugin.

Install the Python dependencies if needed:

```bash
python -m pip install dawdreamer soundfile
```

Run the example on Windows with a mono or stereo WAV input:

```cmd
python examples/ambience_reverb/ambience_reverb.py ^
  "C:\Program Files\Common Files\VST3\Ambience2.0.2.vst3" ^
  input.wav output.wav
```

The plugin editor is interactive and blocks the script until you close it.
Afterward, the script renders the input through Ambience and adds a three-second
reverb tail by default. Set `--tail-seconds` to change the tail length.

# DawDreamer - Multiprocessing Plugins (with XML presets)

This script demonstrates how to use [multiprocessing](https://docs.python.org/3/library/multiprocessing.html) to efficiently generate one-shots of a synthesizer. In this specific script and notebook, we happen to use the [TAL-U-No-LX](https://tal-software.com/products/tal-u-no-lx) VST synthesizer, which comes with several XML presets. We use the `set_parameter()` method in DawDreamer to apply the settings from the XML to the synthesizer.

The number of workers is by default `multiprocessing.cpu_count()`. Each worker has a persistent RenderEngine which loads a plugin instrument of our choice. Each worker consumes paths of presets from a multiprocessing [Queue](https://docs.python.org/3/library/multiprocessing.html#pipes-and-queues). For each preset, the worker renders out audio for a configurable MIDI pitch range. The output audio path includes the pitch and preset name.

To run a CLI example that produces many sounds in parallel with the TAL-U-NO-LX VST plugin using multiprocessing, you may adjust and run the following: 

```bash
python main.py --plugin "path/to/TAL-U-NO-LX-V2.vst3" --preset-dir "path/to/TAL-U-NO-LX_presets"
```

To see all available parameters:
```bash
python main.py --help
```

# DawDreamer - Multiprocessing Plugins

This script demonstrates how to use [multiprocessing](https://docs.python.org/3/library/multiprocessing.html) to efficiently generate one-shots of a synthesizer. The number of workers is by default `multiprocessing.cpu_count()`. Each worker has a persistent RenderEngine which loads a plugin instrument of our choice. Each worker consumes paths of presets from a multiprocessing [Queue](https://docs.python.org/3/library/multiprocessing.html#pipes-and-queues). For each preset, the worker renders out audio for a configurable MIDI pitch range. The output audio path includes the pitch and preset name.

**Not every plugin is guaranteed to work. I have tested Serum on Windows, and it works perfectly.**

Example usage:

```bash
python main.py --plugin "path/to/Serum_x64.dll" --preset-dir "path/to/serum_fxp_files"
```

To see all available parameters:
```bash
python main.py --help
```

Improvement ideas:
* The input could be a more nested directory of presets.
* Alternatively, the items in the input queue could be parameter settings rather than preset paths. A multiprocessing *Processor* could add random parameters to the input queue.
* Variations in velocity
* Variations in note duration

# DawDreamer - Parallel Plugin Rendering (with XML presets)

This script demonstrates how to efficiently generate one-shots of a synthesizer with a [ThreadPoolExecutor](https://docs.python.org/3/library/concurrent.futures.html#threadpoolexecutor).
DawDreamer releases the GIL while rendering, so worker threads render truly in parallel inside a single process.
In this specific script and notebook, we happen to use the [TAL-U-No-LX](https://tal-software.com/products/tal-u-no-lx) VST synthesizer, which comes with several XML presets.
We use the `set_parameter()` method in DawDreamer to apply the settings from the XML to the synthesizer.

The number of workers is by default `os.cpu_count()`.
Each worker has a persistent RenderEngine which loads a plugin instrument of our choice.
Each worker consumes paths of presets from a shared [Queue](https://docs.python.org/3/library/queue.html).
For each preset, the worker renders out audio for a configurable MIDI pitch range.
The output audio path includes the pitch and preset name.
If a plugin misbehaves when multiple instances run in one process, fall back to `multiprocessing` with the same worker structure to isolate each instance in its own process.

To run a CLI example that produces many sounds in parallel with the TAL-U-NO-LX VST plugin, you may adjust and run the following:

```bash
python main.py --plugin "path/to/TAL-U-NO-LX-V2.vst3" --preset-dir "path/to/TAL-U-NO-LX_presets"
```

To see all available parameters:
```bash
python main.py --help
```

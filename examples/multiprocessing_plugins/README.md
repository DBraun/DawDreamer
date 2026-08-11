# DawDreamer - Parallel Plugin Rendering

This script demonstrates how to efficiently generate one-shots of a synthesizer with a [ThreadPoolExecutor](https://docs.python.org/3/library/concurrent.futures.html#threadpoolexecutor).
DawDreamer releases the GIL while rendering, so worker threads render truly in parallel inside a single process.
The number of workers is by default `os.cpu_count()`.
Each worker has a persistent `RenderEngine` which loads a plugin instrument of our choice.
Each worker consumes paths of presets from a shared [Queue](https://docs.python.org/3/library/queue.html).
For each preset, the worker renders out audio for a configurable MIDI pitch range.
The output audio path includes the pitch and preset name.

Compared to `multiprocessing`, threads use far less memory (one Python process instead of one per worker) and start up faster because the plugin binary is only loaded into one process.
If a plugin misbehaves when multiple instances run in one process (for example, due to global state in the plugin), fall back to `multiprocessing` with the same worker structure to isolate each instance in its own process.

**Not every plugin is guaranteed to work. Serum has been tested on Windows, and it should work perfectly.**

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
* Alternatively, the items in the input queue could be parameter settings rather than preset paths. A producer thread could add random parameters to the input queue.
* Variations in velocity
* Variations in note duration

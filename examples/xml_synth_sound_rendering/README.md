# Synth Sound Rendering

This notebook demonstrates the capability of automatic sound rendering with VST synthesizers using DawDreamer. From a high level, the example here loads/parses a random XML preset for TAL-U-No-LX VST synthesizer and sets the parameters of the loaded plugin using the nice `set_parameter()` method in DawDreamer. See [#131](https://github.com/DBraun/DawDreamer/issues/131#issuecomment-1305941285) for more information about the inspiration for this script.

To run a CLI example that produces many sounds in parallel with the TAL-U-NO-LX VST plugin using multiprocessing, you may run the following: 

```bash
python main.py --plugin "path/to/TAL-U-NO-LX-V2.vst3" --preset-dir "path/to/TAL-U-NO-LX_presets"
```

To see all available parameters:
```bash
python main.py --help
```

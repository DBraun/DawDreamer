```
  _____                       _____                                                  
 |  __ \                     |  __ \                                                 
 | |  | |   __ _  __      __ | |  | |  _ __    ___    __ _   _ __ ___     ___   _ __ 
 | |  | |  / _` | \ \ /\ / / | |  | | | '__|  / _ \  / _` | | '_ ` _ \   / _ \ | '__|
 | |__| | | (_| |  \ V  V /  | |__| | | |    |  __/ | (_| | | | | | | | |  __/ | |   
 |_____/   \__,_|   \_/\_/   |_____/  |_|     \___|  \__,_| |_| |_| |_|  \___| |_|   
                                                                                     
* * Digital Audio Workstation with Python * *
```

![Supported Platforms](https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux-green)
[![Test Badge](https://github.com/DBraun/DawDreamer/actions/workflows/all.yml/badge.svg)](https://github.com/DBraun/DawDreamer/actions/workflows/all.yml)
[![PyPI version fury.io](https://badge.fury.io/py/ansicolortags.svg)](https://pypi.python.org/pypi/dawdreamer/)
[![GPLv3 license](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://github.com/DBraun/DawDreamer/blob/main/LICENSE)
![GitHub Repo stars](https://img.shields.io/github/stars/DBraun/DawDreamer?style=social)
[![Generic badge](https://img.shields.io/badge/Documentation-passing-brightgreen.svg)](https://dirt.design/DawDreamer/)

# DawDreamer

Read the [introduction](https://arxiv.org/abs/2111.09931) to DawDreamer, which was presented as a Late-Breaking Demo at the [2021 ISMIR Conference](https://ismir2021.ismir.net/lbd/).

DawDreamer is an audio-processing Python framework supporting core [DAW](https://en.wikipedia.org/wiki/Digital_audio_workstation) features and beyond:
* Composing graphs of multi-channel audio processors
* Audio playback
* [VST instruments and effects](https://github.com/DBraun/DawDreamer/wiki/Plugin-Processor) (with UI editing and state loading/saving)
* [FAUST](https://github.com/DBraun/DawDreamer/wiki/Faust-Processor) effects and polyphonic instruments
* [Time-stretching and looping](https://github.com/DBraun/DawDreamer/wiki/Playback-Warp-Processor), optionally according to Ableton Live warp markers
* [Pitch-warping](https://github.com/DBraun/DawDreamer/wiki/Playback-Warp-Processor)
* Parameter automation at audio-rate and at pulses-per-quarter-note
* Parameter automation saving in absolute audio-rate time
* MIDI playback in absolute time and PPQN time
* MIDI file export in absolute time
* Rendering and saving multiple processors simultaneously
* Support for the [Faust Box](https://github.com/DBraun/DawDreamer/tree/main/examples/Box_API) and Signal APIs
* Transpiling Faust code to [JAX/Flax](https://github.com/DBraun/DawDreamer/tree/main/examples/Faust_to_JAX) and other target languages (C++, Rust, Wasm, etc.)
* Machine learning experiments with [QDax](https://github.com/DBraun/DawDreamer/tree/main/examples/Faust_to_QDax)
* [Multiprocessing support](https://github.com/DBraun/DawDreamer/tree/main/examples/multiprocessing_plugins)
* Full support on macOS, Windows, Linux, Google Colab, and Ubuntu Dockerfile

DawDreamer's foundation is [JUCE](https://github.com/julianstorer/JUCE), with a user-friendly Python interface thanks to [pybind11](https://github.com/pybind/pybind11). DawDreamer evolved from an earlier VSTi audio "renderer", [RenderMan](https://github.com/fedden/RenderMan).

## Installation

macOS requirements:
* 64-bit Python 3.9 or higher
* macOS 11.0 or higher

Windows requirements:
* 64-bit Python 3.8 or higher

Linux requirements:
* 64-bit Python 3.8 or higher

Install with [PyPI](https://pypi.org/project/dawdreamer/):

`pip install dawdreamer`

## API Documentation

[https://dirt.design/DawDreamer/](https://dirt.design/DawDreamer/dawdreamer.html)

## Basic Example

Using Faust, let's make a stereo sine-tone at 440 Hz and -6 dB. You can run this code as-is.

```python
import dawdreamer as daw
from scipy.io import wavfile
SAMPLE_RATE = 44100
engine = daw.RenderEngine(SAMPLE_RATE, 512)  # 512 block size
faust_processor = engine.make_faust_processor("faust")
faust_processor.set_dsp_string(
    """
    declare name "MySine";
    freq = hslider("freq", 440, 0, 20000, 0);
    gain = hslider("vol[unit:dB]", 0, -120, 20, 0) : ba.db2linear;
    process = freq : os.osc : _*gain <: si.bus(2);
    """
    )
print(faust_processor.get_parameters_description())
engine.load_graph([
                   (faust_processor, [])
])
faust_processor.set_parameter("/MySine/freq", 440.)  # 440 Hz
faust_processor.set_parameter("/MySine/vol", -6.)  # -6 dB volume

engine.set_bpm(120.)
engine.render(4., beats=True)  # render 4 beats.
audio = engine.get_audio()  # shaped (2, N samples)
wavfile.write('sine_demo.wav', SAMPLE_RATE, audio.transpose())

# Change settings and re-render
faust_processor.set_parameter("/MySine/freq", 880.)  # 880 Hz
engine.render(4., beats=True)
# and so on...
```

Next, let's make a graph with a VST instrument and effect. This graph will be simple, but you can make more complicated ones.

```python
import dawdreamer as daw
from scipy.io import wavfile
SAMPLE_RATE = 44100
INSTRUMENT_PATH = "path/to/instrument.dll"
EFFECT_PATH = "path/to/effect.dll"

engine = daw.RenderEngine(SAMPLE_RATE, 512)
engine.set_bpm(120.)

synth = engine.make_plugin_processor("synth", INSTRUMENT_PATH)
print('inputs:', synth.get_num_input_channels())
print('inputs:', synth.get_num_output_channels())
print(synth.get_parameters_description())

synth.set_parameter(7, .1234)

# (MIDI note, velocity, start sec, duration sec)
synth.add_midi_note(60, 100, 0.0, 2.)

effect = engine.make_plugin_processor("effect", EFFECT_PATH)

engine.load_graph([
  (synth, []),
  (effect, [synth.get_name()])  # effect needs 2 channels, and "synth" provides those 2.
  ])

engine.render(4.)  # render 4 seconds.
audio = engine.get_audio()
wavfile.write('synth_demo.wav', SAMPLE_RATE, audio.transpose())
synth.clear_midi()
# add midi again, render again, and so on...
```

Please refer to the [Wiki](https://github.com/DBraun/DawDreamer/wiki), [examples](https://github.com/DBraun/DawDreamer/tree/main/examples/), [API documentation](https://dirt.design/DawDreamer), and [tests](https://github.com/DBraun/DawDreamer/tree/main/tests). 

## License

DawDreamer is licensed under GPLv3 to make it easier to comply with all of the dependent projects. If you use DawDreamer, you must obey the licenses of [JUCE](https://github.com/juce-framework/JUCE/), [pybind11](https://github.com/pybind/pybind11/), [Libsamplerate](https://github.com/libsndfile/libsamplerate), [Rubber Band Library](https://github.com/breakfastquay/rubberband/), [Steinberg VST2/3](https://www.steinberg.net/vst-instruments/), and [FAUST](https://github.com/grame-cncm/faust).

## Thanks to contributors to the original [RenderMan](https://github.com/fedden/RenderMan)
* [fedden](https://github.com/fedden), RenderMan creator
* [jgefele](https://github.com/jgefele)
* [harritaylor](https://github.com/harritaylor)
* [cannoneyed](https://github.com/cannoneyed/)

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

DawDreamer is an audio-processing Python framework supporting core [DAW](https://en.wikipedia.org/wiki/Digital_audio_workstation) features:
* Composing graphs of multi-channel audio processors
* Audio playback
* VST instruments and effects
* [FAUST](http://faust.grame.fr/) effects and [polyphonic instruments](https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters)
* Time-stretching and looping according to Ableton Live warp markers
* Pitch-warping
* Parameter automation
* Rendering multiple processors simultaneously
* Full support on macOS, Windows, Linux, and Ubuntu Dockerfile

DawDreamer's foundation is [JUCE](https://github.com/julianstorer/JUCE), with a user-friendly Python interface thanks to [pybind11](https://github.com/pybind/pybind11). DawDreamer evolved from an earlier VSTi audio "renderer", [RenderMan](https://github.com/fedden/RenderMan).

## Installation

`pip install dawdreamer`

## Basic Example
```python
import dawdreamer as daw
import numpy as np
from scipy.io import wavfile
import librosa

SAMPLE_RATE = 44100
BUFFER_SIZE = 128 # Parameters will undergo automation at this block size.
SYNTH_PLUGIN = "C:/path/to/synth.dll"  # for instruments, DLLs work.
SYNTH_PRESET = "C:/path/to/preset.fxp"
REVERB_PLUGIN = "C:/path/to/reverb.dll"  # for effects, both DLLs and .vst3 files work
VOCALS_PATH = "C:/path/to/vocals.wav"
PIANO_PATH = "C:/path/to/piano.wav"
SAMPLE_PATH = "C:/path/to/clap.wav"  # sound to be used for sampler instrument.

def load_audio_file(file_path, duration=None):
  sig, rate = librosa.load(file_path, duration=duration, mono=False, sr=SAMPLE_RATE)
  assert(rate == SAMPLE_RATE)
  return sig

def make_sine(freq: float, duration: float, sr=SAMPLE_RATE):
  """Return sine wave based on freq in Hz and duration in seconds"""
  N = int(duration * sr) # Number of samples 
  return np.sin(np.pi*2.*freq*np.arange(N)/sr)

# Make an engine. We'll only need one.
engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
engine.set_bpm(120.)  # default is 120.

DURATION = 10 # How many seconds we want to render.

# Load audio into a numpy array shaped (Number Channels, Number Samples)
vocals = load_audio_file(VOCALS_PATH, duration=10.)
piano = load_audio_file(PIANO_PATH, duration=10.)

# Make a processor and give it the name "my_synth", which we must remember later.
synth = engine.make_plugin_processor("my_synth", SYNTH_PLUGIN)
synth.load_preset(SYNTH_PRESET)
synth.set_parameter(5, 0.1234) # override a specific parameter.
synth.load_midi("C:/path/to/song.mid")
# We can also add notes one at a time.
synth.add_midi_note(67, 127, 0.5, .25) # (MIDI note, velocity, start sec, duration sec)

# We can automate VST parameters over time. First, we must know the parameter names.
# Get a list of dictionaries where each dictionary describes a controllable parameter.
print(synth.get_plugin_parameters_description()) 
print(synth.get_parameter_name(1)) # For Serum, returns "A Pan" (the panning of oscillator A)
# The Plugin Processor can set automation according to a parameter index.
synth.set_automation(1, make_sine(.5, DURATION)) # 0.5 Hz sine wave.

# For any processor type, we can get the number of inputs and outputs
print("synth num inputs: ", synth.get_num_input_channels())
print("synth num outputs: ", synth.get_num_output_channels())

# The sampler processor works like the plugin processor.
# Provide audio for the sample, and then provide MIDI.
# The note value affects the pitch and playback speed of the sample.
# There are basic sampler parameters such as ADSR for volume and filters which you can
# inspect with `get_parameters_description()`
sampler = engine.make_sampler_processor("my_sampler", load_audio_file(SAMPLE_PATH))
# sampler.set_data(load_audio_file(SAMPLE_PATH_2))  # this is allowed too at any time.
print(sampler.get_parameters_description())
sampler.set_parameter(0, 60.)  # set the center frequency to middle C (60)
sampler.set_parameter(5, 100.) # set the volume envelope's release to 100 milliseconds.
sampler.load_midi("C:/path/to/sampler_rhythm.mid")
# We can also add notes one at a time.
sampler.add_midi_note(67, 127, 0.5, .25) # (MIDI note, velocity, start sec, duration sec)

# We can make basic signal processors such as filters and automate their parameters.
filter_processor = engine.make_filter_processor("filter", "high", 7000.0, .5, 1.)
filter_processor.freq = 7123.  # Some parameters can be get/set like this.
freq_automation = make_sine(.5, DURATION)*5000. + 7000. # 0.5 Hz sine wave centered at 7000 w/ amp 5000.
filter_processor.set_automation("freq", freq_automation) # argument is single channel numpy array.
freq_automation = filter_processor.get_automation("freq") # You can get automation of most processor parameters.
filter_processor.record = True  # This will allow us to access the filter processor's audio after a render.

# Graph idea is based on https://github.com/magenta/ddsp#processorgroup-with-a-list
# A graph is a meaningfully ordered list of tuples.
# In each tuple, the first item is an audio processor.
# The second item is this audio processor's list of input processors.
# You must create each processor with a unique name
# and refer to these unique names in the lists of inputs.
# The audio from the last tuple's processor will be accessed automatically later by engine.get_audio()
graph = [
  (synth, []),  # synth takes no inputs, so we give an empty list.
  (sampler, []),  # sampler takes no inputs.
  (engine.make_reverb_processor("reverb"), ["my_synth"]), # Apply JUCE reverb to the synth named earlier
  (engine.make_plugin_processor("more_reverb", REVERB_PLUGIN), ["reverb"]), # Apply VST reverb
  (engine.make_playback_processor("vocals", vocals), []), # Playback has no inputs.
  (filter_processor, ["vocals"]), # High-pass filter with automation set earlier.
  (engine.make_add_processor("added"), ["more_reverb", "filter", "my_sampler"])
]

engine.load_graph(graph)

engine.render(DURATION)  # Render 10 seconds audio.

# Return the audio from the graph's last processor, even if its recording wasn't enabled.
# The shape will be numpy.ndarray shaped (chans, samples)
audio = engine.get_audio()  
wavfile.write('my_song.wav', SAMPLE_RATE, audio.transpose()) # don't forget to transpose!

# You can get the audio of any processor whose recording was enabled.
filtered_audio = filter_processor.get_audio()
# Or get audio according to the processor's unique name
filtered_audio = engine.get_audio("filter")

# You can modify processors without recreating the graph.
synth.load("C:/path/to/other_preset.fxp")
engine.render(DURATION)  # render audio again!
```

## FAUST

### New to FAUST?

* Browse the suggested [Documentation and Resources](https://github.com/grame-cncm/faust#documentation-and-resources).
* Julius Smith's [Audio Signal Processing in FAUST](https://ccrma.stanford.edu/~jos/aspf/).
* Browse the [Libraries](https://faustlibraries.grame.fr/).
* The [Syntax Manual](https://faustdoc.grame.fr/manual/syntax/).
* Join the [communities](https://faust.grame.fr/community/help/).

### Using FAUST processors

Let's start by looking at FAUST DSP files, which end in `.dsp`. For convenience, the standard library is always imported, so you don't need to `import("stdfaust.lib");` Here's an example using a demo stereo reverb:

#### **faust_reverb.dsp:**
```dsp
process = dm.zita_light;
```

#### **faust_test.py:**
```python
DSP_PATH = "C:/path/to/faust_reverb.dsp"  # Must be absolute path
faust_processor = engine.make_faust_processor("faust")
faust_processor.set_dsp(DSP_PATH)  # You can do this anytime.

# Using compile() isn't necessary, but it's an early warning check.
assert(faust_processor.compile())

print(faust_processor.get_parameters_description())

# You can set parameters by index or by address.
faust_processor.set_parameter("/Zita_Light/Dry/Wet_Mix", 1.)
faust_processor.set_parameter(0, 1.)

# Unlike VSTs, these parameters aren't necessarily 0-1 values.
# For example, if you program your FAUST code to have a 15000 kHz filter cutoff
# you can set it naturally:
#     faust_processor.set_parameter(7, 15000)

print('val: ', faust_processor.get_parameter("/Zita_Light/Dry/Wet_Mix"))
print('val: ', faust_processor.get_parameter(0))

graph = [
  (engine.make_playback_processor("piano", load_audio_file("piano.wav")), []),
  (faust_processor, ["piano"])
]

engine.load_graph(graph)
engine.render(DURATION)
```

Here's an example that mixes two stereo inputs into one stereo output and applies a low-pass filter.

#### **dsp_4_channels.dsp:**
```dsp
declare name "MyEffect";
myFilter= fi.lowpass(10, hslider("cutoff",  15000.,  20,  20000,  .01));
process = _, _, _, _ :> myFilter, myFilter;
```

#### **faust_test_stereo_mixdown.py:**
```python
faust_processor = engine.make_faust_processor("faust")
faust_processor.set_dsp("C:/path/to/dsp_4_channels.dsp")  # Must be absolute path
print(faust_processor.get_parameters_description())
faust_processor.set_parameter("/MyEffect/cutoff", 7000.0)  # Change the cutoff frequency.
# or set automation like this
faust_processor.set_automation("/MyEffect/cutoff", 15000+5000*make_sine(2, DURATION))
graph = [
  (engine.make_playback_processor("piano", load_audio_file("piano.wav")), []),
  (engine.make_playback_processor("vocals", load_audio_file("vocals.wav")), []),
  (faust_processor, ["piano", "vocals"])
]

engine.load_graph(graph)
engine.render(DURATION)
```

### Polyphony in Faust

Polyphony is supported too. You simply need to provide DSP code that refers to correctly named parameters such as `freq` or `note`, `gain`, and `gate`. For more information, see the FAUST [manual](https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters). In DawDreamer, you must set the number of voices on the processor to 1 or higher. 0 disables polyphony. Refer to `tests/test_faust_poly.py`.

### Soundfiles in Faust

Faust code in DawDreamer can use the [soundfile](https://faustdoc.grame.fr/manual/syntax/#soundfile-primitive) primitive. Normally `soundfile` is meant to load `.wav` files, but DawDreamer uses it to receive data from numpy arrays.

**soundfile_test.py**
```python
# suppose audio1, audio2, and audio3 are np.array shaped (Channels, Samples)
soundfiles = {
  'mySound': [audio1, audio2, audio3]
}
faust_processor.set_soundfiles(soundfiles)
```

**soundfile_test.dsp**
```dsp
soundChoice = nentry("soundChoice", 0, 0, 2, 1); // choose between 0, 1, 2
process = soundChoice,_~+(1):soundfile("mySound",2):!,!,_,_;
```

Note that `soundfile("mySound",2)` has 2 as a hint that the audio is stereo. It's unrelated to the Python side where mySound's dictionary value has 3 numpy arrays.

## Pitch-stretching and Time-stretching with Warp Markers

(For a companion project related to warp markers, see [AbletonParsing](https://github.com/DBraun/AbletonParsing).
)

Time-stretching and pitch-stretching are currently available thanks to [Rubber Band Library](https://github.com/breakfastquay/rubberband/).

```python
engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

playback_processor = engine.make_playbackwarp_processor("drums", load_audio_file("drums.wav"))
playback_processor.time_ratio = 2.  # Play back in twice the amount of time (i.e., slowed down).
playback_processor.transpose = 3.  # Up 3 semitones.

graph = [
  (playback_processor, []),
]

engine.load_graph(graph)
```

You can set an Ableton Live `.asd` file containing warp markers to do beat-matching:

```python
engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

# Suppose that the Ableton clip info thinks the input audio is 120 BPM,
# but we want to play it back at 130 BPM.
engine.set_bpm(130.)
playback_processor = engine.make_playbackwarp_processor("drums", load_audio_file("drum_loop.wav"))
assert(playback_processor.set_clip_file("drum_loop.wav.asd"))

graph = [
  (playback_processor, []),
]

engine.load_graph(graph)
```

This will set several properties:
* `.start_marker` (float) : Start marker position in beats relative to 1.1.1
* `.end_marker` (float) : End marker position in beats relative to 1.1.1
* `.loop_start` (float) : Loop start position in beats relative to 1.1.1
* `.loop_end` (float) : Loop end position in beats relative to 1.1.1
* `.warp_on` (bool) : Whether warping is enabled
* `.loop_on` (bool) : Whether looping is enabled

Any of these properties can be changed after an `.asd` file is loaded.

If `.warp_on` is True, then any value set by `.time_ratio` will be ignored. If `.warp_on` is False, then the `start_marker` and `loop_start` are the first sample of the audio, and the `end_marker` and `loop_end` are the last sample.

With `set_clip_positions`, you can use the same audio clip at multiple places along the timeline.

```python
playback_processor.set_clip_positions([[0., 4., 0.], [5., 9., 1.]])
```

Each tuple of three numbers is the (global timeline clip start, global timeline clip end, local clip offset). Imagine dragging a clip onto an arrangement view. The clip start and clip end are the bounds of the clip on the global timeline. The local clip offset is an offset to the start marker set by the ASD file. In the example above, the first clip starts at 0 beats, ends at 4 beats, and has no offset. The second clip starts at 5 beats, ends at 9 beats, and has a 1 beat clip offset.

## License

DawDreamer is licensed under GPLv3 to make it easier to comply with all of the dependent projects. If you use DawDreamer, you must obey the licenses of [JUCE](https://github.com/juce-framework/JUCE/), [pybind11](https://github.com/pybind/pybind11/), [Libsamplerate](https://github.com/libsndfile/libsamplerate), [Rubber Band Library](https://github.com/breakfastquay/rubberband/), [Steinberg VST2/3](https://www.steinberg.net/vst-instruments/), and [FAUST](https://github.com/grame-cncm/faust).

## Wiki

Please refer to the [Wiki](https://github.com/DBraun/DawDreamer/wiki) for more [examples](https://github.com/DBraun/DawDreamer/wiki/Documentation), the [API documentation](https://dirt.design/DawDreamer), and the [Developer's Guide](https://github.com/DBraun/DawDreamer/wiki/Developer's-Guide). The [tests](https://github.com/DBraun/DawDreamer/tree/main/tests) may also be helpful.

## Thanks to contributors to the original [RenderMan](https://github.com/fedden/RenderMan)
* [fedden](https://github.com/fedden), RenderMan creator
* [jgefele](https://github.com/jgefele)
* [harritaylor](https://github.com/harritaylor)
* [cannoneyed](https://github.com/cannoneyed/)
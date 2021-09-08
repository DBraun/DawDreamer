```
  _____                       _____                                                  
 |  __ \                     |  __ \                                                 
 | |  | |   __ _  __      __ | |  | |  _ __    ___    __ _   _ __ ___     ___   _ __ 
 | |  | |  / _` | \ \ /\ / / | |  | | | '__|  / _ \  / _` | | '_ ` _ \   / _ \ | '__|
 | |__| | | (_| |  \ V  V /  | |__| | | |    |  __/ | (_| | | | | | | | |  __/ | |   
 |_____/   \__,_|   \_/\_/   |_____/  |_|     \___|  \__,_| |_| |_| |_|  \___| |_|   
                                                                                     
* * VST Instruments and Effects with Python * *
```

![Supported Platforms](https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux-green)
[![Test Badge](https://github.com/DBraun/DawDreamer/actions/workflows/all.yml/badge.svg)](https://github.com/DBraun/DawDreamer/actions/workflows/all.yml)
![GitHub Repo stars](https://img.shields.io/github/stars/DBraun/DawDreamer?style=social)

# DawDreamer

DawDreamer is an audio-processing Python framework supporting core [DAW](https://en.wikipedia.org/wiki/Digital_audio_workstation) features:
* Composing graphs of audio processors
* Audio playback
* VST instruments
* VST effects
* [FAUST](http://faust.grame.fr/) effects and polyphonic instruments
* Time-stretching and looping according to Ableton Live warp markers
* Pitch-warping
* Parameter automation
* Rendering multiple processors simultaneously
* Full support on Linux (Dockerfile), macOS, and Windows

DawDreamer's foundation is [JUCE](https://github.com/julianstorer/JUCE), with a user-friendly Python interface thanks to [pybind11](https://github.com/pybind/pybind11). DawDreamer evolved from an earlier VSTi audio "renderer", [RenderMan](https://github.com/fedden/RenderMan).

## Basic Example
```python
import dawdreamer as daw
import numpy as np
from scipy.io import wavfile
import librosa

SAMPLE_RATE = 44100
BUFFER_SIZE = 128 # For speed when not using automation, choose a larger buffer such as 512.
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
# The shape will be numpy.ndarray shaped (2, NUM_SAMPLES)
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

## Building / Installation

### All Platforms

You can find a working Linux Makefile, Visual Studio Solution, and Xcode Project in the `Builds/` folder. If you want to make changes, the best way is to get [JUCE's Projucer](https://juce.com/get-juce) and open `DawDreamer.jucer`.

### Windows

DawDreamer has been tested with Visual Studio 2019 and VS2019 Build Tools (v142).

Install [Python 3.8.x Windows x86-64](https://www.python.org/downloads/release/python-3810/) to `C:/Python38` and set a permanent environment variable `PYTHONPATH` equal to `C:/Python38`.

With the Projucer, open `DawDreamer.jucer`. Use it to create a Visual Studio solution and then build in Release mode. Note the post-build command, which moves the recently built `dawdreamer.dll` to `C:/Python38`. Also copy `thirdparty/libfaust/win-x64/Release/bin/faust.dll` to this directory.

Now you can import dawdreamer:

    python
    >> import dawdreamer as daw
    >> engine = daw.RenderEngine(44100,512)

In order to build in Debug, you must unzip `thirdparty/libfaust/win-x64/Debug/bin/faust.zip` into `faust.dll` in the same folder.

### MacOS

The macOS Deployment Target is 10.15. This can be changed from either the Projucer or in Xcode.

Install [Python 3.9](https://www.python.org/downloads/release/python-395/) with the universal installer.

Use Projucer and DawDreamer.jucer to create an Xcode project or use `Builds/MacOSX/DawDreamer.xcodeproj`. There's a bug in the JUCE projucer app that causes the generated shared object to be suffixed with `dylib`. This means python won't be able to import the module. Until this bug is fixed, change directory into the `Builds/MacOSX/build/<Debug/Release>` (depending on your Xcode scheme) and run:
```bash
mv dawdreamer.so.dylib dawdreamer.so
otool -L dawdreamer.so
install_name_tool -change @rpath/libfaust.2.dylib @loader_path/libfaust.2.dylib dawdreamer.so
otool -L dawdreamer.so
```

Note that the `otool` commands above don't serve a functional purpose. They just display the before and after of the change made with the `install_name_tool` command.

Move `dawdreamer.so` to a directory of your choice. Then find `thirdparty/libfaust/darwin-x64/Release/libfaust.a`, rename it to `libfaust.2.dylib` and place it next to `dawdreamer.so`.

Then try `DawDreamer`:
    
    python3
    >> import dawdreamer as daw
    >> engine = daw.RenderEngine(44100,512)


### Linux

To build `DawDreamer` on Linux, refer to the steps inside the `Dockerfile`.

### Docker

To build an image with the `Dockerfile`, run:

```bash
docker build -t dawdreamer .
```

## API

API documentation is in active development [here](https://ccrma.stanford.edu/~braun/dawdreamer), but in the meantime look at more examples:

```python
import dawdreamer as daw
import librosa
import numpy as np

# SAMPLE_RATE is the number of audio samples per second.
SAMPLE_RATE = 44100  

# Audio is rendered one block at a time, and the size of the block is
# the BUFFER_SIZE. If you're using the set_automation function, you should
# choose a smaller power of 2 buffer size such as 64 or 128.
BUFFER_SIZE = 512

# Note: all paths must be absolute paths
# Note: only VST2 plugins are supported for now (i.e. VST3 plugins do not work, for now)
SYNTH_PLUGIN = "C:/path/to/synth.dll"
REVERB_PLUGIN = "C:/path/to/reverb.dll"
# fxp is a conventional file extension for VST presets.
SYNTH_PRESET = "C:/path/to/preset.fxp"

# a path to a stereo audio file.
VOCALS_PATH = "C:/path/to/vocals.wav"

def load_audio_file(file_path, duration=None):
  sig, rate = librosa.load(file_path, duration=duration, mono=False, sr=SAMPLE_RATE)
  assert(rate == SAMPLE_RATE)
  return sig

# Make an engine. We'll only need one.
engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

# Make a processor and give it the name "my_synth", which we must remember later.
synth = engine.make_plugin_processor("my_synth", SYNTH_PLUGIN)

assert(synth.load_preset(SYNTH_PRESET))

# a list of dictionaries where each dictionary describes a controllable parameter
print(synth.get_plugin_parameters_description())

synth.get_parameter(5)  # Get the parameter at index 5. It'll be between 0 and 1.

synth.set_parameter(5, 0.1234)  # Set the parameter at index 5.

# patch will be a list of tuples. The first value in the tuple is
# the parameter index and the second value is the parameter value.
patch = synth.get_patch()

# You can set all parameters at once with set_patch().
# The following example assumes you have exactly two parameters.
synth.set_patch([(0, 0.1352), (1, 0.2346)])

assert(synth.load_midi("C:/path/to/song.mid"))

# The number of MIDI events in the buffer.
# note-on and note-off events are counted separately.
print(synth.n_midi_events)

synth.clear_midi()

# Add notes one at a time.
synth.add_midi_note(60, 127, 0.5, .25)  # (MIDI note, velocity, start sec, duration sec)
# 60 is middle C. Velocity is an integer from 0-127.

# Examples of other Processors
vocals = load_audio_file(VOCALS_PATH, duration=10.)
playback_processor = engine.make_playback_processor("my_vocals", vocals)
playback_processor.set_data(vocals) # You can do this anytime.

threshold = 0. # dB level of threshold
ratio = 2. # greater than or equal to 1.
attack = 2. # attack of compressor in milliseconds
release = 50. # release of compressor in milliseconds

compressor_processor = engine.make_compressor_processor("my_compressor", threshold, ratio, attack, release)
# CompressorProcessor has getters/setters
compressor_processor.threshold = threshold
compressor_processor.ratio = ratio
compressor_processor.attack = attack
compressor_processor.release = release

# The add processor sums signals together.
# When you add it to a graph, you probably want to use multiple inputs.
add_processor = engine.make_add_processor("my_add")

# You can also use the add processor to adjust gain levels.
# Assuming we will provide two inputs to this processor, we can construct
# the add processor and pass an array of gain levels at the same time.
add_processor = engine.make_add_processor("my_add", [0.5, 0.8])
add_processor.gain_levels = [0.7, 0.7] # Adjust gain levels whenever you want.

# Basic reverb processor from JUCE.
room_size = 0.5
damping = 0.5
wet_level = 0.33
dry_level = 0.4
width = 1.
reverb_processor = engine.make_reverb_processor("my_reverb", room_size, damping, wet_level, dry_level, width)
# ReverbProcessor has getters/setters
reverb_processor.room_size = room_size
reverb_processor.damping = damping
reverb_processor.wet_level = wet_level
reverb_processor.dry_level = dry_level
reverb_processor.width = width

# Basic filter processor from JUCE.
filter_mode = "high" # "low", "high", "band", "low_shelf", "high_shelf", "notch"
freq = 1000.0  # cutoff frequency in Hz.
q = 0.707107 # safe choice is 1./rad(2)=0.707107.
gain = 1. # gain values only matter when the mode is low_shelf or high_shelf.

filter_processor = engine.make_filter_processor("my_filter", filter_mode, freq, q, gain)
# FilterProcessor has getters/setters
reverb_processor.mode = filter_mode
reverb_processor.freq = freq
reverb_processor.q = q
reverb_processor.gain = gain

panner_processor = engine.make_panner_processor("my_panner", "linear", 0.)
# the rules: "linear", "balanced", "sin3dB", "sin4p5dB", "sin6dB", "squareRoot3dB", "squareRoot4p5dB"
panner_processor.rule = "balanced" 
panner_processor.pan = -.5 # -1. is fully left and 1. is fully right

delay_rule = "linear" # only linear is supported right now
delay_ms = 200. # delay in milliseconds
delay_wet = .3 # 0 means use all of original signal and none of the "wet" delayed signal. 1. is the opposite.
delay_processor = engine.make_delay_processor("my_delay", delay_rule, delay_ms, delay_wet)
# delay_processor.rule = "linear" # modifying the rule is not supported yet.
delay_processor.delay = delay_ms
delay_processor.wet = delay_wet

# Some plugins can take more than one input. For example, a sidechain processor plugin
# can reduce the volume of the first input according to the second input's volume.
sidechain_processor = engine.make_plugin_processor("sidechain_compressor", "path/to/sidechain.dll")

# Graph idea is based on https://github.com/magenta/ddsp#processorgroup-with-a-list
# A graph is a meaningfully ordered list of tuples.
# In each tuple, the first item is an audio processor.
# The second item is this audio processor's list of input processors.
# You must create each processor with a unique name
# and refer to these unique names in the lists of inputs.
# The audio from the last tuple's processor will be accessed automatically later by engine.get_audio()
graph = [
  (synth, []),
  (playback_processor, []),
  (panner_processor, ["my_vocals"]),
  (reverb_processor, ["my_panner"]),
  (add_processor, ["my_synth", "my_reverb"]),
  (compressor_processor, ["my_add"]),
  (filter_processor, ["my_compressor"]),
  (delay_processor, ["my_filter"]),
  (sidechain_processor, ["my_filter", "my_vocals"])
]

engine.load_graph(graph)

engine.render(10.)  # Render 10 seconds audio.
audio = engine.get_audio()  # Returns numpy.ndarray shaped (2, NUM_SAMPLES)

# Even after a render, we can still modify our processors and re-render the graph.
# All of our MIDI is still loaded.
synth.load_preset("C:/path/to/other_preset.fxp")
reverb_processor.freq = 2000.0 # change a parameter on a processor.
playback_processor.set_data(load_audio_file("piano.wav", duration=10.))  # give different waveform
engine.render(10.)  # render audio again!

# or load a new graph
graph = [
  (synth, []),
]
engine.load_graph(graph)
engine.render(10.)

```

## FAUST

Faust on Linux relies on the Ubuntu package service, so if you used the Dockerfile, no extra steps are necessary. For Windows and macOS, Faust features depend on precompiled libraries in `thirdparty/libfaust`. If you'd like to compile these yourself, please follow the instructions for [TD-FAUST](https://github.com/DBraun/TD-Faust/) (Downloading TouchDesigner is not necessary).

### Windows

Run the latest `win64.exe` installer from FAUST's [releases](https://github.com/grame-cncm/faust/releases). After installing, copy the `.lib` files from `C:/Program Files/Faust/share/faust/` to `C:/share/faust/`. The reason is that we're using `C:/Python38/python.exe`, so the sibling directory would be `C:/share/faust`. If you're running python from a different location, you can determine the different location for the `share/faust/*.lib` files.

### macOS

Open the latest `.dmg` installer from FAUST's [releases](https://github.com/grame-cncm/faust/releases). Copy the `.lib` files from `Faust-2.X.X/share/faust/` to either `/usr/local/share/faust/*.lib` or `/usr/share/faust/*.lib`.

### Using FAUST processors

Let's start by looking at FAUST DSP files, which end in `.dsp`. For convenience, the standard library is always imported, so you don't need to `import("stdfaust.lib");` All code must result in a `process` with 2 outputs and an even number of inputs. Here's an example using a demo stereo reverb:

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

Polyphony is supported too. You simply need to provide DSP code that refers to correctly named parameters such as `freq` or `note`, `gain`, and `gate`. For more information, see the FAUST [manual](https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters). In DawDreamer, you must set the number of voices on the processor to 1 or higher. 0 disables polyphony. Refer to `tests/test_faust_poly.py`.

## Pitch-stretching and Time-stretching with Warp Markers

Time-stretching and pitch-stretching are currently available thanks to [Rubber Band Library](https://github.com/breakfastquay/rubberband/).

```python
SAMPLE_PATH = "drums.wav"

engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

playback_processor = engine.make_playbackwarp_processor("drums", load_audio_file(SAMPLE_PATH))
playback_processor.time_ratio = 2.  # Play back in twice the amount of time (i.e., slowed down).
playback_processor.transpose = 3.  # Up 3 semitones.

graph = [
  (playback_processor, []),
]

assert(engine.load_graph(graph))
```

You can set an Ableton Live `.asd` file containing warp markers to do beat-matching:

```python
engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

# Suppose that the Ableton clip info thinks the input audio is 120 BPM,
# but we want to play it back at 130 BPM.
engine.set_bpm(130.)

playback_processor.set_data(load_audio_file("synth_loop.wav"))
assert(playback_processor.set_clip_file("synth_loop.wav.asd"))

graph = [
  (playback_processor, []),
]

assert(engine.load_graph(graph))
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

## Tests

Go to the `tests` directory and run `pytest .`

## License

DawDreamer is licensed under GPLv3 to make it easier to comply with all of the dependent projects. If you use DawDreamer, you must obey the licenses of JUCE, pybind11, Libsamplerate, Rubber Band Library, Steinberg VST2/3, FAUST.

## Release Notes

[Release Notes](https://github.com/DBraun/DawDreamer/wiki/Release-Notes)

## Thanks to contributors to the original [RenderMan](https://github.com/fedden/RenderMan)
* [fedden](https://github.com/fedden), RenderMan creator
* [jgefele](https://github.com/jgefele)
* [harritaylor](https://github.com/harritaylor)
* [cannoneyed](https://github.com/cannoneyed/)
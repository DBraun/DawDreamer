```
  _____                       _____                                                  
 |  __ \                     |  __ \                                                 
 | |  | |   __ _  __      __ | |  | |  _ __    ___    __ _   _ __ ___     ___   _ __ 
 | |  | |  / _` | \ \ /\ / / | |  | | | '__|  / _ \  / _` | | '_ ` _ \   / _ \ | '__|
 | |__| | | (_| |  \ V  V /  | |__| | | |    |  __/ | (_| | | | | | | | |  __/ | |   
 |_____/   \__,_|   \_/\_/   |_____/  |_|     \___|  \__,_| |_| |_| |_|  \___| |_|   
                                                                                     
* * Digital Audio Workstation (DAW) without a GUI * *
```

| `build` |
|:-------:|
| [![Build Status](https://travis-ci.com/DBraun/DawDreamer.svg?token=K9QCyFBiJjeEFq8GLsFx&branch=master)](https://travis-ci.com/DBraun/DawDreamer) |

# DawDreamer

DawDreamer is an audio-processing Python framework supporting core DAW features such as audio playback, VST MIDI instruments, and VST effects. DawDreamer is written with [JUCE](https://github.com/julianstorer/JUCE), with a user-friendly Python interface thanks to [pybind11](https://github.com/pybind/pybind11). DawDreamer draws from an earlier VSTi audio "renderer", [RenderMan](https://github.com/fedden/RenderMan).

## Basic Example
```python
import dawdreamer as daw
import numpy as np
from scipy.io import wavfile
import librosa

SAMPLE_RATE = 44100
BUFFER_SIZE = 512
SYNTH_PLUGIN = "C:/path/to/synth.dll"  # .vst3 files work too.
SYNTH_PRESET = "C:/path/to/preset.fxp"
REVERB_PLUGIN = "C:/path/to/reverb.dll"  # .vst3 files work too.
VOCALS_PATH = "C:/path/to/vocals.wav"
PIANO_PATH = "C:/path/to/piano.wav"

def load_audio_file(file_path, duration=None):
  sig, rate = librosa.load(file_path, duration=duration, mono=False, sr=SAMPLE_RATE)
  assert(rate == SAMPLE_RATE)
  return sig

# Make an engine. We'll only need one.
engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

vocals = load_audio_file(VOCALS_PATH, duration=10.)
piano = load_audio_file(PIANO_PATH, duration=10.)

# Make a processor and give it the name "my_synth", which we must remember later.
synth = engine.make_plugin_processor("my_synth", SYNTH_PLUGIN)
synth.load_preset(SYNTH_PRESET)
synth.set_parameter(5, 0.1234)  # override a specific parameter.
synth.load_midi("C:/path/to/song.mid")
# We can also add notes one at a time.
synth.add_midi_note(67, 127, 0.5, .25)  # (MIDI note, velocity, start sec, duration sec)

# Graph idea is based on https://github.com/magenta/ddsp#processorgroup-with-a-list
# A graph is a meaningfully ordered list of tuples.
# In each tuple, the first item is an audio processor.
# The second item is this audio processor's list of input processors.
# You must create each processor with a unique name
# and refer to these unique names in the lists of inputs.
# The audio from the last tuple's processor will be accessed automatically later by engine.get_audio()
graph = [
  (synth, []),  # synth takes no inputs, so we give an empty list.
  (engine.make_reverb_processor("reverb"), ["my_synth"]), # Apply JUCE reverb to the synth named earlier
  (engine.make_plugin_processor("more_reverb", REVERB_PLUGIN), ["reverb"]), # Apply VST reverb
  (engine.make_playback_processor("vocals", vocals), []), # Playback has no inputs.
  (engine.make_filter_processor("filter", "high", 7000.0, .5, 1.), ["vocals"]), # High-pass filter
  (engine.make_add_processor("added"), ["more_reverb", "filter"])
]

engine.load_graph(graph)

engine.render(10.)  # Render 10 seconds audio.
audio = engine.get_audio()  # Returns python list of lists of shape (2, NUM_SAMPLES)
audio = np.array(audio, np.float32).transpose()
wavfile.write('my_song.wav', SAMPLE_RATE, audio)

# You can modify processors without recreating the graph.
synth.load("C:/path/to/other_preset.fxp")
engine.render(10.)  # render audio again!
```

## Building / Installation

### All Platforms

The goal is to have a working Linux Make file, Visual Studio Solution, and Xcode Project in the builds folder. If the build for your platform isn't working, you should use [JUCE's Projucer](https://juce.com/get-juce) to open `DawDreamer.jucer` and create it.

### Windows

I've tested with Visual Studio 2019 and VS2019 Build Tools (v142).

On Windows, the JUCE project makes several assumptions about having a Python 3.8 virtual environment located at `C:/Python38dawdreamer`.  You can use a different virtual environment as long as you modify all references in `DawDreamer.jucer`, which is actually a text file.

Install [Python 3.8.x Windows x86-64](https://www.python.org/downloads/release/python-385/) to `C:/Python38`. Use [virtualenv](https://docs.python-guide.org/dev/virtualenvs/) to create a virtual environment: 
`python -m venv C:/Python38dawdreamer`

In this repo, use `env.bat` to activate the virtual environment. In this window, install the python module requirements:
`pip install -r requirements.txt`

With the Projucer, open `DawDreamer.jucer`. Use it to create a Visual Studio solution and then build in Release mode. Note the post-build command, which moves the recently built `dawdreamer.dll` to `C:/Python38dawdreamer`.

Now you can activate the virtual environment and import dawdreamer:

    C:/Python38dawdreamer/Scripts/activate.bat
    python
    >> import dawdreamer as daw
    >> engine = daw.RenderEngine(44100,512)

### MacOS

First, get [brew.sh](https://brew.sh/). In a Mac OS Terminal, place the following and hit enter.
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```
Use pyenv to install Python 3.8.5:
```bash
brew update && brew upgrade pyenv && env PYTHON_CONFIGURE_OPTS="--enable-framework" pyenv install 3.8.5
```
Use Projucer and DawDreamer.jucer to create an Xcode project or use `Builds/MacOSX/DawDreamer.xcodeproj`. There's a bug in the JUCE projucer app that causes the generated shared object to be suffixed with `dylib`. This means python wont be able to import the module. Until this bug is fixed, change directory into the `Builds/MacOSX/build/<Debug/Release>` (depending on your Xcode scheme) and run:
```bash
mv dawdreamer.so.dylib dawdreamer.so
```
Move `dawdreamer.so` to a directory of your choice. Open a terminal window to this directory and type the follow to create a reusable virtual environment titled `myVenv` or your choice.
```bash
pyenv virtualenv 3.8.5 myVenv
```
Each time you want to use `DawDreamer`, come to this directory and activate the environment:
```bash
pyenv activate myVenv
```
Then try `DawDreamer`:
    
    python
    >> import dawdreamer as daw
    >> engine = daw.RenderEngine(44100,512)


### Linux

Linux hasn't been tested since the transition from RenderMan, but it might work.

Install [Python 3.8.x](https://www.python.org/downloads/source/).

JUCE itself has a list of dependancies for Linux; it's a very big library - if you don't know it you should definitely take some time out to check it out! Depending on your distribution and setup you may already have some / all of the following libraries. If you are on Ubuntu, the following commands will install your dependancies. Find the respective packages for other distros using Google please!

```bash
sudo apt-get -y install llvm
sudo apt-get -y install clang
sudo apt-get -y install libfreetype6-dev
sudo apt-get -y install libx11-dev
sudo apt-get -y install libxinerama-dev
sudo apt-get -y install libxrandr-dev
sudo apt-get -y install libxcursor-dev
sudo apt-get -y install mesa-common-dev
sudo apt-get -y install libasound2-dev
sudo apt-get -y install freeglut3-dev
sudo apt-get -y install libxcomposite-dev
sudo apt-get -y install libcurl4-gnutls-dev
```

Well done! You've made it this far! Should you still have problems, which is always a possibility with Linux, a good place to start is the JUCE forums, particularly [here](https://forum.juce.com/t/juce-4-2-1-setup-on-apt-based-linux-ubuntu-16-04-lts-mint-elementary-os-freya/17164) and [here](https://forum.juce.com/t/list-of-juce-dependencies-under-linux/15121).

To build the library for Linux, change to the right directory and run make:
```
cd Builds/LinuxMakefile/
make
```

## API

Proper documentation will be created eventually, but in the meantime look at more examples:

```python
import dawdreamer as daw
import librosa

# SAMPLE_RATE is the number of audio samples per second.
SAMPLE_RATE = 44100  

# Audio is rendered one block at a time, and the size of the block is
# the BUFFER_SIZE. Eventually, DawDreamer will support animating parameters
# over time, and you'll probably want a smaller buffer size if you're animating.
BUFFER_SIZE = 512

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
release = 50. # attack of compressor in milliseconds

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
panner_processor.rule = "balanced" # "linear", "balanced", "sin3dB", "sin4p5dB", "sin6dB", "squareRoot3dB", "squareRoot4p5dB"
panner_processor.pan = -.5 # -1. is fully left and 1. is fully right

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
  (filter_processor, ["my_compressor"])
]

engine.load_graph(graph)

engine.render(10.)  # Render 10 seconds audio.
audio = engine.get_audio()  # Returns python list of lists. The shape is (2, NUM_SAMPLES)

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

## License

If you use DawDreamer, you must obey the licenses of JUCE, pybind11, and Steinberg VST2/3, and Maximillian.

## Contributors to the original [RenderMan](https://github.com/fedden/RenderMan)
* [fedden](https://github.com/fedden), RenderMan creator
* [jgefele](https://github.com/jgefele)
* [harritaylor](https://github.com/harritaylor)
* [cannoneyed](https://github.com/cannoneyed/)

If you use this code for academic works, feel free to refer to the DawDreamer repo and RenderMan's DOI:
[![DOI](https://zenodo.org/badge/82790125.svg)](https://zenodo.org/badge/latestdoi/82790125)

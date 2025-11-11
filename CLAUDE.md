# DawDreamer Architecture & Development Guide

## Project Overview

DawDreamer is a **Digital Audio Workstation (DAW) framework for Python**. It enables programmatic audio processing with support for VST plugins, Faust DSP code, and complex audio routing graphs. Licensed under GPLv3.

### Key Capabilities
- Composable audio processor graphs (DAG-based)
- VST 2/3 instrument and effect support with UI and state management
- Faust effects and polyphonic instruments (Box and Signal APIs)
- Time-stretching, looping, pitch-warping via RubberBand
- Parameter automation at audio-rate and PPQN (pulses-per-quarter-note) rates
- MIDI playback and file export
- Multi-processor parallel rendering
- Transpilation: Faust to JAX/Flax, C++, Rust, WebAssembly
- Full platform support: macOS (Apple Silicon/Intel), Windows, Linux, Google Colab

**Platforms**: macOS 11.0+, Windows (x86_64), Linux (x86_64)
**Python Support**: 3.10-3.12 (3.8-3.12 in wheel metadata)

---

## Architecture Overview

### High-Level Structure

```
DawDreamer/
├── Source/                          # C++ core implementation
│   ├── source.cpp                   # pybind11 Python bindings (entry point)
│   ├── RenderEngine.{h,cpp}         # Main audio engine & DAG execution
│   ├── ProcessorBase.{h,cpp}        # Abstract processor base class
│   ├── *Processor.h                 # Processor implementations (Faust, Plugin, Playback, etc.)
│   ├── FaustProcessor.{h,cpp}       # Faust DSP integration
│   ├── FaustBoxAPI.{h,cpp}          # Faust Box API wrapper
│   ├── FaustSignalAPI.{h,cpp}       # Faust Signal API wrapper
│   ├── PluginProcessor.{h,cpp}      # VST plugin host
│   ├── PlaybackWarpProcessor.{h,cpp}# Time-stretch/pitch-warp (RubberBand)
│   └── Sampler/                     # Multi-sampler with UI
├── Builds/                          # Platform-specific build configs
│   ├── MacOSX/DawDreamer.xcodeproj/ # Xcode project
│   ├── VisualStudio2022/            # Visual Studio solution
│   └── LinuxMakefile/               # Linux Makefile
├── DawDreamer.jucer                 # JUCE Projucer configuration
├── build_macos.sh                   # macOS build script
├── build_linux.sh                   # Linux build script
├── setup.py                         # Python wheel packaging
├── dawdreamer/                      # Python package (installed wheel content)
│   ├── __init__.py
│   ├── dawdreamer.so (or .pyd/.dll) # Compiled C++ extension module
│   ├── architecture/                # Faust architecture files
│   └── faustlibraries/              # Faust standard library
├── tests/                           # pytest test suite
│   ├── dawdreamer_utils.py          # Shared test utilities
│   ├── test_*.py                    # Test modules
│   ├── assets/                      # Test audio files
│   └── faust_dsp/                   # Faust DSP test files
└── thirdparty/                      # External dependencies (git submodules)
    ├── JUCE/                        # Audio framework
    ├── pybind11/                    # C++ ↔ Python bindings
    ├── faust/                       # Faust DSP compiler
    ├── libsamplerate/               # High-quality sample rate conversion
    ├── rubberband/                  # Time-stretch/pitch-shift library
    └── faustlibraries/              # Faust standard library
```

### Core Components & Responsibilities

#### 1. **RenderEngine** (Source/RenderEngine.{h,cpp})
- Central orchestrator managing the audio processing pipeline
- Implements `AudioPlayHead` interface for tempo/timing
- Manages DAG (Directed Acyclic Graph) of processors
- Handles audio buffer allocation and rendering lifecycle
- Tracks BPM and automation (audio-rate and PPQN-based)
- Exposes factory methods: `make_*_processor()` for creating each processor type

**Key Methods:**
- `loadGraph(DAG)` - validates and loads processor graph
- `render(duration, isBeats)` - processes audio for specified duration
- `getAudioFrames()` - returns rendered audio as numpy array
- `setBPM()`, `setBPMwithPPQN()` - tempo management

#### 2. **ProcessorBase** (Source/ProcessorBase.{h,cpp})
- Abstract base class for all audio processors
- Manages input/output channels, parameter automation, recording
- Handles parameter metadata and JUCE parameter tree
- Provides automation record/playback at both audio-rate and PPQN-rate

**Key Methods:**
- `setAutomation(parameter_name, data, ppqn)` - set parameter automation
- `getAutomation()` - retrieve automation data
- `process()` - virtual method for subclasses to implement

#### 3. **Processor Implementations** (Various .h files)
Each processor is a `ProcessorBase` subclass handling a specific audio function:

- **FaustProcessor** - Compiles/executes Faust DSP code in real-time
- **PluginProcessor** - Hosts VST 2/3 plugins (instruments and effects)
- **PlaybackProcessor** - Plays back audio buffers from memory
- **PlaybackWarpProcessor** - Time-stretches/pitch-shifts audio (RubberBand)
- **FilterProcessor** - Built-in IIR filter (low/high/band-pass)
- **OscillatorProcessor** - Simple sine wave generator (test utility)
- **CompressorProcessor** - Dynamics processor
- **ReverbProcessor** - Convolver-based reverb
- **DelayProcessor** - Multi-tap delay line
- **PannerProcessor** - Stereo panning
- **AddProcessor** - Mixer/summer
- **SamplerProcessor** - Sampled instrument with MPE support

#### 4. **Faust Integration** (FaustProcessor.{h,cpp}, FaustBoxAPI.cpp, FaustSignalAPI.cpp)
- **FaustProcessor**: Real-time compilation and execution of Faust DSP strings
  - Embeds libfaust compiler for on-the-fly code generation
  - Supports Box and Signal API modes
  - Transpiles Faust to C++ backend for performance
  - Extracts parameters for GUI automation

- **FaustBoxAPI** & **FaustSignalAPI**: Expose Faust's two programming paradigms
  - Box API: Component-based, for building instrument architectures
  - Signal API: Signal-processing focused, lower-level

#### 5. **Plugin Host** (PluginProcessor.{h,cpp})
- Loads and hosts VST 2/3 plugins dynamically
- Wraps JUCE's plugin hosting infrastructure
- Manages plugin state loading/saving
- Provides synchronized parameter automation

#### 6. **Python Bindings** (Source/source.cpp)
- **pybind11-based** C++ → Python interface
- Exposes all processors, RenderEngine, parameters
- Converts numpy arrays ↔ C++ audio buffers
- Handles memory safety and reference counting

---

## Build System Architecture

### Overview

DawDreamer uses a **multi-step build process**:
1. **Download dependencies**: Prebuilt libfaust libraries (via Python script)
2. **Build C++ dependencies**: libsamplerate (via CMake)
3. **Build C++ extension**: DawDreamer itself (via Xcode/VS/Make)
4. **Package Python module**: Combine compiled extension with Python wrapper (via setup.py)

**Key insight**: Unlike typical Python packages that build with `pip install`, DawDreamer requires pre-building the C++ components before running setup.py. This is because JUCE-based projects use platform-specific build systems (Xcode, Visual Studio, Makefiles) rather than setuptools' build process.

### Build Tools & Dependencies

**C++ Side:**
- **JUCE 8.x**: Audio framework providing MIDI, VST host, audio I/O
- **Faust**: DSP language compiler (embedded)
- **libsamplerate**: SRC for time-stretching audio
- **RubberBand**: Advanced time/pitch modification library
- **pybind11**: C++11 → Python 3 bindings

**Python Side:**
- **setuptools**: Wheel packaging
- **Python 3.10-3.12**: Required headers and libs

**Build Flow by Platform:**

#### macOS (Xcode)
```bash
# Prerequisites:
# - Xcode with command line tools
# - CMake (can install via Homebrew: brew install cmake)
# - Faust compiler (can install via Homebrew: brew install faust)
# - Python 3.10-3.12 installed

# Setup environment
export PYTHONMAJOR=3.11
export pythonLocation=/Library/Frameworks/Python.framework/Versions/3.11
export ARCHS=arm64  # or x86_64 for Intel Macs

# Step 1: Initialize git submodules (required on first build)
git submodule update --init --recursive

# Step 2: Download prebuilt libfaust libraries
cd thirdparty/libfaust
python3.11 download_libfaust.py
cd ../..

# Step 3: Build libsamplerate dependency
cd thirdparty/libsamplerate
mkdir -p build_release
cd build_release
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="$ARCHS"
make -j$(sysctl -n hw.ncpu)
cd ../../..

# Step 4: Build main library via Xcode
xcodebuild -project Builds/MacOSX/DawDreamer.xcodeproj \
  -target "DawDreamer - Dynamic Library" \
  -configuration Release-$ARCHS \
  clean build

# Output: Builds/MacOSX/build/Release-<ARCH>/dawdreamer.so.dylib

# Step 5: Copy the built library (setup.py expects .so not .dylib)
cp Builds/MacOSX/build/Release-$ARCHS/dawdreamer.so.dylib \
   Builds/MacOSX/build/Release-$ARCHS/dawdreamer.so

# Step 6: Build Python package
ARCHS=$ARCHS python3.11 setup.py develop
# Or for wheel: python3.11 -m build --wheel

# Verify installation
python3.11 -c "import dawdreamer as daw; print('Success!')"
```

**Common Issues & Solutions:**

1. **"Configuration Release is not in the project"**: Use `Release-arm64` or `Release-x86_64` instead of just `Release`

2. **"library 'samplerate' not found"**: Build libsamplerate first (Step 3)

3. **"library 'faustwithllvm' not found"**: Download libfaust prebuilt libraries (Step 2)

4. **"dawdreamer.so not found" during setup.py**: The Xcode build outputs `dawdreamer.so.dylib` but setup.py expects `dawdreamer.so`. Copy/rename the file (Step 5)

5. **Wrong architecture**: Make sure `ARCHS` matches your system (arm64 for Apple Silicon, x86_64 for Intel)

#### Linux (CMake/Makefile)
```bash
# Setup Python paths
export PYTHONLIBPATH=/usr/lib/python3.10
export PYTHONINCLUDEPATH=/usr/include/python3.10

# Build libsamplerate
cd thirdparty/libsamplerate
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release
make -C build_release
cd ../..

# Build via Makefile
cd Builds/LinuxMakefile
make CONFIG=Release CXXFLAGS="-I$PYTHONINCLUDEPATH" LDFLAGS="-L$PYTHONLIBPATH"
cd ../..
# Output: dawdreamer/dawdreamer.so
```

#### Windows (Visual Studio 2022)
```bash
# Setup environment (x64 Native Tools Command Prompt)
set PYTHONMAJOR=3.11
set pythonLocation=C:\Python311

# Build libsamplerate
cd thirdparty\libsamplerate
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release
cmake --build build_release --config Release
cd ..\..\

# Build solution
msbuild Builds/VisualStudio2022/DawDreamer.sln /property:Configuration=Release
# Output: Builds/VisualStudio2022/x64/Release/Dynamic Library/dawdreamer.dll
```

### Wheel Packaging (setup.py)
1. Expects pre-compiled native binary at expected path
2. Copies binary to `dawdreamer/` package directory (renamed to .so/.pyd/.dll)
3. Includes Faust architecture files and faustlibraries as package data
4. Creates multi-platform wheel with native extension
5. Uses `BinaryDistribution` class to force platform-specific wheel tag

**Key setup.py Behavior:**
- Reads version from `DawDreamer.jucer` XML
- Platform-specific binary path detection
- Bundles licenses and Faust resources
- Creates non-zip-safe wheel (binary extension)

---

## Key Build Files Explained

### DawDreamer.jucer (JUCE Project File)
- XML configuration for JUCE Projucer tool
- Defines C++ header paths (includes pybind11, Faust, libsamplerate)
- Specifies compiler flags and linked libraries:
  - macOS: `samplerate`, `faustwithllvm`
  - Linux: Similar deps via Makefile
  - Windows: Via Visual Studio project
- Defines build configurations (Debug/Release for each arch)
- Platform-specific deployment targets (macOS 12.0+)

### Builds/MacOSX/DawDreamer.xcodeproj
- Xcode project generated from .jucer file
- Build phases include linking libsamplerate and Faust libraries
- Outputs to: `Builds/MacOSX/build/Release-<ARCH>/dawdreamer.so.dylib`
- **Important**: Setup.py expects `dawdreamer.so` (not `.dylib`), so you must copy/rename the output file
- Build configurations available: `Debug-x86_64`, `Release-x86_64`, `Release-arm64`

### Builds/LinuxMakefile/
- Auto-generated Makefile from JUCE Projucer
- Typical compilation pattern: `make CONFIG=Release`
- Uses `strip` to reduce binary size

---

## Development Workflow

### Prerequisites

**macOS:**
- Xcode with command line tools (tested with Xcode 16.0.1, older versions should work)
- CMake (`brew install cmake`)
- Faust compiler (`brew install faust`)
- Python 3.10-3.12 with development headers
- Architecture: Apple Silicon (arm64) or Intel (x86_64)

**Linux:**
- build-essential, clang
- libboost-dev
- X11 development libraries (libx11-dev, libxext-dev, libxrandr-dev, etc.)
- ALSA/JACK audio development libraries
- CMake, Python 3.10-3.12 with development headers
- Faust compiler

**Windows:**
- Visual Studio 2022 with C++ desktop development workload
- CMake
- Python 3.10-3.12 with development libraries
- Faust compiler

### Getting Started

```bash
# Clone and initialize submodules
git clone https://github.com/DBraun/DawDreamer.git
cd DawDreamer
git submodule update --init --recursive

# Download libfaust (required)
cd thirdparty/libfaust
python download_libfaust.py
cd ../..
```

### Quick Start: Building for Development (macOS)

For local development on macOS with Python 3.11, follow these steps:

```bash
# 1. Set your architecture (arm64 for Apple Silicon, x86_64 for Intel)
export ARCHS=arm64

# 2. Build libsamplerate dependency
cd thirdparty/libsamplerate
mkdir -p build_release && cd build_release
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=$ARCHS
make -j$(sysctl -n hw.ncpu)
cd ../../..

# 3. Build DawDreamer C++ library
xcodebuild -project Builds/MacOSX/DawDreamer.xcodeproj \
  -target "DawDreamer - Dynamic Library" \
  -configuration Release-$ARCHS \
  build

# 4. Copy output file (Xcode builds .dylib but setup.py expects .so)
cp Builds/MacOSX/build/Release-$ARCHS/dawdreamer.so.dylib \
   Builds/MacOSX/build/Release-$ARCHS/dawdreamer.so

# 5. Install in development mode
ARCHS=$ARCHS python3.11 setup.py develop

# 6. Test it
python3.11 -c "import dawdreamer as daw; \
  engine = daw.RenderEngine(44100, 512); \
  print('Success! DawDreamer is working.')"
```

**After making C++ changes:**
```bash
# Rebuild C++ only (skip steps 1-2)
xcodebuild -project Builds/MacOSX/DawDreamer.xcodeproj \
  -target "DawDreamer - Dynamic Library" \
  -configuration Release-$ARCHS \
  build

# Copy the rebuilt library
cp Builds/MacOSX/build/Release-$ARCHS/dawdreamer.so.dylib \
   Builds/MacOSX/build/Release-$ARCHS/dawdreamer.so

# Reinstall (fast since it just updates the .so file)
ARCHS=$ARCHS python3.11 setup.py develop

# Test your changes
python3.11 -c "import dawdreamer as daw; ..."
```

**Building a wheel for distribution:**
```bash
# After completing steps 1-4 above:
ARCHS=$ARCHS python3.11 -m build --wheel

# The wheel will be in dist/
# Install it with: pip install dist/dawdreamer-*.whl
```

### Modifying C++ Source Code

**Workflow:**
1. Edit source files in `Source/`
2. Open `DawDreamer.jucer` with JUCE Projucer (free download)
   - Projucer regenerates platform-specific build files
   - DO NOT edit .xcodeproj/.sln files directly
3. Rebuild via platform-specific build script or IDE
4. For wheel testing: run tests immediately in `/tests` directory (binary copied there)

**Adding a New Processor:**
1. Create `Source/MyProcessor.h` (header-only or with .cpp)
2. Implement as `public ProcessorBase` subclass
3. Add pybind11 binding in `Source/source.cpp`
4. Add factory method to RenderEngine
5. Update DawDreamer.jucer to include new files
6. Rebuild

### Modifying Python Bindings
- Edit `Source/source.cpp` (the pybind11 glue code)
- Rebuild C++ extension
- Changes affect module structure and Python API

### Testing Approach
```bash
# Run all tests
cd tests
python -m pytest -s

# Run specific test file
python -m pytest -s test_faust_processor.py

# Run specific test
python -m pytest -s test_faust_processor.py::test_faust_passthrough
```

**Test Structure:**
- Each test imports `dawdreamer_utils` for common utilities
- Tests use fixture data in `assets/` (audio files)
- Faust DSP snippets in `faust_dsp/`
- Output files written to `output/` directory
- Parametrized tests common (testing multiple buffer sizes, durations)

**Key Test Utilities** (dawdreamer_utils.py):
- `load_audio_file()` - loads audio via librosa/soundfile
- `make_sine()` - generates test sine wave
- `render()` - helper to render engine and optionally save to file
- Constants: `SAMPLE_RATE=44100`, asset/output paths

### Understanding the Rendering Pipeline

**Typical user code:**
```python
import dawdreamer as daw
import numpy as np

# 1. Create engine with sample rate and block size
engine = daw.RenderEngine(44100, 512)

# 2. Create processors (factory methods on engine)
synth = engine.make_faust_processor("synth")
synth.set_dsp_string("process = hslider(...) : os.osc;")

effect = engine.make_plugin_processor("reverb", "path/to/plugin.dll")

# 3. Build processor graph as list of (processor, input_names)
graph = [
    (synth, []),           # synth takes no input
    (effect, ["synth"])    # effect processes synth output
]
engine.load_graph(graph)

# 4. Render
engine.set_bpm(120)
engine.render(4., beats=True)  # 4 beats

# 5. Retrieve audio
audio = engine.get_audio()  # shape (2, samples)
```

**Internal flow:**
1. `load_graph()` validates DAG, prepares processor connections
2. `render()` allocates buffers, calls `process()` on each processor in dependency order
3. Each processor reads from input buffers, writes to output buffers
4. Automation is applied during `process()` based on tempo/time
5. Audio returned as numpy array (channels × samples)

---

## Important Patterns & Conventions

### Parameter Automation
- **Audio-rate**: Automation array size = samples in render (one value per sample)
- **PPQN-rate**: Automation array size = pulses in render (at specified PPQN)
  - Engine internally converts PPQN to audio-rate using BPM
  - Example: 960 PPQN, 120 BPM, 4 beats = 3840 automation points

### Processor Naming
- Each processor must have a **unique name** (arbitrary string)
- Used for graph connectivity: `(effect_proc, ["input_proc_name"])`
- Prevent name collisions; no automatic uniqueness checking

### Plugin State Management
- VST plugins store state in XML or binary format
- State can be saved/loaded via `set_plugin_state()`, `get_plugin_state()`
- State includes all parameter values and UI positions

### Faust Compilation
- Faust code compiled to C++ at runtime (not pre-compiled)
- Compilation happens on first `compile()` or implicit in `set_dsp_string()`
- Cached internally; recompilation only if code changes
- Parameters extracted from Faust metadata (hslider, hgroup, etc.)

### Multi-Channel Audio
- All processors support arbitrary channel counts
- Input channels must match previous processor output channels
- Playback processor defines initial channel count
- Can use AddProcessor to sum/route channels

### Sample Rate Handling
- Specified at RenderEngine creation time
- All processors use same sample rate (no per-processor rate conversion)
- PlaybackWarpProcessor can internally resample audio for time-stretching

### Memory & Performance
- Audio buffers allocated once per render call
- Processors reuse buffers between renders (efficient for batch processing)
- No hidden copies; direct pointer passing between processors
- Recording can be enabled per-processor to capture intermediate output

---

## Common Development Tasks

### Build on macOS for local testing:
```bash
export PYTHONMAJOR=3.11
export pythonLocation=/Library/Frameworks/Python.framework/Versions/3.11
export ARCHS=arm64  # or x86_64
./build_macos.sh
cd tests && python -m pytest -s
```

### Build wheel and install locally:
```bash
python -m pip install build wheel
python -m build --wheel
pip install dist/dawdreamer-*.whl
```

### Debug a test:
```bash
cd tests
python -m pytest -s test_faust_processor.py::test_faust_passthrough -vv
# Add print statements to test code or C++ source, rebuild
```

### Add a new Faust processor example:
```python
# In tests/test_myfeature.py
engine = daw.RenderEngine(44100, 512)
faust_proc = engine.make_faust_processor("faust")
faust_proc.set_dsp_string("""
    declare name "Example";
    freq = hslider("freq", 440, 0, 20000, 0);
    process = freq : os.osc : *.0.1 <: _, _;
""")
engine.load_graph([(faust_proc, [])])
engine.render(2.)
audio = engine.get_audio()
```

### Profile C++ code:
- Use Xcode profiler or Linux perf
- Watch for buffer allocations in hot paths
- Monitor CPU usage during `render()` call

---

## Key Dependencies & Versions

| Dependency      | Purpose                           | Version/Notes                 |
|-----------------|-----------------------------------|-------------------------------|
| JUCE            | Audio framework                   | See JUCE version management below |
| pybind11        | C++/Python bindings               | Submodule, header-only        |
| Faust           | DSP language compiler             | Submodule, embedded compiler  |
| libfaust        | Pre-compiled Faust runtime        | Downloaded via script          |
| libsamplerate   | Sample rate conversion            | Built from source (CMake)     |
| RubberBand      | Time/pitch modification           | Submodule, optional build flag|
| Python headers  | C API and development files       | 3.10-3.12 required            |

### JUCE Version Management

**IMPORTANT**: DawDreamer uses a two-JUCE-version system:

1. **thirdparty/JUCE submodule**: Pinned at JUCE 5.3.2 (commit cf4f12a452)
   - **DO NOT update this submodule**
   - It remains at JUCE 5 for stability and compatibility
   - Used as a fallback reference

2. **JuceLibraryCode/modules**: Active JUCE version used for builds
   - Currently using **JUCE 8.0.10+**
   - Updated manually by contributors using Projucer
   - This is the version that actually compiles into DawDreamer

**How to update JUCE for development:**

```bash
# 1. Install JUCE 8.x separately (not in the repo)
# Download from https://juce.com/get-juce/ or:
git clone --branch 8.0.10 https://github.com/juce-framework/JUCE.git ~/JUCE

# 2. Open DawDreamer.jucer with the NEW version of Projucer
# The Projucer app is inside the JUCE folder you just downloaded:
~/JUCE/extras/Projucer/Builds/MacOSX/build/Debug/Projucer.app

# 3. In Projucer:
#    - File > Open > DawDreamer.jucer
#    - Projucer will detect the JUCE version and update JuceLibraryCode/modules
#    - File > Save Project (or Ctrl+S)
#    - This regenerates JuceLibraryCode/ with the new JUCE version

# 4. Build as normal
xcodebuild -project Builds/MacOSX/DawDreamer.xcodeproj ...
```

**Key Points:**
- DawDreamer.jucer is configured to use `path="JuceLibraryCode/modules"` for all modules
- JuceLibraryCode/ is the "working copy" of JUCE that gets compiled
- thirdparty/JUCE stays pinned at JUCE 5 and should not be modified
- When you open the .jucer file with a newer Projucer, it automatically updates JuceLibraryCode/

---

## Continuous Integration

**GitHub Actions** (`.github/workflows/all.yml`):
- Builds on macOS, Windows, Linux
- Runs test suite
- Builds wheels for all platforms
- Publishes to PyPI on release

**Key CI patterns:**
- Each platform builds in isolation
- Wheel tested after installation
- Tests run post-installation (not from source)

---

## Documentation & Resources

- **API Docs**: https://dirt.design/DawDreamer/
- **GitHub Wiki**: Processor tutorials, examples
- **Examples**: `/examples` directory with real-world usage
- **JUCE Documentation**: https://juce.com/learn/documentation
- **Faust Documentation**: https://faustdoc.grame.fr/
- **Original Paper**: https://arxiv.org/abs/2111.09931

---

## Architecture Decision Notes

**Why C++ + Python?**
- Audio processing performance-critical → C++
- User-facing API simple and flexible → Python
- pybind11 provides clean, minimal overhead binding

**Why JUCE?**
- Cross-platform audio I/O, MIDI, VST hosting
- Mature, well-maintained, large community
- Good plugin support

**Why Faust?**
- Live DSP compilation at runtime
- Vast standard library (filters, reverbs, etc.)
- Transpiles to multiple backends (C++, JAX, etc.)

**Why DAG-based architecture?**
- Enables flexible routing without limitations of linear chains
- Dependency resolution prevents cycle detection issues
- Natural mapping to music production workflows

**Block-based processing:**
- Audio processed in small chunks (512 samples typical)
- Allows for lower latency parameter automation
- Matches real-time audio plugin standards

---

## Troubleshooting Tips

### Build Issues

**"Configuration Release is not in the project"**
- **Cause**: Trying to use `-configuration Release` instead of `-configuration Release-arm64` or `-configuration Release-x86_64`
- **Solution**: Use the full configuration name: `xcodebuild -configuration Release-arm64`

**"library 'samplerate' not found" or "library 'faustwithllvm' not found"**
- **Cause**: Dependencies not built yet
- **Solution**:
  1. Build libsamplerate: `cd thirdparty/libsamplerate && mkdir -p build_release && cd build_release && cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 && make`
  2. Download libfaust: `cd thirdparty/libfaust && python download_libfaust.py`

**"dawdreamer.so not found" during setup.py**
- **Cause**: Xcode outputs `dawdreamer.so.dylib` but setup.py expects `dawdreamer.so`
- **Solution**: Copy the file: `cp Builds/MacOSX/build/Release-$ARCHS/dawdreamer.so.dylib Builds/MacOSX/build/Release-$ARCHS/dawdreamer.so`

**"ERROR: file:///...thirdparty/libfaust does not appear to be a Python project"**
- **Cause**: Using `pip install -e .` instead of `setup.py develop`
- **Solution**: Use `ARCHS=arm64 python3.11 setup.py develop` instead

**Architecture mismatch errors**
- **Cause**: ARCHS environment variable doesn't match your system
- **Solution**:
  - Apple Silicon: `export ARCHS=arm64`
  - Intel Mac: `export ARCHS=x86_64`
  - Check your architecture: `uname -m`

**Xcode version compatibility**
- **Note**: Originally documentation suggested Xcode 14 only, but Xcode 16.0.1 has been tested successfully
- **Solution**: Most recent Xcode versions should work. If issues persist, try Xcode 14-16 range.

**Submodules not initialized**
- **Cause**: Missing thirdparty dependencies
- **Solution**: `git submodule update --init --recursive`

**CMake not found**
- **Solution**: Install via Homebrew: `brew install cmake`

**Faust compiler not found**
- **Solution**: Install via Homebrew: `brew install faust`

### Runtime Issues

**ImportError: cannot import dawdreamer**
- Check Python version matches build: `python3.11 --version`
- Verify installation: `python3.11 -c "import dawdreamer"`
- Check that setup.py was run with correct ARCHS: `ARCHS=arm64 python3.11 setup.py develop`

**Faust compilation errors**
- Verify Faust libraries are in package: `ls dawdreamer/faustlibraries/`
- Check Faust code syntax
- Try simpler DSP code first: `process = _;` (passthrough)

**Test Failures:**
- Ensure test assets exist in `tests/assets/`
- Check Python version: 3.10-3.12
- For VST tests: plugins must exist at specified paths
- Audio comparison tolerance might need adjustment for platform

**Plugin Loading:**
- Paths must be absolute or relative to cwd
- Verify plugin architecture matches (x86_64/arm64)
- VST3 takes priority; fallback to VST2 if available
- macOS: Plugin may need to be code-signed or security settings adjusted

### Development Tips

**Faster incremental builds:**
- After first full build, only rebuild C++ when needed
- Use `xcodebuild ... build` (without `clean`) for incremental builds
- Python changes don't require C++ rebuild

**Debugging C++ code:**
- Build Debug configuration: `-configuration Debug-arm64`
- Use Xcode debugger or lldb
- Add print statements via `std::cout` or `DBG()` macro (JUCE)

**Testing specific functionality:**
- Use pytest with specific test: `python -m pytest -s test_faust_processor.py::test_name`
- Add `-v` for verbose output
- Use `--pdb` to drop into debugger on failure

---

## Code Quality & Pre-commit Hooks

DawDreamer uses **pre-commit** to automatically enforce code quality standards before each commit.

### Setup

```bash
# Install pre-commit (if not already installed)
pip install pre-commit

# Install the git hooks
pre-commit install

# (Optional) Run on all files
pre-commit run --all-files
```

### What Pre-commit Checks

**Automatic Fixes:**
- **Python formatting**: Ruff formatter (replaces black, flake8, isort)
- **C++ formatting**: clang-format (LLVM-based style with Allman braces)
- **Trailing whitespace**: Automatically removed
- **End-of-file**: Ensures files end with newline
- **Line endings**: Normalized to LF (Unix-style)

**Validation:**
- **YAML/TOML syntax**: Validates configuration files
- **Shell scripts**: Shellcheck for build_*.sh scripts
- **Merge conflicts**: Detects unresolved conflict markers
- **Large files**: Prevents accidental commits of large files (>1MB)
- **Spell checking**: Codespell catches common typos

**Exclusions:**
- All checks skip `thirdparty/`, `JuceLibraryCode/`, `Builds/`, and binary files
- Won't modify JUCE project files (`.jucer`)

### Configuration Files

- **`.pre-commit-config.yaml`**: Hook configurations and versions
- **`.clang-format`**: C++ formatting rules (4-space indent, 100-char line length)
- **`pyproject.toml`**: Ruff configuration (Python 3.11+, line length 100)
  - **Star imports allowed**: F403/F405 disabled for tests, Faust API, and examples
  - The Faust Box/Signal APIs intentionally use `from dawdreamer.faust.box import *` for DSL-style usage
  - Test files use `from dawdreamer_utils import *` for convenience

### Bypassing Hooks (Not Recommended)

```bash
# Skip pre-commit hooks for a single commit (use sparingly!)
git commit --no-verify -m "message"
```

### Manual Formatting

```bash
# Format Python code manually
ruff format .
ruff check --fix .

# Format C++ code manually
clang-format -i Source/*.cpp Source/*.h
```

### Pre-commit in CI/CD

Currently, pre-commit hooks run **locally only**. Consider adding to GitHub Actions workflow:

```yaml
- name: Run pre-commit
  run: |
    pip install pre-commit
    pre-commit run --all-files
```

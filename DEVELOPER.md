# DawDreamer Developer Guide

> **For quick build/install instructions optimized for LLMs and automation, see [CLAUDE.md](CLAUDE.md)**

## Table of Contents
- [Architecture Overview](#architecture-overview)
- [Getting Started](#getting-started)
- [Platform-Specific Builds](#platform-specific-builds)
- [Development Workflow](#development-workflow)
- [Testing](#testing)
- [Troubleshooting](#troubleshooting)

---

## Architecture Overview

DawDreamer is a **Digital Audio Workstation (DAW) framework for Python**. It enables programmatic audio processing with support for VST plugins, Faust DSP code, and complex audio routing graphs. Licensed under GPLv3.

### Key Capabilities
- Composable audio processor graphs (DAG-based)
- VST 2/3 instrument and effect support with UI and state management
- Faust effects and polyphonic instruments (Box and Signal APIs)
- Time-stretching, looping, pitch-warping via RubberBand
- Parameter automation at audio-rate and PPQN (pulses-per-quarter-note) rates
- MIDI playback and file export
- Multi-processor parallel rendering
- Transpilation: Faust to JAX/Flax, C++, Rust, WebAssembly, etc.

**Platforms**: macOS 12.0+, Windows (x86_64), Linux (x86_64)
**Python Support**: 3.11-3.14 (3.8-3.12 in wheel metadata)

### High-Level Structure

```
DawDreamer/
├── Source/                          # C++ core implementation
│   ├── source.cpp                   # nanobind Python bindings (entry point)
│   ├── RenderEngine.{h,cpp}         # Main audio engine & DAG execution
│   ├── ProcessorBase.{h,cpp}        # Abstract processor base class
│   ├── *Processor.h                 # Processor implementations
│   ├── FaustProcessor.{h,cpp}       # Faust DSP integration
│   ├── PluginProcessor.{h,cpp}      # VST plugin host
│   └── Sampler/                     # Multi-sampler with UI
├── Builds/                          # Platform-specific build configs
│   ├── MacOSX/DawDreamer.xcodeproj/
│   ├── VisualStudio2022/
│   └── LinuxMakefile/
├── DawDreamer.jucer                 # JUCE Projucer configuration
├── dawdreamer/                      # Python package
│   ├── dawdreamer.so (or .pyd/.dll) # Compiled C++ extension
│   ├── architecture/                # Faust architecture files
│   └── faustlibraries/              # Faust standard library
├── tests/                           # pytest test suite
└── thirdparty/                      # External dependencies (git submodules)
    ├── JUCE/                        # Audio framework
    ├── nanobind/                    # C++ ↔ Python bindings
    ├── faust/                       # Faust DSP compiler
    ├── libsamplerate/               # Sample rate conversion
    └── rubberband/                  # Time-stretch/pitch-shift
```

### Core Components

#### 1. RenderEngine (Source/RenderEngine.{h,cpp})
- Central orchestrator managing the audio processing pipeline
- Implements `AudioPlayHead` interface for tempo/timing
- Manages DAG (Directed Acyclic Graph) of processors
- Handles audio buffer allocation and rendering lifecycle

**Key Methods:**
- `loadGraph(DAG)` - validates and loads processor graph
- `render(duration, isBeats)` - processes audio for specified duration
- `getAudioFrames()` - returns rendered audio as numpy array
- `setBPM()`, `setBPMwithPPQN()` - tempo management

#### 2. ProcessorBase (Source/ProcessorBase.{h,cpp})
- Abstract base class for all audio processors
- Manages input/output channels, parameter automation, recording
- Provides automation record/playback at both audio-rate and PPQN-rate

#### 3. Processor Implementations
- **FaustProcessor** - Compiles/executes Faust DSP code in real-time
- **PluginProcessor** - Hosts VST 2/3 plugins
- **PlaybackProcessor** - Plays back audio buffers
- **PlaybackWarpProcessor** - Time-stretches/pitch-shifts audio (RubberBand)
- **FilterProcessor** - Built-in IIR filter
- **OscillatorProcessor** - Simple sine wave generator
- **CompressorProcessor** - Dynamics processor
- **SamplerProcessor** - Sampled instrument with MPE support

#### 4. Python Bindings (Source/source.cpp)
- nanobind-based C++ → Python interface
- Exposes all processors, RenderEngine, parameters
- Converts numpy arrays ↔ C++ audio buffers

---

## Getting Started

### Prerequisites

Clone the repo and initialize submodules:
```bash
git clone https://github.com/DBraun/DawDreamer.git
cd DawDreamer
git submodule update --init --recursive
```

Download libfaust:
```bash
cd thirdparty/libfaust
python download_libfaust.py
cd ../..
```

### Build Dependencies

**All Platforms:**
- CMake
- Python 3.11-3.14 with development headers
- Git

**Linux:**
- build-essential, clang, pkg-config
- libboost-all-dev, libfreetype6-dev
- X11 libraries (libx11-dev, libxinerama-dev, libxrandr-dev, libxcursor-dev, libxcomposite-dev)
- ALSA/audio libraries (libasound2-dev, freeglut3-dev, mesa-common-dev)
- libncurses-dev (provides libtinfo for Faust LLVM linking)

**macOS:**
- Xcode with command line tools (14-16)
- Homebrew: `brew install cmake faust`
- Note: ncurses/tinfo NOT needed (part of system)

**Windows:**
- Visual Studio 2022 with C++ desktop development workload

---

## Platform-Specific Builds

### Linux

#### Install Dependencies (Ubuntu/Debian):
```bash
apt-get install -yq --no-install-recommends \
  ca-certificates build-essential clang pkg-config \
  libboost-all-dev libboost-python-dev libfreetype6-dev \
  libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev \
  mesa-common-dev libasound2-dev freeglut3-dev \
  libxcomposite-dev libcurl4-gnutls-dev libncurses-dev \
  git cmake python3 python3-dev
```

#### Set Environment Variables:
```bash
export PYTHONLIBPATH=/usr/lib/python3.12
export PYTHONINCLUDEPATH=/usr/include/python3.12
```

#### Build libsamplerate:
```bash
cd thirdparty/libsamplerate
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release
cmake --build build_release --config Release
cd ../..
```

#### Build, Copy, and Install:

`setup.py` handles the full flow: building C++ via `make`, copying the `.so`, and installing the Python package. If the C++ sources haven't changed since the last build, it skips recompilation.

```bash
pip install -e .
```

To build manually (e.g., for debugging):
```bash
cd Builds/LinuxMakefile
make CONFIG=Release CXXFLAGS="-I$PYTHONINCLUDEPATH" LDFLAGS="-L$PYTHONLIBPATH"
cd ../..
```

Build output: `Builds/LinuxMakefile/build/libdawdreamer.so`
`setup.py` copies this to `dawdreamer/dawdreamer.so` automatically.

**WSL2 Note**: The `make` build system may not detect source changes across the Windows/Linux filesystem boundary due to timestamp caching. If changes aren't picked up, delete the build directory first:
```bash
rm -rf Builds/LinuxMakefile/build
```

### macOS

#### Set Environment:
```bash
export PYTHONMAJOR=3.11
export pythonLocation=/Library/Frameworks/Python.framework/Versions/3.11
export ARCHS=arm64  # or x86_64 for Intel
```

#### Build libsamplerate:
```bash
cd thirdparty/libsamplerate
mkdir -p build_release && cd build_release
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=$ARCHS
make -j$(sysctl -n hw.ncpu)
cd ../../..
```

#### Build DawDreamer:
```bash
xcodebuild -project Builds/MacOSX/DawDreamer.xcodeproj \
  -target "DawDreamer - Dynamic Library" \
  -configuration Release-$ARCHS \
  build

# Copy output (Xcode builds .dylib but setup.py expects .so)
cp Builds/MacOSX/build/Release-$ARCHS/dawdreamer.so.dylib \
   Builds/MacOSX/build/Release-$ARCHS/dawdreamer.so
```

**Or use the build script:**
```bash
./build_macos.sh
```

#### Install:
```bash
ARCHS=$ARCHS python3.11 setup.py develop
```

### Windows

#### Set Environment (x64 Native Tools Command Prompt):
```cmd
set PYTHONMAJOR=3.11
set pythonLocation=C:\Python311
```

#### Build libsamplerate:
```cmd
cd thirdparty\libsamplerate
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release
cmake --build build_release --config Release
cd ..\..
```

#### Build DawDreamer:
```cmd
msbuild Builds/VisualStudio2022/DawDreamer.sln /property:Configuration=Release
```

Output: `Builds/VisualStudio2022/x64/Release/Dynamic Library/dawdreamer.dll`

**Note:** Post-build command copies `dawdreamer.dll` to `C:/Python311/dawdreamer.pyd`. Remove this file after building a wheel to avoid conflicts.

### Docker

```bash
docker build -t dawdreamer .
docker run -it dawdreamer /bin/bash
```

See [Issue #82](https://github.com/DBraun/DawDreamer/issues/82#issuecomment-1097937567) for details.

---

## Development Workflow

### Modifying C++ Source

1. Edit source files in `Source/`
2. If adding/removing files, open `DawDreamer.jucer` with [JUCE Projucer](https://juce.com/get-juce)
   - Projucer regenerates platform-specific build files
   - **DO NOT** edit .xcodeproj/.sln files directly
3. Rebuild and reinstall: `pip install -e .`
   - Detects source changes, rebuilds C++, copies `.so`, installs package
   - On WSL2, if changes aren't detected: `rm -rf Builds/LinuxMakefile/build` first

**Adding a New Processor:**
1. Create `Source/MyProcessor.h` (header-only or with .cpp)
2. Implement as `public ProcessorBase` subclass
3. Add nanobind binding in `Source/source.cpp`
4. Add factory method to RenderEngine
5. Update DawDreamer.jucer to include new files
6. Rebuild

### Modifying Python Bindings

Edit `Source/source.cpp` (nanobind glue code), then rebuild.

### Understanding the Rendering Pipeline

**Typical usage:**
```python
import dawdreamer as daw

# Create engine
engine = daw.RenderEngine(44100, 512)

# Create processors
synth = engine.make_faust_processor("synth")
synth.set_dsp_string("process = os.osc(440) : *(0.1);")

effect = engine.make_plugin_processor("reverb", "path/to/plugin.dll")

# Build graph
graph = [
    (synth, []),           # synth takes no input
    (effect, ["synth"])    # effect processes synth output
]
engine.load_graph(graph)

# Render
engine.set_bpm(120)
engine.render(4., beats=True)  # 4 beats

# Get audio
audio = engine.get_audio()  # shape (channels, samples)
```

**Internal flow:**
1. `load_graph()` validates DAG, prepares processor connections
2. `render()` allocates buffers, calls `process()` on each processor in dependency order
3. Each processor reads from input buffers, writes to output buffers
4. Automation applied during `process()` based on tempo/time
5. Audio returned as numpy array

### Important Patterns

**Parameter Automation:**
- Audio-rate: One value per sample
- PPQN-rate: Values at specified pulses-per-quarter-note

**Processor Naming:**
- Each processor must have a unique name
- Used for graph connectivity: `(effect_proc, ["input_proc_name"])`

**Multi-Channel Audio:**
- All processors support arbitrary channel counts
- Input channels must match previous processor output

**Sample Rate:**
- Specified at RenderEngine creation
- All processors use same rate (no per-processor conversion)

---

## Testing

### Running Tests

```bash
cd tests
python -m pytest -s                           # All tests
python -m pytest -s test_faust_processor.py   # Specific file
python -m pytest -s test_faust_processor.py::test_name  # Specific test
```

### Test Structure

- Each test imports `dawdreamer_utils` for common utilities
- Tests use fixture data in `assets/` (audio files)
- Faust DSP snippets in `faust_dsp/`
- Output files written to `output/` directory
- Parametrized tests common (testing multiple buffer sizes, durations)

### Key Test Utilities (dawdreamer_utils.py)

- `load_audio_file()` - loads audio via librosa/soundfile
- `make_sine()` - generates test sine wave
- `render()` - helper to render engine and optionally save to file
- Constants: `SAMPLE_RATE=44100`, asset/output paths

---

## Building a Wheel

### macOS

Define architecture:
```bash
export ARCHS=arm64  # or x86_64
```

### All Platforms

Install build tools:
```bash
python -m pip install build wheel
```

Build wheel:
```bash
python -m build --wheel
```

Install wheel:
```bash
python -m pip install dist/dawdreamer-*.whl
```

**Note:** On macOS, `ARCHS` environment variable must be set. On Windows/Linux, it's auto-detected.

---

## Troubleshooting

### Build Issues

**Missing libfaust**
- Run `python download_libfaust.py` in `thirdparty/libfaust/`

**Architecture mismatch (macOS)**
- Ensure `ARCHS` matches your system: `arm64` or `x86_64`
- Check with: `uname -m`

**Missing libtinfo.so (Linux)**
- Install: `sudo apt-get install libncurses-dev`
- Verify: `ldd dawdreamer/dawdreamer.so | grep tinfo`

**setup.py develop is slow (WSL2)**
- Normal on WSL2 due to cross-filesystem I/O (1-2 minutes)
- Processing ~200+ Faust library files

**C++ changes not detected (WSL2)**
- WSL2 cross-filesystem timestamps may be stale, causing `make` to skip rebuilds
- Fix: `rm -rf Builds/LinuxMakefile/build` then run `pip install -e .` again

### Runtime Issues

**ImportError: cannot import dawdreamer**
- Check Python version: `python3 --version` (should be 3.11-3.14)
- Verify installation: `python3 -c "import dawdreamer"`
- On macOS, check ARCHS was set during build

**Faust compilation errors**
- Verify libraries: `ls dawdreamer/faustlibraries/`
- Test simple code first: `process = _;` (passthrough)

**Plugin loading issues**
- Use absolute paths or paths relative to cwd
- Verify plugin architecture matches (x86_64/arm64)
- On macOS, plugin may need code-signing

### Development Tips

**Faster incremental builds:**
- Use `xcodebuild ... build` (without `clean`)
- Python changes don't require C++ rebuild

**Debugging C++:**
- Build Debug configuration: `-configuration Debug-arm64`
- Use Xcode debugger or lldb
- Add print statements via `std::cout` or `DBG()` macro (JUCE)

**Testing specific functionality:**
- Use pytest with `-v` for verbose output
- Use `--pdb` to drop into debugger on failure

---

## Code Quality

### Pre-commit Hooks

Install:
```bash
pip install pre-commit
pre-commit install
```

**What it checks:**
- Python: Ruff formatter (replaces black, flake8, isort)
- C++: clang-format (LLVM-style, Allman braces)
- Trailing whitespace, end-of-file, line endings
- YAML/TOML syntax, shell scripts (shellcheck)
- Spell checking (codespell)

**Configuration:**
- `.pre-commit-config.yaml` - Hook configurations
- `.clang-format` - C++ rules (4-space indent, 100-char line)
- `pyproject.toml` - Ruff config (Python 3.11+, line length 100)

---

## Architecture Decisions

**Why C++ + Python?**
- Performance-critical audio processing → C++
- Simple, flexible user API → Python
- nanobind provides clean, minimal overhead bindings

**Why JUCE?**
- Cross-platform audio I/O, MIDI, VST hosting
- Mature, well-maintained, large community

**Why Faust?**
- Live DSP compilation at runtime
- Vast standard library
- Transpiles to multiple backends

**Why DAG-based?**
- Flexible routing without linear chain limitations
- Natural mapping to music production workflows

**Block-based processing:**
- Audio processed in small chunks (512 samples typical)
- Lower latency parameter automation
- Matches real-time audio plugin standards

---

## Key Dependencies

| Dependency      | Purpose                     | Version/Notes             |
|-----------------|-----------------------------|---------------------------|
| JUCE            | Audio framework             | Latest main branch        |
| nanobind        | C++/Python bindings         | Submodule, header-only    |
| Faust           | DSP language compiler       | Submodule, embedded       |
| libfaust        | Pre-compiled Faust runtime  | Downloaded via script     |
| libsamplerate   | Sample rate conversion      | Built from source (CMake) |
| RubberBand      | Time/pitch modification     | Submodule                 |
| Python headers  | C API and dev files         | 3.11-3.14 required        |

---

## Resources

- **API Docs**: https://dirt.design/DawDreamer/
- **GitHub Wiki**: Processor tutorials, examples
- **Examples**: `/examples` directory with real-world usage
- **JUCE Documentation**: https://juce.com/learn/documentation
- **Faust Documentation**: https://faustdoc.grame.fr/
- **Original Paper**: https://arxiv.org/abs/2111.09931
- **Local Docs**: The `docs/` directory contains comprehensive documentation

---

## Continuous Integration

**GitHub Actions** (`.github/workflows/all.yml`):
- Builds on macOS, Windows, Linux
- Runs test suite
- Builds wheels for all platforms
- Publishes to PyPI on release

---

For quick build instructions optimized for LLMs and automation, see **[CLAUDE.md](CLAUDE.md)**.

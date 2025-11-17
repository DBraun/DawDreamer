# DawDreamer - LLM Quick Build & Install Guide

> **For architecture, development workflow, and detailed documentation, see [DEVELOPER.md](DEVELOPER.md)**

DawDreamer is a Digital Audio Workstation (DAW) framework for Python enabling programmatic audio processing with VST plugins, Faust DSP, and complex audio routing graphs.

**Python**: 3.11-3.14 | **License**: GPLv3

---

## Quick Install (Library Already Built)

If `dawdreamer/dawdreamer.so` exists:

```bash
# Minimal output (recommended for LLMs)
python3 setup.py develop --quiet && echo "✓ Installed" || echo "✗ Failed"

# Verify
python3 -c "import dawdreamer; dawdreamer.RenderEngine(44100, 512)" && echo "✓ Working"
```

**WSL2 Note**: Takes 1-2 minutes (normal - processing Faust libraries).

---

## Quick Build (Library Doesn't Exist)

### Linux

```bash
# Install dependencies
sudo apt-get install -yq build-essential clang cmake git python3-dev \
  libboost-all-dev libfreetype6-dev libncurses-dev \
  libx11-dev libxrandr-dev libasound2-dev libxcomposite-dev

# Setup and build
git submodule update --init --recursive
cd thirdparty/libfaust && python3 download_libfaust.py && cd ../..

export PYTHONLIBPATH=/usr/lib/python3.12
export PYTHONINCLUDEPATH=/usr/include/python3.12

cd thirdparty/libsamplerate && \
  cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release && \
  cmake --build build_release && cd ../..

cd Builds/LinuxMakefile && \
  make CONFIG=Release CXXFLAGS="-I$PYTHONINCLUDEPATH" LDFLAGS="-L$PYTHONLIBPATH" && \
  cd ../..

python3 setup.py develop --quiet && echo "✓ Done"
```

### macOS

```bash
export PYTHONMAJOR=3.11 ARCHS=arm64
export pythonLocation=/Library/Frameworks/Python.framework/Versions/3.11
./build_macos.sh && ARCHS=$ARCHS python3.11 setup.py develop
```

### Windows

```cmd
set PYTHONMAJOR=3.11 && set pythonLocation=C:\Python311
msbuild Builds/VisualStudio2022/DawDreamer.sln /property:Configuration=Release
python setup.py develop
```

---

## Common Issues

| Issue | Solution |
|-------|----------|
| Slow install (WSL2) | Normal - wait 1-2 min |
| Missing libtinfo (Linux) | `sudo apt-get install libncurses-dev` |
| Import fails | Check Python 3.11-3.14: `python3 --version` |
| Arch mismatch (macOS) | Set `ARCHS=arm64` or `x86_64` |

---

**See [DEVELOPER.md](DEVELOPER.md) for architecture, development workflow, testing, and detailed troubleshooting.**

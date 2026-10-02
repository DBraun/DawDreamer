# DawDreamer - LLM Quick Build & Install Guide

> **For architecture, development workflow, and detailed documentation, see [DEVELOPER.md](DEVELOPER.md)**

DawDreamer is a Digital Audio Workstation (DAW) framework for Python enabling programmatic audio processing with VST plugins, Faust DSP, and complex audio routing graphs.

**Python**: 3.11-3.14 | **License**: GPLv3

---

## Quick Install

`setup.py` handles the full flow: building C++ (if needed), copying the `.so`, and installing. It detects source changes and skips recompilation when up-to-date.

```bash
pip install -e .

# Verify
python3 -c "import dawdreamer; dawdreamer.RenderEngine(44100, 512)"
```

**WSL2 Note**: First install takes several minutes (C++ build + processing Faust libraries). Subsequent installs skip the C++ build if sources haven't changed.

---

## First-Time Setup (Prerequisites)

### Linux

```bash
# Install dependencies
sudo apt-get install -yq build-essential clang cmake git python3-dev \
  libboost-all-dev libfreetype6-dev libncurses-dev \
  libx11-dev libxrandr-dev libasound2-dev libxcomposite-dev

# Submodules and Faust
git submodule update --init --recursive
cd thirdparty/libfaust && python3 download_libfaust.py && cd ../..

# Build and install (builds C++, copies .so, installs package)
pip install -e .
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
| Slow first install (WSL2) | Normal - C++ build + Faust libs |
| C++ changes not detected (WSL2) | `rm -rf Builds/LinuxMakefile/build` then reinstall |
| Missing libtinfo (Linux) | `sudo apt-get install libncurses-dev` |
| Import fails | Check Python 3.11-3.14: `python3 --version` |
| Arch mismatch (macOS) | Set `ARCHS=arm64` or `x86_64` |

---

**See [DEVELOPER.md](DEVELOPER.md) for architecture, development workflow, testing, and detailed troubleshooting.**

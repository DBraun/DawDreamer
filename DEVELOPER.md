## Getting started

Clone this repo and update submodules: `git submodule update --init --recursive`

Then go to `thirdparty/libfaust` and run `python download_libfaust.py`.

## Linux

If you don't already have python3 installed, do

`apt-get install -yq --no-install-recommends python3 python3.10-dev`
Then set these environment variables, confirming that they make sense for your system.
```bash
export PYTHONLIBPATH=/usr/lib/python3.10
export PYTHONINCLUDEPATH=/usr/include/python3.10
```

Then look at the content of the script `build_linux.sh`. If you're on Ubuntu instead of CentOS, then instead of the `yum install` command, you'd substitute the following:
```
apt-get install -yq --no-install-recommends \
ca-certificates \
build-essential \
clang \
pkg-config \
libboost-all-dev \
libboost-python-dev \
libfreetype6-dev \
libx11-dev \
libxinerama-dev \
libxrandr-dev \
libxcursor-dev \
mesa-common-dev \
libasound2-dev \
freeglut3-dev \
libxcomposite-dev \
libcurl4-gnutls-dev \
git \
cmake \
python3 \
python3.10-dev
```
After confirming `build_linux.sh` makes sense for your operating system, run `sh build_linux.sh`. When it finishes, you can proceed to the section "Building a Wheel".

## Docker

To build an image with the `Dockerfile`, run:

```bash
docker build -t dawdreamer .
```
Then run the dockerfile:
```bash
docker run -it dawdreamer /bin/bash
```

See https://github.com/DBraun/DawDreamer/issues/82#issuecomment-1097937567

## Windows and macOS

### Pre-requisite software:

* [CMake](https://cmake.org/download/)
* Python
  * On Windows install [Python 3.11.x Windows x86-64](https://www.python.org/downloads/release/python-3113/) to `C:/Python311`
  * On macOS install with the standard settings (including the Pylauncher) [Python 3.11.3 (macOS) Universal Installer](https://www.python.org/downloads/release/python-3113/)

### Environment Variables:
These are case-sensitive and feel free to replace `3.11` for your needs.
* `PYTHONMAJOR`: 3.11
* `pythonLocation`:
  * `C:\Python311` on Windows
  * `/Library/Frameworks/Python.framework/Versions/3.11` on macOS
  * `/usr/local/opt/python@3.11/Frameworks/Python.framework/Versions/3.11` on macOS, if you installed Python with homebrew

### Install Faust Libraries

The [Faust Libraries](https://github.com/grame-cncm/faustlibraries) should already be in the `dawdreamer/faustlibraries` directory of this repo. If you *aren't* going to create a wheel, you need to copy the Faust Libraries to a specific location:

* macOS / Linux: Download [Faust Libraries](https://github.com/grame-cncm/faustlibraries) to `/usr/local/share/faust` or `/usr/share/faust`. Example file path: `/usr/local/share/faust/stdfaust.lib`
* Windows:  Download [Faust Libraries](https://github.com/grame-cncm/faustlibraries) to `C:/share/faust`. The reason is that we're using `C:/Python311/python.exe`, so the sibling directory would be `C:/share/faust`. Example file path: `C:/Python39/share/faust/stdfaust.lib`

### Build libsamplerate

```bash
cd thirdparty/libsamplerate
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release
cmake --build build_release --config Release
```

### Building DawDreamer

You can find a working Linux Makefile, Visual Studio Solution, and Xcode Project in the `Builds/` folder. If you want to make changes, the best way is to get [JUCE's Projucer](https://juce.com/get-juce) and open `DawDreamer.jucer`.

#### Windows

In an x64 Native Tools Command Prompt for Visual Studio 2022, run

```bash
msbuild Builds/VisualStudio2022/DawDreamer.sln /property:Configuration=Release
```

This will automatically use a post-build command which moves the recently built `dawdreamer.dll` to `C:/Python311/dawdreamer.pyd`. If you will later build a wheel, you will want to **remove** this `dawdreamer.pyd` after building the wheel. Then when you install the wheel, the locally installed wheel from site-packages will be used instead of  `C:/Python311/dawdreamer.pyd`.

#### MacOS

On macOS, Xcode 15 will not work. You should have Xcode 14 installed.

On macOS, you can run `build_macos.sh`. Note that it will move the `dawdreamer.so` to the `tests` folder so that you can run the tests there without building and installing a wheel.

## Building a Wheel

> On macOS you should define your architecture type such as
`export ARCHS=x86_64` or `export ARCHS=arm64`

We will build a wheel inside the root of this DawDreamer repo. Start by installing the `build` and `wheel` modules:

`python -m pip install build wheel`

Then build the wheel:

`python -m build --wheel`

Then install the wheel:

`python -m pip install dist/dawdreamer.whl` with the correct file path to the wheel.

## Tests

Go to the [`tests`](https://github.com/DBraun/DawDreamer/tree/main/tests) directory and follow the instructions.

Installation
============

Quick Install
-------------

For most users, installation is as simple as:

.. code-block:: bash

   pip install dawdreamer

System Requirements
-------------------

**macOS:**

* 64-bit Python 3.11-3.14
* Apple Silicon (arm64)
* macOS 11.0 or higher

**Windows:**

* x86_64 CPU
* 64-bit Python 3.11-3.14

**Linux:**

* x86_64 CPU
* 64-bit Python 3.11-3.14

Building from Source
--------------------

If you need to build DawDreamer from source (for development or custom builds), follow these instructions.

Prerequisites
~~~~~~~~~~~~~

1. Clone the repository and initialize submodules:

   .. code-block:: bash

      git clone https://github.com/DBraun/DawDreamer.git
      cd DawDreamer
      git submodule update --init --recursive

2. Download the Faust libraries:

   .. code-block:: bash

      cd thirdparty/libfaust
      python download_libfaust.py
      cd ../..

Platform-Specific Build Instructions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Linux
^^^^^

1. Install dependencies (Ubuntu/Debian):

   .. code-block:: bash

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
        python3.11-dev

2. Set environment variables:

   .. code-block:: bash

      export PYTHONLIBPATH=/usr/lib/python3.11
      export PYTHONINCLUDEPATH=/usr/include/python3.11

3. Build libsamplerate:

   .. code-block:: bash

      cd thirdparty/libsamplerate
      cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release
      cmake --build build_release --config Release
      cd ../..

4. Run the build script:

   .. code-block:: bash

      sh build_linux.sh

Docker
^^^^^^

To build and run DawDreamer in a Docker container:

.. code-block:: bash

   # Build the image
   docker build -t dawdreamer .

   # Run the container
   docker run -it dawdreamer /bin/bash

See `Issue #82 <https://github.com/DBraun/DawDreamer/issues/82#issuecomment-1097937567>`_ for more details.

Windows
^^^^^^^

1. Install prerequisites:

   * `CMake <https://cmake.org/download/>`_
   * `Python 3.11.x Windows x86-64 <https://www.python.org/downloads/>`_ to ``C:/Python311``

2. Set environment variables:

   .. code-block:: batch

      set PYTHONMAJOR=3.11
      set pythonLocation=C:\Python311

3. Build libsamplerate:

   .. code-block:: bash

      cd thirdparty\libsamplerate
      cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release
      cmake --build build_release --config Release
      cd ..\..

4. Build DawDreamer using Visual Studio 2022 (in x64 Native Tools Command Prompt):

   .. code-block:: bash

      msbuild Builds/VisualStudio2022/DawDreamer.sln /property:Configuration=Release

   .. note::
      The post-build command will automatically copy ``dawdreamer.dll`` to ``C:/Python311/dawdreamer.pyd``.
      If you later build a wheel, you should **remove** this file to avoid conflicts.

macOS
^^^^^

1. Install prerequisites:

   * Xcode with command line tools (Xcode 14-16 tested)
   * `CMake <https://cmake.org/download/>`_ (via Homebrew: ``brew install cmake``)
   * `Faust compiler <https://faust.grame.fr/>`_ (via Homebrew: ``brew install faust``)
   * Python 3.11-3.14 (install from `python.org <https://www.python.org/downloads/>`_ with standard settings)

2. Set environment variables:

   .. code-block:: bash

      export PYTHONMAJOR=3.11
      export pythonLocation=/Library/Frameworks/Python.framework/Versions/3.11
      export ARCHS=arm64  # or x86_64 for Intel Macs

3. Build libsamplerate:

   .. code-block:: bash

      cd thirdparty/libsamplerate
      mkdir -p build_release && cd build_release
      cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=$ARCHS
      make -j$(sysctl -n hw.ncpu)
      cd ../../..

4. Run the build script:

   .. code-block:: bash

      ./build_macos.sh

   This will automatically copy ``dawdreamer.so`` to the ``tests`` folder for testing.

Building a Wheel
~~~~~~~~~~~~~~~~

After completing the platform-specific build:

1. Install build dependencies:

   .. code-block:: bash

      python -m pip install build wheel

2. Build the wheel:

   .. code-block:: bash

      # macOS: set architecture first
      export ARCHS=arm64  # or x86_64

      python -m build --wheel

3. Install the wheel:

   .. code-block:: bash

      python -m pip install dist/dawdreamer-*.whl

Faust Libraries
~~~~~~~~~~~~~~~

The `Faust Libraries <https://github.com/grame-cncm/faustlibraries>`_ are included in the ``dawdreamer/faustlibraries`` directory.

If you're **not** building a wheel, you need to manually install the Faust Libraries:

* **macOS/Linux**: Download to ``/usr/local/share/faust`` or ``/usr/share/faust``

  Example: ``/usr/local/share/faust/stdfaust.lib``

* **Windows**: Download to ``C:/share/faust``

  Example: ``C:/share/faust/stdfaust.lib``

.. note::
   When building wheels, the Faust Libraries are automatically included as package data.

Verification
------------

Test your installation:

.. code-block:: python

   import dawdreamer as daw
   engine = daw.RenderEngine(44100, 512)
   print("DawDreamer installed successfully!")

For more comprehensive testing, see the `tests directory <https://github.com/DBraun/DawDreamer/tree/main/tests>`_.

Troubleshooting
---------------

If you encounter issues:

1. Check the `GitHub Issues <https://github.com/DBraun/DawDreamer/issues>`_ for known problems
2. Verify Python version: ``python --version`` (should be 3.11-3.14)
3. On macOS, ensure you're using the correct architecture arm64
4. On Windows, ensure you're using the x64 Native Tools Command Prompt
5. See the `CLAUDE.md <https://github.com/DBraun/DawDreamer/blob/main/CLAUDE.md>`_ file for detailed troubleshooting

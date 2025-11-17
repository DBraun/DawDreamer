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
* 64-bit Python 3.11-3.14 (tested with 3.12 on Ubuntu/WSL2)
* For WSL2 users: Expect longer installation times (1-2 minutes) due to cross-filesystem I/O

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
        libncurses-dev \
        git \
        cmake \
        python3 \
        python3-dev

   .. note::
      Replace ``python3-dev`` with your specific Python version if needed (e.g., ``python3.12-dev``).
      Python 3.11-3.14 are supported. Check your version with ``python3 --version``.

   .. note::
      **ncurses/tinfo dependency (Linux only)**: The ``libncurses-dev`` package provides ``libtinfo.so`` which is
      required for Faust's LLVM linking on Linux. On some systems you may need ``libtinfo-dev`` instead.
      This is NOT needed on macOS (ncurses is part of the system).
      Verify after build with: ``ldd dawdreamer/dawdreamer.so | grep tinfo``

2. Set environment variables (adjust version as needed):

   .. code-block:: bash

      export PYTHONLIBPATH=/usr/lib/python3.12
      export PYTHONINCLUDEPATH=/usr/include/python3.12

3. Build libsamplerate:

   .. code-block:: bash

      cd thirdparty/libsamplerate
      cmake -DCMAKE_BUILD_TYPE=Release -Bbuild_release
      cmake --build build_release --config Release
      cd ../..

4. Run the build script:

   .. code-block:: bash

      sh build_linux.sh

5. Install the Python package:

   .. code-block:: bash

      python3 setup.py develop

   .. note::
      On WSL2 with NTFS filesystems, this step can take 1-2 minutes due to processing
      thousands of Faust library files across the filesystem boundary. This is normal.
      You'll see "Building editable for dawdreamer (pyproject.toml): still running..."
      which indicates progress is being made.

6. Verify the installation:

   .. code-block:: python

      python3 -c "import dawdreamer as daw; engine = daw.RenderEngine(44100, 512); print('Success!')"

**Streamlined Installation for Automation/LLMs:**

If you want minimal command-line output (useful for LLMs or automated builds):

.. code-block:: bash

   # Quick install if C++ library already exists (minimal output)
   python3 setup.py develop --quiet 2>&1 | grep -E '(Successfully|ERROR|Failed)' || \
     (echo "Installing (1-2 min on WSL2)..." && \
      python3 setup.py develop --quiet && \
      echo "✓ Installed successfully")

   # Verify silently
   python3 -c "import dawdreamer; dawdreamer.RenderEngine(44100, 512)" && \
     echo "✓ Working" || echo "✗ Failed"

.. note::
   The ``--quiet`` flag suppresses verbose output. On WSL2, you'll see a brief pause
   (1-2 minutes) while processing Faust libraries - this is normal.

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
5. On Linux/WSL2, ``setup.py develop`` taking 1-2 minutes is normal when processing Faust libraries
6. If the C++ library is already built (``dawdreamer/dawdreamer.so`` exists), you can skip the build steps and just run ``python3 setup.py develop``
7. See the `CLAUDE.md <https://github.com/DBraun/DawDreamer/blob/main/CLAUDE.md>`_ file for detailed troubleshooting

Common Issues
~~~~~~~~~~~~~

**Linux: "setup.py develop" is taking a long time**

This is expected behavior on WSL2/NTFS. The installation processes ~200+ architecture files
and ~50+ Faust library directories across the filesystem boundary. Wait 1-2 minutes for completion.
You'll see "Successfully installed dawdreamer-0.8.4" when done.

**ImportError after installation**

Verify that the installation completed:

.. code-block:: bash

   python3 -c "import dawdreamer; print('Installed successfully')"

If it fails, try reinstalling:

.. code-block:: bash

   python3 setup.py develop --uninstall
   python3 setup.py develop

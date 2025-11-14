Release Notes
=============

This page documents major changes and new features across DawDreamer versions.

.. note::
   **Versioning**: DawDreamer uses effort-based versioning. Version numbers reflect the scope of changes rather than strict semantic versioning rules.

v0.8.4 (2025)
-------------

**Infrastructure upgrades:**

* **Switched from pybind11 to nanobind** - Modern C++17 Python bindings with improved performance and smaller binary size
* **Updated libfaust to 2.81.10** - Latest Faust compiler with bug fixes and improvements
* **Comprehensive documentation overhaul**:

  * Migrated GitHub Wiki content to Sphinx documentation
  * New user guide with detailed processor documentation
  * Added Faust Box and Signal API reference
  * Improved Quick Start guide with ``record`` and ``get_parameter_range()`` examples
  * Better organization with dedicated pages for Playback and PlaybackWarp processors
  * Switched to Read the Docs theme for improved navigation

**Under the hood:**

* Better cross-platform build system
* Improved Python 3.10-3.12 compatibility
* Enhanced type hints and docstrings

v0.8.3 (2024-09-09)
-------------------

**System Requirements:**

* macOS builds require macOS 12 (Monterey) or later
* All platforms require Python 3.10 or higher

**Plugin Processor enhancements:**

* ``get_parameter_range(index: int, search_steps: int = 1000, convert: bool = True)`` - Extract parameter ranges with optional text-to-numerical conversion

**Faust Processor enhancements:**

* **Updated to libfaust 2.70.3** - Latest Faust compiler version
* ``opt_level`` property - Control LLVM optimization level
* ``.faust_libraries_paths`` - Support for multiple library paths (plural)
* ``.faust_assets_paths`` - Support for multiple asset paths (plural)
* ``boxFFun`` and ``sigFFun`` added to Box and Signal APIs
* Fixed bug where ``boxVGroup`` incorrectly referenced ``boxHGroup``

v0.8.0 (2023-11-17)
-------------------

**Breaking Changes:**

* ``boxFromDSP`` now returns just a ``box`` instead of tuple ``(box, inputs, outputs)``
* ``getBoxType`` has been removed

**Box API improvements:**

* Box objects gained new properties: ``.valid``, ``.inputs``, and ``.outputs``
* ``FaustProcessor.compile_box`` now searches for library files in ``.faust_libraries_path``

**Dependency updates:**

* Updated libfaust to 2.69.3
* Updated JUCE to 7.0.8
* Added Python 3.12 support in PyPI builds

**Documentation:**

* Minor refinements to Faust-to-JAX and Faust-to-QDax notebook documentation

v0.7.4 (2023-09-26)
-------------------

**System Requirements:**

* Python 3.8+ required for Windows/Linux
* Python 3.9+ required for macOS

**Bug Fixes:**

* Improved MIDI note termination between renders - ensures all notes turn off properly when a render concludes before a Note Off event
* Corrected off-by-one sample error in ``RenderEngine.cpp`` affecting output size with block size 2

**Dependency updates:**

* JUCE upgraded to 7.0.7
* libfaust updated to 2.68.1
* Updated pybind11, rubberband, libsamplerate

**Faust Processor enhancements:**

* ``soundfile`` primitive can now load directly from filesystem with automatic fallback to ``set_soundfiles()`` dictionary
* Enhanced ``boxSoundfile`` support for loading files specified via compilation
* Python numeric types can implicitly convert to ``boxReal``/``boxInt`` and ``sigReal``/``sigInt`` functions

v0.7.3 (2023-09-18)
-------------------

**System Requirements:**

* PyPI now requires Python 3.8+ for Windows/Linux
* PyPI now requires Python 3.9+ for macOS

**Bug Fixes:**

* Enhanced MIDI note handling between renders to ensure notes turn off properly if a render ends before a Note Off event
* Corrected output render size calculation in ``RenderEngine.cpp`` (off-by-one sample when block size = 2)

**Dependency updates:**

* JUCE upgraded to 7.0.7
* libfaust updated to 2.68.1
* Updated pybind11

**Faust Processor enhancements:**

* ``soundfile`` primitive can load directly from filesystem with automatic fallback
* Python numeric types can implicitly convert to Box and Signal API functions

**Code organization:**

* Refactored Sampler Processor Plugin project
* Refactored libfaust Box/Signal API files

v0.7.1 (2023-06-01)
-------------------

**Multiprocessing support:**

* Added multiprocessing capabilities with new tests and examples
* See ``tests/test_multiprocessing.py`` for usage examples

**Architecture improvements:**

* Removed mutex in ``PluginProcessor`` (no longer necessary)
* JUCE ``MessageManager`` instance now created before ``AudioProcessorGraph`` for better performance

v0.7.0 (2023-05-18)
-------------------

**Breaking Changes:**

* Dropped PyPI builds for macOS Python 3.7/3.8 - Python 3.9+ required
* Minimum macOS version raised to 11.0

**Bug Fixes:**

* Fixed crash issue with ``PluginProcessor.record_automation``
* Fixed certain ``isBox`` functions in the Faust Box API

**Faust updates:**

* Updated to Faust 2.59.6 and associated Faust Libraries
* Enhanced Box/Signal APIs with new functions

**Build system changes:**

* Removed libfaust files from repository
* Build now dynamically downloads libfaust
* Updated faust, faustlibraries, and pybind11 submodules

**Known Issues:**

* `Issue #152 <https://github.com/DBraun/DawDreamer/issues/152>`_ identified as high-priority bug (unresolved)

v0.6.3 (2022-04-19)
-------------------

**Plugin Processor enhancements:**

* ``open_editor()`` - Open plugin GUI for editing
* ``load_state(filepath: str)`` - Load complete plugin state
* ``save_state(filepath: str)`` - Save complete plugin state
* ``can_set_bus(inputs: int, outputs: int)`` - Check bus configuration support
* ``set_bus(inputs: int, outputs: int)`` - Set multi-channel bus configuration

See ``tests/test_plugins.py`` for usage examples.

v0.6.1 (2022-04-04)
-------------------

**Playback Warp Processor improvements:**

* Fixed issue when warping is disabled (`Issue #81 <https://github.com/DBraun/DawDreamer/issues/81>`_)
* Added ``reset_warp_markers(bpm: float)`` - Reset warp markers to steady BPM
* Added ``warp_markers`` property - Set warp markers as 2D array (`Issue #73 <https://github.com/DBraun/DawDreamer/issues/73>`_)

See ``tests/test_playbackwarp_processor.py`` for usage examples.

v0.6.0 (2022-03-24)
-------------------

**Breaking Changes:**

* Functions like ``load_graph()`` now **throw exceptions** instead of returning bools
* Previously: ``assert engine.load_graph(graph)``
* Now: Use ``try/except`` blocks to catch errors instead of ``assert``
* **Benefit**: Clearer error messages without needing ``assert`` everywhere

**Non-breaking changes:**

* Fixed bug in Faust processor MIDI file loading
* Fixed bug in Add Processor channel count (now always stereo)
* Added ``all_events`` keyword arg to ``load_midi()`` for Plugin Processor (default: True for backwards compatibility)
* Added ``faust_libraries_path`` property to Faust Processor for custom ``.lib`` file locations
* Added Faust property for release length to avoid false-positive voice stealing warnings
* Revised Render Engine graph construction to warn instead of error when too many signals connect to a node
* Upgraded JUCE, Faust, and pybind11 dependencies

v0.5.8.1 (2021-01-13)
---------------------

**Multi-channel support:**

* Multi-channel support for VSTs, Faust, and other processors
* Added ``get_num_input_channels()`` and ``get_num_output_channels()``

**macOS improvements:**

* Better macOS support for all CPU types (Apple Silicon and Intel)
* Improved ``pip install dawdreamer`` experience

v0.5.7.8 (2021-10-09)
---------------------

**Faust soundfile primitive:**

* Faust code can now use the ``soundfile`` primitive
* Pass NumPy audio from Python to Faust with ``set_soundfiles()`` method
* Enables sample-based instruments and convolution reverbs

v0.5.7 (2021-08-14)
-------------------

**Breaking Change:**

* **Plugin Processor automation changed**: ``set_automation(str, np.array)`` → ``set_automation(int, np.array)``
* **Reason**: Parameter names aren't always unique, so parameters must be identified by index
* **Migration**: Use parameter index instead of name string

v0.5.6 (2021-05-09)
-------------------

**Major additions:**

* **Faust Processor added** - Real-time Faust DSP compilation and execution
* Removed JUCE from global namespace (cleaner API)

v0.5.0 (2021-04-01)
-------------------

**Linux support:**

* Added Linux build (thanks `@guillaumephd <https://github.com/guillaumephd>`_)
* Expanded platform support to macOS, Windows, and Linux

v0.4.0 (2020-12-01)
-------------------

**Parameter automation:**

* Added parameter automation feature
* Support for audio-rate and PPQN-rate automation
* Enables dynamic parameter control over time

v0.1.0 (2020-08-14)
-------------------

**Initial release:**

* First public release of DawDreamer
* Core RenderEngine and processor graph system
* VST plugin hosting
* Basic audio processing capabilities

Upgrade Guide
-------------

v0.8.0 Breaking Changes
~~~~~~~~~~~~~~~~~~~~~~~

**Old code:**

.. code-block:: python

   box, inputs, outputs = boxFromDSP(dsp_string)
   box_type = getBoxType(box)

**New code:**

.. code-block:: python

   box = boxFromDSP(dsp_string)
   # Use new Box properties instead
   inputs = box.inputs
   outputs = box.outputs
   is_valid = box.valid
   # getBoxType() removed - use Box properties

v0.7.0 Breaking Changes
~~~~~~~~~~~~~~~~~~~~~~~

**Platform requirements changed:**

* macOS builds now require Python 3.9+ (dropped 3.7/3.8)
* macOS minimum version raised to 11.0

If using macOS Python 3.7 or 3.8, upgrade to Python 3.9 or later.

v0.6.0 Breaking Changes
~~~~~~~~~~~~~~~~~~~~~~~

**Old code:**

.. code-block:: python

   assert engine.load_graph(graph)
   assert synth.set_dsp(dsp_path)

**New code:**

.. code-block:: python

   try:
       engine.load_graph(graph)
       synth.set_dsp(dsp_path)
   except RuntimeError as e:
       print(f"Error: {e}")

v0.5.7 Breaking Changes
~~~~~~~~~~~~~~~~~~~~~~~

**Old code:**

.. code-block:: python

   synth.set_automation("A Pan", automation_data)

**New code:**

.. code-block:: python

   # Get parameter index first
   params = synth.get_parameters_description()
   # Find index of "A Pan" parameter
   index = 1  # Example

   synth.set_automation(index, automation_data)

Versioning Scheme
-----------------

DawDreamer uses **effort-based versioning**, where version numbers reflect the scope and effort of changes rather than strict semantic versioning rules.

* **Major changes** (0.X.0): Significant new features, infrastructure upgrades, or architectural changes
* **Minor changes** (0.0.X): Bug fixes, small improvements, documentation updates

.. note::
   Breaking API changes are noted explicitly in release notes. Check the Upgrade Guide for migration instructions when breaking changes occur.

Future Roadmap
--------------

Planned features (subject to change):

* Additional audio effects and processors
* Improved VST3 compatibility
* Real-time audio I/O support
* MIDI input/output devices
* Enhanced automation curves (bezier, exponential, etc.)
* More Faust transpilation targets

See the `GitHub Issues <https://github.com/DBraun/DawDreamer/issues>`_ for feature requests and known bugs.

Contributing
------------

DawDreamer is open source (GPLv3). Contributions are welcome:

* **Bug reports**: `Open an issue <https://github.com/DBraun/DawDreamer/issues>`_
* **Feature requests**: Discuss in `GitHub Discussions <https://github.com/DBraun/DawDreamer/discussions>`_
* **Code contributions**: Submit a pull request
* **Documentation**: Help improve these docs

See the `Contributing Guide <https://github.com/DBraun/DawDreamer/blob/main/CONTRIBUTING.md>`_ for details.

See Also
--------

* :doc:`installation` - Installation instructions
* :doc:`quickstart` - Getting started guide
* `GitHub Releases <https://github.com/DBraun/DawDreamer/releases>`_ - Download specific versions
* `PyPI <https://pypi.org/project/dawdreamer/>`_ - Python Package Index page

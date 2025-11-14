DawDreamer Documentation
=========================

DawDreamer is a **Digital Audio Workstation (DAW) framework for Python**. It enables programmatic audio processing with support for VST plugins, Faust DSP code, and complex audio routing graphs.

Read the `introduction to DawDreamer <https://arxiv.org/abs/2111.09931>`_, which was presented as a Late-Breaking Demo at the `2021 ISMIR Conference <https://ismir2021.ismir.net/lbd/>`_.

.. image:: https://img.shields.io/badge/platform-macOS%20%7C%20Windows%20%7C%20Linux-lightgrey
   :alt: Platform support

.. image:: https://img.shields.io/pypi/v/dawdreamer
   :target: https://pypi.org/project/dawdreamer/
   :alt: PyPI version

.. image:: https://img.shields.io/badge/License-GPL%20v3-blue.svg
   :target: https://www.gnu.org/licenses/gpl-3.0
   :alt: License: GPL v3

Key Features
------------

* **Composable audio processor graphs** - Build complex signal processing chains with `DAG <https://en.wikipedia.org/wiki/Digital_audio_workstation>`_-based routing
* **VST 2/3 support** - Host instrument and effect plugins with full state management and automation
* **Faust DSP integration** - Real-time compilation of Faust code for custom effects and instruments
* **Time-stretching and pitch-shifting** - RubberBand integration with Ableton Live warp marker support
* **Parameter automation** - Audio-rate and PPQN-based automation for precise control
* **MIDI support** - Playback, recording, and file export
* **Multi-platform** - macOS (Apple Silicon/Intel), Windows, Linux, Google Colab, Docker
* **Transpilation** - Convert Faust to `JAX/Flax <https://github.com/DBraun/DawDreamer/tree/main/examples/Faust_to_JAX>`_, C++, Rust, WebAssembly
* **Faust Box and Signal APIs** - Advanced Faust programming with the `Box API <https://github.com/DBraun/DawDreamer/tree/main/examples/Box_API>`_
* **Machine learning experiments** - Integration with `QDax <https://github.com/DBraun/DawDreamer/tree/main/examples/Faust_to_QDax>`_
* **Multiprocessing support** - Parallel processing capabilities (`example <https://github.com/DBraun/DawDreamer/tree/main/examples/multiprocessing_plugins>`_)

Quick Start
-----------

Installation
~~~~~~~~~~~~

.. code-block:: bash

   pip install dawdreamer

Basic Example
~~~~~~~~~~~~~

.. code-block:: python

   import dawdreamer as daw
   import numpy as np

   # Create engine
   engine = daw.RenderEngine(44100, 512)

   # Add a Faust processor
   faust = engine.make_faust_processor("synth")
   faust.set_dsp_string("process = os.osc(440) * 0.1;")

   # Load graph and render
   engine.load_graph([(faust, [])])
   engine.render(2.0)  # 2 seconds

   # Get audio
   audio = engine.get_audio()  # shape: (channels, samples)

Documentation
-------------

.. toctree::
   :maxdepth: 2
   :caption: User Guide

   installation
   quickstart
   user_guide/index
   examples

.. toctree::
   :maxdepth: 2
   :caption: API Reference

   api_reference/index

.. toctree::
   :maxdepth: 1
   :caption: Additional Resources

   compatibility
   release_notes

Links
-----

* **GitHub Repository**: https://github.com/DBraun/DawDreamer
* **Examples**: https://github.com/DBraun/DawDreamer/tree/main/examples
* **Issue Tracker**: https://github.com/DBraun/DawDreamer/issues
* **Original Paper**: https://arxiv.org/abs/2111.09931

Built With
----------

DawDreamer's foundation is `JUCE <https://github.com/juce-framework/JUCE>`_, with a user-friendly Python interface thanks to `nanobind <https://github.com/wjakob/nanobind>`_. DawDreamer evolved from an earlier VSTi audio "renderer", `RenderMan <https://github.com/fedden/RenderMan>`_.

License
-------

DawDreamer is licensed under GPLv3 to make it easier to comply with all of the dependent projects. If you use DawDreamer, you must obey the licenses of JUCE, nanobind, `Libsamplerate <https://github.com/libsndfile/libsamplerate>`_, `Rubber Band Library <https://github.com/breakfastquay/rubberband/>`_, `Steinberg VST2/3 <https://www.steinberg.net/vst-instruments/>`_, and `FAUST <https://github.com/grame-cncm/faust>`_.

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

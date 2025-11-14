Examples
========

DawDreamer includes several complete examples demonstrating real-world use cases. All examples are available in the `examples/ directory <https://github.com/DBraun/DawDreamer/tree/main/examples>`_ on GitHub.

Faust Box API
-------------

**Location**: `examples/Box_API/ <https://github.com/DBraun/DawDreamer/tree/main/examples/Box_API>`_

Demonstrates using Faust's Box API to programmatically construct DSP graphs in Python. The Box API allows building complex audio processing networks without writing Faust DSP code directly.

**Topics covered:**

* Building Faust processors from Python using the Box API
* Creating oscillators, filters, and effects programmatically
* Composing complex DSP graphs
* Converting Box API code to Faust DSP strings

**Format**: Jupyter Notebook

Faust to JAX/Flax Transpilation
--------------------------------

**Location**: `examples/Faust_to_JAX/ <https://github.com/DBraun/DawDreamer/tree/main/examples/Faust_to_JAX>`_

Shows how to transpile Faust DSP code to JAX/Flax for differentiable audio processing and machine learning applications.

**Topics covered:**

* Transpiling Faust code to JAX
* Creating differentiable audio processors
* Integration with Flax neural networks
* Gradient-based optimization of DSP parameters

**Format**: Jupyter Notebook

**Use cases:**

* Neural audio synthesis
* Differentiable audio effects
* Machine learning for audio processing

Faust to QDax
-------------

**Location**: `examples/Faust_to_QDax/ <https://github.com/DBraun/DawDreamer/tree/main/examples/Faust_to_QDax>`_

Demonstrates integration with QDax (Quality Diversity in JAX) for evolutionary audio synthesis.

**Topics covered:**

* Transpiling Faust to QDax-compatible formats
* Evolutionary optimization of synthesizer parameters
* Quality diversity algorithms for sound design
* Fitness functions for audio evaluation

**Format**: Jupyter Notebook

**Use cases:**

* Automated sound design
* Evolutionary synthesis
* Parameter space exploration

Mashup Generator
----------------

**Location**: `examples/mashup_generator/ <https://github.com/DBraun/DawDreamer/tree/main/examples/mashup_generator>`_

Creates audio mashups by beat-matching and mixing multiple songs.

**Topics covered:**

* Beat detection and tempo analysis
* Time-stretching for tempo matching
* Multi-track mixing and synchronization
* Crossfading and transitions

**Format**: Jupyter Notebook

**Use cases:**

* DJ-style mashups
* Automated remixing
* Beat-matched playlists

DJ Mixing
---------

**Location**: `examples/dj_mixing/ <https://github.com/DBraun/DawDreamer/tree/main/examples/dj_mixing>`_

Demonstrates DJ-style mixing techniques including beatmatching, crossfading, and EQ mixing.

**Topics covered:**

* Tempo synchronization
* Crossfader automation
* EQ filtering for smooth transitions
* Cue point management

**Format**: Python script + README

**Use cases:**

* Automated DJ sets
* Mix generation
* Audio analysis and matching

Multiprocessing with Plugins
-----------------------------

**Location**: `examples/multiprocessing_plugins/ <https://github.com/DBraun/DawDreamer/tree/main/examples/multiprocessing_plugins>`_

Shows how to use Python's multiprocessing to render multiple VST plugin instances in parallel.

**Topics covered:**

* Parallel processing of plugin chains
* Batch rendering optimization
* Process pool management
* Combining results from multiple renders

**Format**: Python script + README

**Use cases:**

* Batch audio processing
* Preset rendering
* Dataset generation
* Performance optimization

XML Synth Sound Rendering
--------------------------

**Location**: `examples/xml_synth_sound_rendering/ <https://github.com/DBraun/DawDreamer/tree/main/examples/xml_synth_sound_rendering>`_

Renders synthesizer presets defined in XML configuration files, useful for creating sound libraries or datasets.

**Topics covered:**

* XML-based preset definition
* Batch rendering of synth sounds
* Parameter randomization
* Dataset creation for machine learning

**Format**: Jupyter Notebook + Python scripts + README

**Use cases:**

* Sound library creation
* ML dataset generation
* Preset exploration
* Automated sound design

Running the Examples
--------------------

Prerequisites
~~~~~~~~~~~~~

Install DawDreamer and common dependencies:

.. code-block:: bash

   pip install dawdreamer
   pip install librosa scipy numpy matplotlib jupyter

Some examples may require additional dependencies:

* **JAX/Flax examples**: ``pip install jax flax``
* **QDax examples**: ``pip install qdax``
* **VST plugin examples**: Appropriate VST plugins installed

Jupyter Notebooks
~~~~~~~~~~~~~~~~~

Launch Jupyter and navigate to the examples:

.. code-block:: bash

   cd examples/
   jupyter notebook

Open the desired ``.ipynb`` file and run the cells.

Python Scripts
~~~~~~~~~~~~~~

Run standalone examples directly:

.. code-block:: bash

   cd examples/dj_mixing/
   python dj_mixing.py

Check each example's README for specific instructions.

Additional Resources
--------------------

**Tests Directory**

The `tests/ directory <https://github.com/DBraun/DawDreamer/tree/main/tests>`_ contains comprehensive unit tests that also serve as usage examples:

* ``test_faust_processor.py`` - Faust DSP examples
* ``test_plugin_processor.py`` - VST plugin usage
* ``test_faust_poly*.py`` - Polyphonic synthesis
* ``test_faust_soundfile.py`` - Sample-based instruments
* And many more...

**Wiki (Archived)**

Previously, the `DawDreamer Wiki <https://github.com/DBraun/DawDreamer/wiki>`_ contained tutorials and guides. This content has been migrated to this documentation.

**Community Examples**

Check the `GitHub issues <https://github.com/DBraun/DawDreamer/issues>`_ and `discussions <https://github.com/DBraun/DawDreamer/discussions>`_ for community-contributed examples and use cases.

Contributing Examples
---------------------

If you've created a useful DawDreamer example:

1. Add it to the `examples/ directory <https://github.com/DBraun/DawDreamer/tree/main/examples>`_
2. Include a README explaining the example
3. Add requirements.txt if using additional dependencies
4. Submit a pull request

Good examples include:

* Clear documentation and comments
* Self-contained code (minimal dependencies)
* Real-world use cases
* Educational value

Next Steps
----------

* Explore the :doc:`user_guide/index` for detailed processor documentation
* Check the :doc:`api_reference/index` for complete API details
* See the :doc:`quickstart` for basic usage patterns

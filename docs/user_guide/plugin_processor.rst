Plugin Processor
================

The ``PluginProcessor`` hosts VST 2/3 and Audio Unit (AU) plugins as instruments and effects. It provides full state management, parameter automation, and MIDI support.

Basic Usage
-----------

Creating a Plugin Processor
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import dawdreamer as daw

   SAMPLE_RATE = 44100
   BUFFER_SIZE = 128
   SYNTH_PLUGIN = "/path/to/synth.dll"  # .dll, .vst3, .vst, .component

   engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

   # Make a processor with a unique name
   synth = engine.make_plugin_processor("my_synth", SYNTH_PLUGIN)
   assert synth.get_name() == "my_synth"

**Supported formats:**

* Windows: ``.dll``, ``.vst3``
* macOS: ``.vst``, ``.vst3``, ``.component`` (AU)
* Linux: ``.so``, ``.vst3``

Plugin State Management
-----------------------

Opening the Editor
~~~~~~~~~~~~~~~~~~

Many plugins have graphical user interfaces (GUIs) that you can open to tweak parameters visually:

.. code-block:: python

   synth.open_editor()  # Opens the plugin's UI window

.. warning::
   ``open_editor()`` is blocking - it will pause Python execution until you close the editor window.

Saving and Loading State
~~~~~~~~~~~~~~~~~~~~~~~~~

Save the plugin's complete state (all parameter values, internal buffers, etc.):

.. code-block:: python

   # Save state after editing
   synth.save_state('/path/to/state1')

   # Load state without opening editor
   synth.load_state('/path/to/state1')

State files contain all plugin settings in a format specific to DawDreamer.

Loading Presets
~~~~~~~~~~~~~~~

Load plugin-specific preset files:

.. code-block:: python

   # VST2 FXP format
   synth.load_preset('/path/to/preset.fxp')

   # VST3 preset format
   synth.load_vst3_preset('/path/to/preset.vstpreset')

Parameters
----------

Listing Parameters
~~~~~~~~~~~~~~~~~~

Get descriptions of all available parameters:

.. code-block:: python

   # Returns a list of dictionaries
   params = synth.get_parameters_description()
   print(params)

   # Get specific parameter name by index
   param_name = synth.get_parameter_name(1)
   print(param_name)  # Example: "A Pan" for Serum oscillator A panning

Setting Parameters
~~~~~~~~~~~~~~~~~~

.. note::
   Plugin processor parameters are **always normalized to [0, 1]**, even for discrete parameters.

.. code-block:: python

   # Set parameter by index
   synth.set_parameter(1, 0.1234)

   # Set parameter by name (if supported)
   synth.set_parameter("A Pan", 0.5)

Getting Parameter Values
~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   value = synth.get_parameter(1)
   print(f"Parameter 1 value: {value}")

Getting Parameter Ranges
~~~~~~~~~~~~~~~~~~~~~~~~~

For discrete parameters (like oscillator waveform selectors), you can query the valid ranges:

.. code-block:: python

   parameter_index = 10

   # Get parameter range with converted float values
   par_range = synth.get_parameter_range(parameter_index, search_steps=1000, convert=True)

   # par_range is a dict with (min_value, max_value) tuples as keys
   # and corresponding labels/values as dict values
   print(par_range)
   # Example output: {(0.0, 0.25): 0.0, (0.25, 0.5): 0.333, (0.5, 0.75): 0.667, (0.75, 1.0): 1.0}

   # Get parameter range as strings (useful for text labels)
   par_range_str = synth.get_parameter_range(parameter_index, search_steps=1000, convert=False)
   print(par_range_str)
   # Example output: {(0.0, 0.25): "Saw", (0.25, 0.5): "Square", (0.5, 0.75): "Triangle", (0.75, 1.0): "Sine"}

**Parameters:**

* ``parameter_index`` (int): The parameter index to query
* ``search_steps`` (int, default=1000): Number of steps to search for discrete values
* ``convert`` (bool, default=True): If True, return numeric values; if False, return string labels

**Return value:**

Dictionary with ``(min, max)`` tuples as keys, where ``min < max``. Values are either floats (if ``convert=True``) or strings (if ``convert=False``).

**Use cases:**

* Discovering discrete parameter values (e.g., waveform types, filter modes)
* Building UI controls with labeled options
* Understanding parameter stepping behavior

Parameter Automation
--------------------

Plugin processors support both audio-rate and PPQN-rate parameter automation.

.. seealso::
   See :ref:`Parameter Automation <parameter-automation>` in the User Guide for complete documentation on automation modes, recording automation, and examples.

**Quick example:**

.. code-block:: python

   import numpy as np

   # Audio-rate automation (one value per sample)
   duration = 10.0
   num_samples = int(duration * 44100)
   automation = 0.5 + 0.5 * np.sin(np.linspace(0, 4*np.pi, num_samples))
   synth.set_automation(1, automation)  # Parameter index 1

   # PPQN-rate automation (musically-synced)
   ppqn = 960
   beats = 20
   pulses = beats * ppqn
   automation_ppqn = np.linspace(0, 1, pulses)
   synth.set_automation(1, automation_ppqn, ppqn=ppqn)

MIDI Support
------------

Adding Individual Notes
~~~~~~~~~~~~~~~~~~~~~~~

Add MIDI notes one at a time:

.. code-block:: python

   # add_midi_note(note, velocity, start_time, duration)
   synth.add_midi_note(60, 127, 0.5, 0.25)  # Middle C at 0.5s for 0.25s

   # Use beats=True for musical timing
   synth.add_midi_note(67, 127, 1, 0.5, beats=True)  # G4 at beat 1 for half a beat

Loading MIDI Files
~~~~~~~~~~~~~~~~~~

Load MIDI files with absolute time (seconds):

.. code-block:: python

   MIDI_PATH = "/path/to/song.mid"

   # Load in absolute time (seconds) - BPM changes won't affect timing
   synth.load_midi(MIDI_PATH, clear_previous=True, beats=False, all_events=True)

Load MIDI files with beat-relative timing:

.. code-block:: python

   # Load in beat time - BPM changes WILL affect timing
   synth.load_midi(MIDI_PATH, beats=True)

**Parameters:**

* ``clear_previous``: Clear existing MIDI data (default: True)
* ``beats``: Use beat timing instead of seconds (default: False)
* ``all_events``: Include all MIDI events, not just notes (default: True)

Saving MIDI Files
~~~~~~~~~~~~~~~~~

Export MIDI data with absolute time:

.. code-block:: python

   # After rendering
   synth.save_midi("my_midi_output.mid")

Clearing MIDI
~~~~~~~~~~~~~

Remove all MIDI notes and events:

.. code-block:: python

   synth.clear_midi()

Graph Integration
-----------------

Basic Chain
~~~~~~~~~~~

.. code-block:: python

   synth = engine.make_plugin_processor("synth", SYNTH_PLUGIN)
   reverb = engine.make_plugin_processor("reverb", REVERB_PLUGIN)

   graph = [
       (synth, []),                          # No inputs
       (reverb, [synth.get_name()])          # Processes synth output
   ]

   engine.load_graph(graph)
   engine.render(5.0)  # Render 5 seconds

Re-rendering
~~~~~~~~~~~~

Modify processors and re-render without rebuilding the graph:

.. code-block:: python

   engine.render(5.0)
   audio1 = engine.get_audio()

   # Modify plugin
   synth.load_preset("/path/to/other_preset.fxp")

   # Re-render (MIDI is still loaded)
   engine.render(5.0)
   audio2 = engine.get_audio()

Multi-Channel Buses
-------------------

Some plugins support multiple input/output channel configurations (e.g., ambisonics, surround).

Checking Bus Support
~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   plugin = engine.make_plugin_processor("amb", "/path/to/ambisonics.dll")

   # Check if 1 input, 9 outputs is supported
   if plugin.can_set_bus(1, 9):
       plugin.set_bus(1, 9)

Sidechain Processing
~~~~~~~~~~~~~~~~~~~~

Some plugins accept multiple inputs (e.g., sidechain compressors):

.. code-block:: python

   import librosa

   # Create processors
   sidechain_comp = engine.make_plugin_processor("sidechain", "/path/to/sidechain.dll")

   vocals_audio, _ = librosa.load("vocals.wav", sr=44100, mono=False)
   piano_audio, _ = librosa.load("piano.wav", sr=44100, mono=False)

   vocals = engine.make_playback_processor("vocals", vocals_audio)
   piano = engine.make_playback_processor("piano", piano_audio)

   # Sidechain: reduce vocals volume based on piano
   graph = [
       (vocals, []),
       (piano, []),
       (sidechain_comp, ["vocals", "piano"])  # First input = main, second = sidechain
   ]

   engine.load_graph(graph)

Channel Counts
~~~~~~~~~~~~~~

Check a plugin's input/output channel configuration:

.. code-block:: python

   print("Inputs:", synth.get_num_input_channels())
   print("Outputs:", synth.get_num_output_channels())

Complete Example
----------------

.. code-block:: python

   import dawdreamer as daw
   import numpy as np
   from scipy.io import wavfile

   SAMPLE_RATE = 44100
   BUFFER_SIZE = 128
   PPQN = 960
   SYNTH_PLUGIN = "/path/to/synth.dll"
   REVERB_PLUGIN = "/path/to/reverb.dll"

   engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

   # Create synth
   synth = engine.make_plugin_processor("synth", SYNTH_PLUGIN)

   # Edit and save state
   synth.open_editor()
   synth.save_state('/path/to/state1')

   # Setup automation
   synth.record_automation = True
   duration = 10
   automation = 0.5 + 0.5 * np.sin(np.linspace(0, 4*np.pi, int(duration * SAMPLE_RATE)))
   synth.set_automation(1, automation)

   # Add MIDI
   synth.add_midi_note(60, 100, 0.0, 2.0)
   synth.add_midi_note(64, 100, 2.0, 2.0)
   synth.add_midi_note(67, 100, 4.0, 2.0)

   # Create effect
   reverb = engine.make_plugin_processor("reverb", REVERB_PLUGIN)

   # Build graph
   graph = [
       (synth, []),
       (reverb, ["synth"])
   ]

   engine.load_graph(graph)
   engine.set_bpm(120.)
   engine.render(10.)

   # Save audio
   audio = engine.get_audio()
   wavfile.write('plugin_output.wav', SAMPLE_RATE, audio.transpose())

   # Get automation data
   recorded_automation = synth.get_automation()

   # Save MIDI
   synth.save_midi('output.mid')

Tips and Best Practices
------------------------

**State Management**
   * Always save state after tweaking in the editor
   * State files are more complete than presets (include all settings)
   * Use ``load_state()`` for consistent reproduction

**Parameter Automation**
   * Record automation with ``record_automation = True`` if you need to retrieve it later
   * Use smaller buffer sizes (128-256) for smoother automation curves
   * PPQN automation is better for music-synced effects

**MIDI Timing**
   * Use ``beats=True`` for musical timing that adapts to tempo changes
   * Use ``beats=False`` for fixed timing independent of BPM
   * Clear MIDI with ``clear_midi()`` before adding new notes

**Performance**
   * Plugin CPU usage varies greatly by plugin
   * Some plugins have high-quality modes that increase CPU usage
   * Consider rendering offline for complex chains

**Compatibility**
   * Not all plugins are compatible with headless (no-GUI) rendering
   * Test plugins individually before building complex graphs
   * See :doc:`../compatibility` for known plugin compatibility

Common Issues
-------------

**Plugin fails to load**
   * Verify plugin path is absolute, not relative
   * Check plugin format matches platform (.dll on Windows, .vst3/.component on macOS)
   * Some plugins require installation/authorization before use

**Parameters don't change sound**
   * Some plugins require specific MIDI/audio input to produce sound
   * Check if plugin is an effect (needs input) vs instrument (generates audio)
   * Verify parameter index and value range [0, 1]

**MIDI notes not playing**
   * Check if plugin is an instrument (not an effect)
   * Verify MIDI note range (some synths have limited ranges)
   * Ensure render duration is long enough for notes to sound

**Sidechain not working**
   * Verify plugin supports sidechain input
   * Check input order in graph (main signal first, sidechain second)
   * Some plugins require internal routing configuration

**Automation sounds stepped/quantized**
   * Decrease buffer size for finer automation resolution
   * Check if plugin has internal parameter smoothing
   * Verify automation array length matches render duration

Further Reading
---------------

* :doc:`render_engine` - BPM and timing
* :doc:`faust_processor` - Custom DSP alternative to plugins
* :doc:`../compatibility` - Plugin compatibility table
* :doc:`../examples` - Real-world plugin examples

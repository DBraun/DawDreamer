Quick Start Guide
=================

This guide will get you up and running with DawDreamer in minutes.

Basic Concepts
--------------

DawDreamer's core components:

* **RenderEngine**: The main audio engine that manages processors and renders audio
* **Processors**: Audio processing nodes (Faust, VST plugins, playback, effects, etc.)
* **Graph**: A directed acyclic graph (DAG) defining how processors connect
* **Parameters**: Controllable values that can be automated over time

Example 1: Faust Sine Wave
---------------------------

Let's create a simple stereo sine tone at 440 Hz and -6 dB using Faust DSP.

.. code-block:: python

   import dawdreamer as daw
   from scipy.io import wavfile

   SAMPLE_RATE = 44100
   engine = daw.RenderEngine(SAMPLE_RATE, 512)  # 512 sample block size

   # Create a Faust processor
   faust_processor = engine.make_faust_processor("faust")

   # Set the DSP code
   faust_processor.set_dsp_string(
       """
       declare name "MySine";
       freq = hslider("freq", 440, 0, 20000, 0);
       gain = hslider("vol[unit:dB]", 0, -120, 20, 0) : ba.db2linear;
       process = freq : os.osc : _*gain <: si.bus(2);
       """
   )

   # Inspect available parameters
   print(faust_processor.get_parameters_description())

   # Load the processor into the engine's graph
   engine.load_graph([(faust_processor, [])])

   # Set parameters by address or by index
   faust_processor.set_parameter("/MySine/freq", 440.)  # 440 Hz
   faust_processor.set_parameter("/MySine/vol", -6.)    # -6 dB volume

   # Render 4 beats at 120 BPM
   engine.set_bpm(120.)
   engine.render(4., beats=True)

   # Get the rendered audio
   audio = engine.get_audio()  # shape: (2, num_samples)

   # Save to file
   wavfile.write('sine_demo.wav', SAMPLE_RATE, audio.transpose())

   # Change settings and re-render
   faust_processor.set_parameter("/MySine/freq", 880.)  # 880 Hz
   engine.render(4., beats=True)
   audio = engine.get_audio()
   # ...and so on

Example 2: VST Plugin Chain
----------------------------

Now let's create a graph with a VST instrument and effect processor.

.. code-block:: python

   import dawdreamer as daw
   from scipy.io import wavfile

   SAMPLE_RATE = 44100
   INSTRUMENT_PATH = "path/to/instrument.dll"  # .vst3 or .dll on Windows
   EFFECT_PATH = "path/to/effect.dll"

   engine = daw.RenderEngine(SAMPLE_RATE, 512)
   engine.set_bpm(120.)

   # Load a VST instrument
   synth = engine.make_plugin_processor("synth", INSTRUMENT_PATH)

   # Inspect the plugin
   print('inputs:', synth.get_num_input_channels())
   print('outputs:', synth.get_num_output_channels())
   print(synth.get_parameters_description())

   # Get parameter range information (useful for discrete parameters)
   parameter_index = 10
   par_range = synth.get_parameter_range(parameter_index, search_steps=1000, convert=True)
   # par_range is a dict with (min, max) tuples as keys and values/labels as dict values
   # If convert=True, values are floats; if False, they're strings to interpret
   print(f"Parameter {parameter_index} range:", par_range)

   # Set a parameter by index (values are always 0.0 to 1.0)
   synth.set_parameter(7, 0.1234)

   # Add MIDI notes (note, velocity, start_time, duration)
   synth.add_midi_note(60, 100, 0.0, 2.0)  # Middle C for 2 seconds

   # Load an effect plugin
   effect = engine.make_plugin_processor("effect", EFFECT_PATH)

   # Create the processing graph
   engine.load_graph([
       (synth, []),                          # synth has no inputs
       (effect, [synth.get_name()])          # effect processes synth output
   ])

   # Render 4 seconds
   engine.render(4.0)
   audio = engine.get_audio()

   # Save to file
   wavfile.write('synth_demo.wav', SAMPLE_RATE, audio.transpose())

   # Clear MIDI and render again
   synth.clear_midi()
   synth.add_midi_note(64, 100, 0.0, 1.0)  # E for 1 second
   engine.render(2.0)
   # ...and so on

Example 3: Audio Playback and Effects
--------------------------------------

Process an existing audio file with effects and capture intermediate outputs:

.. code-block:: python

   import dawdreamer as daw
   from scipy.io import wavfile
   import librosa

   SAMPLE_RATE = 44100
   engine = daw.RenderEngine(SAMPLE_RATE, 512)

   # Load an audio file
   audio, _ = librosa.load('input.wav', sr=SAMPLE_RATE, mono=False)

   # Create a playback processor
   playback = engine.make_playback_processor("playback", audio)

   # Create a filter processor
   filter_proc = engine.make_filter_processor("filter", "high")
   filter_proc.set_frequency(2000.0)  # High-pass at 2 kHz

   # Enable recording to capture this processor's output
   filter_proc.record = True

   # Create the graph
   engine.load_graph([
       (playback, []),
       (filter_proc, ["playback"])
   ])

   # Render the full audio
   duration = audio.shape[1] / SAMPLE_RATE
   engine.render(duration)

   # Get the final output (from last processor in graph)
   filtered_audio = engine.get_audio()

   # Get the filter's output specifically (because we enabled recording)
   filter_output = filter_proc.get_audio()
   # Or: filter_output = engine.get_audio("filter")

   # Save both outputs
   wavfile.write('filtered.wav', SAMPLE_RATE, filtered_audio.transpose())
   wavfile.write('filter_only.wav', SAMPLE_RATE, filter_output.transpose())

Key Concepts Explained
-----------------------

RenderEngine
~~~~~~~~~~~~

The ``RenderEngine`` is the central orchestrator:

* Created with sample rate and block size
* Manages the processor graph
* Handles tempo and automation
* Renders audio to memory

.. code-block:: python

   engine = daw.RenderEngine(sample_rate=44100, block_size=512)

Processor Graph
~~~~~~~~~~~~~~~

The graph is a list of ``(processor, inputs)`` tuples:

* Each processor has a unique name
* Inputs are specified as a list of processor names
* The graph must be acyclic (no loops)
* Processors are executed in dependency order

.. code-block:: python

   graph = [
       (processor_a, []),                      # No inputs
       (processor_b, []),                      # No inputs
       (processor_c, ["processor_a"]),         # Takes A's output
       (processor_d, ["processor_b", "processor_c"])  # Takes B and C
   ]
   engine.load_graph(graph)

Parameter Automation
~~~~~~~~~~~~~~~~~~~~

Parameters can be automated over time:

.. code-block:: python

   import numpy as np

   # Create automation data (one value per sample)
   duration = 4.0
   num_samples = int(duration * SAMPLE_RATE)
   frequency_automation = 440 + 440 * np.sin(np.linspace(0, 4*np.pi, num_samples))

   # Apply automation
   processor.set_automation("/MySine/freq", frequency_automation)

BPM and Timing
~~~~~~~~~~~~~~

Render time can be specified in seconds or beats:

.. code-block:: python

   engine.set_bpm(120.)
   engine.render(4., beats=True)   # Render 4 beats
   engine.render(2., beats=False)  # Render 2 seconds

Next Steps
----------

* Explore the :doc:`user_guide/index` for detailed processor documentation
* Check out :doc:`examples` for real-world use cases
* Read the :doc:`api_reference/index` for complete API details
* See the `GitHub repository <https://github.com/DBraun/DawDreamer>`_ for more examples and tests

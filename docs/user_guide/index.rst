User Guide
==========

This guide covers all aspects of using DawDreamer, from basic rendering concepts to advanced processor usage.

.. toctree::
   :maxdepth: 2

   render_engine
   faust_processor
   plugin_processor
   playback
   playback_warp
   other_processors

Overview
--------

DawDreamer is built around a few core concepts:

**RenderEngine**
   The central audio processing engine that manages processors, timing, and rendering.

**Processors**
   Audio processing nodes that can generate, modify, or analyze audio. Each processor has inputs, outputs, and controllable parameters.

**Graph**
   A directed acyclic graph (DAG) that defines how processors are connected. Audio flows from processors with no inputs through the graph to processors with no outputs.

**Parameters**
   Controllable values on processors that can be set statically or automated over time.

**Automation**
   Time-varying parameter control, specified at either audio-rate (per-sample) or PPQN-rate (pulses-per-quarter-note).

Processor Types
---------------

DawDreamer supports several processor types:

**Faust Processor**
   Real-time Faust DSP code compilation and execution. Perfect for custom effects and instruments.

   See :doc:`faust_processor` for details.

**Plugin Processor**
   Hosts VST 2/3 and Audio Unit plugins (instruments and effects).

   See :doc:`plugin_processor` for details.

**Playback Processor**
   Plays back audio from memory with optional looping.

**Playback Warp Processor**
   Time-stretching, pitch-shifting, and warp marker support (Ableton Live compatible).

   See :doc:`playback_warp` for details.

**Effect Processors**
   Built-in effects including filter, compressor, reverb, delay, and panner.

   See :doc:`other_processors` for details.

**Utility Processors**
   Add (mixer), sampler, and oscillator processors for routing and testing.

Basic Workflow
--------------

1. **Create a RenderEngine**

   .. code-block:: python

      import dawdreamer as daw
      engine = daw.RenderEngine(44100, 512)

2. **Create Processors**

   .. code-block:: python

      synth = engine.make_faust_processor("synth")
      effect = engine.make_filter_processor("filter", "low")

3. **Configure Processors**

   .. code-block:: python

      synth.set_dsp_string("process = os.osc(440) * 0.1;")
      effect.set_frequency(1000.0)

4. **Load the Graph**

   .. code-block:: python

      engine.load_graph([
          (synth, []),
          (effect, ["synth"])
      ])

5. **Render**

   .. code-block:: python

      engine.render(4.0)  # 4 seconds
      audio = engine.get_audio()

6. **Modify and Re-render**

   .. code-block:: python

      synth.set_dsp_string("process = os.osc(880) * 0.1;")
      engine.render(2.0)
      # ...and so on

Common Patterns
---------------

Multi-Channel Routing
~~~~~~~~~~~~~~~~~~~~~

Processors automatically handle channel counts. If a processor expects 2 channels but receives 4, you can use mix-down or routing:

.. code-block:: python

   # Mix 4 channels to 2
   mixer = engine.make_add_processor("mixer")
   mixer.set_gain(0.5)  # Avoid clipping

Parallel Processing
~~~~~~~~~~~~~~~~~~~

Process the same input with multiple effects:

.. code-block:: python

   input_proc = engine.make_playback_processor("input", audio)
   delay = engine.make_delay_processor("delay")
   reverb = engine.make_reverb_processor("reverb")
   mixer = engine.make_add_processor("mix")

   engine.load_graph([
       (input_proc, []),
       (delay, ["input"]),
       (reverb, ["input"]),
       (mixer, ["delay", "reverb"])
   ])

.. _parameter-automation:

Parameter Automation
~~~~~~~~~~~~~~~~~~~~

All processors support parameter automation - changing parameter values over time. There are two automation modes:

**Audio-Rate Automation**

One automation value per audio sample for sample-accurate control:

.. code-block:: python

   import numpy as np

   def make_sine(freq, duration, sr=44100):
       """Generate sine wave for automation"""
       N = int(duration * sr)
       return np.sin(np.pi * 2. * freq * np.arange(N) / sr)

   duration = 10.0  # seconds
   sample_rate = 44100
   num_samples = int(duration * sample_rate)

   # Sweep frequency from 100 Hz to 5000 Hz
   automation = np.linspace(100, 5000, num_samples)
   filter_proc.set_automation("frequency", automation)

   # Or oscillating parameter between 0 and 1 at 0.5 Hz
   automation = 0.5 + 0.5 * make_sine(0.5, duration)
   synth.set_automation(1, automation)  # Plugin parameters use index

**PPQN-Rate Automation**

Automation aligned with musical tempo (pulses per quarter note):

.. code-block:: python

   PPQN = 960
   num_beats = 20

   # Parameter alternates between 0.25 and 0.75 four times per beat
   automation = make_sine(4, num_beats, sr=PPQN)
   automation = 0.25 + 0.5 * (automation > 0).astype(np.float32)
   synth.set_automation(1, automation, ppqn=PPQN)

.. note::
   PPQN automation adapts to tempo changes automatically. Use ``engine.set_bpm()`` to control tempo.

**Recording Automation**

You can record automation data after rendering:

.. code-block:: python

   # Enable automation recording
   processor.record_automation = True

   # Render
   engine.render(10.0)

   # Retrieve audio-rate automation for all parameters
   all_automation = processor.get_automation()  # Returns dict of parameter arrays

.. tip::
   Use smaller buffer sizes (128-256) for finer granularity in recorded automation. Larger buffer sizes may result in "steppiness".

Recording Intermediate Outputs
------------------------------

Any processor can record its output for later retrieval:

.. code-block:: python

   # Enable recording on any processor
   processor.record = True

   # Render
   engine.render(5.0)

   # Retrieve recorded audio
   recorded_audio = processor.get_audio()

   # Or by processor name
   recorded_audio = engine.get_audio("processor_name")

This is useful for:

* Debugging signal flow
* Capturing intermediate processing stages
* Layering different processing chains
* Exporting stems from a mix

Complete Example: Multi-Track Mix
----------------------------------

Here's a complete example combining multiple processors:

.. code-block:: python

   import dawdreamer as daw
   import librosa
   from scipy.io import wavfile

   SAMPLE_RATE = 44100
   BUFFER_SIZE = 512

   engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

   # Load audio files
   vocals_audio, _ = librosa.load("vocals.wav", sr=SAMPLE_RATE, mono=False)
   piano_audio, _ = librosa.load("piano.wav", sr=SAMPLE_RATE, mono=False)
   guitar_audio, _ = librosa.load("guitar.wav", sr=SAMPLE_RATE, mono=False)

   # Create playback processors
   vocals = engine.make_playback_processor("vocals", vocals_audio)
   piano = engine.make_playback_processor("piano", piano_audio)
   guitar = engine.make_playback_processor("guitar", guitar_audio)

   # Add effect to guitar
   filter_proc = engine.make_filter_processor("filter", "low", 5000., 0.707, 1.0)
   filter_proc.record = True  # Record filtered guitar

   # Create mixer
   mixer = engine.make_add_processor("mixer", [0.4, 0.4, 0.4])

   # Add master effects
   comp = engine.make_compressor_processor("comp", -12.0, 3.0, 5.0, 50.0)
   reverb = engine.make_reverb_processor("reverb", 0.5, 0.5, 0.2, 0.8, 1.0)

   # Build processing graph
   graph = [
       (vocals, []),
       (piano, []),
       (guitar, []),
       (filter_proc, ["guitar"]),
       (mixer, ["vocals", "piano", "filter"]),
       (comp, ["mixer"]),
       (reverb, ["comp"])
   ]

   engine.load_graph(graph)
   engine.render(10.0)

   # Get outputs
   final_mix = engine.get_audio()
   filtered_guitar = filter_proc.get_audio()

   # Save files
   wavfile.write('final_mix.wav', SAMPLE_RATE, final_mix.transpose())
   wavfile.write('filtered_guitar_stem.wav', SAMPLE_RATE, filtered_guitar.transpose())

Best Practices
--------------

**Gain Staging**
   * Use Add processor to control relative levels in multi-track mixes
   * Watch for clipping (peaks > 1.0 or < -1.0)
   * Apply compression on the master bus for cohesive dynamics
   * Leave headroom (e.g., -6dB) before final mastering

**Processor Ordering**
   * EQ/Filter → Dynamics (compressor) → Time-based effects (reverb, delay)
   * This is a common mixing practice but not a strict rule
   * Experiment with different orderings for creative effects

**Automation**
   * Use smaller buffer sizes (128-256) for smoother automation curves
   * PPQN automation adapts to tempo changes automatically
   * Audio-rate automation is sample-accurate but uses more memory

**Recording Outputs**
   * Enable ``record = True`` only on processors you need to capture
   * Minimal performance impact
   * Useful for debugging signal flow or exporting stems

Troubleshooting
---------------

**Graph validation errors**
   * Ensure all processor names are unique
   * Check that input processor names exist
   * Verify the graph is acyclic (no loops)

**Channel mismatch**
   * Check processor input/output channel counts with ``get_num_input_channels()`` and ``get_num_output_channels()``
   * Use Add processor to mix channels if needed

**Parameter not found**
   * Use ``get_parameters_description()`` to list all parameters
   * Check parameter address format (e.g., ``"/MySine/freq"``)

**Audio clipping**
   * Reduce gain levels on Add processor
   * Check individual processor output levels
   * Apply compression to control peaks
   * Leave headroom in your mix

**No audio output**
   * Verify graph connections are correct
   * Check that at least one processor generates audio
   * Ensure render duration is > 0
   * Check processor parameters (gain, volume, etc.)

Next Steps
----------

* Explore :doc:`render_engine` for tempo and BPM management
* Learn about :doc:`faust_processor` for custom DSP
* Discover :doc:`plugin_processor` for VST integration
* See :doc:`../examples` for real-world use cases

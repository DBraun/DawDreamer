Render Engine and BPM
=====================

The ``RenderEngine`` is the central orchestrator in DawDreamer. It manages the audio processing pipeline, tempo, timing, and graph execution.

Creating a RenderEngine
-----------------------

Create a render engine by specifying the sample rate and block size:

.. code-block:: python

   import dawdreamer as daw

   SAMPLE_RATE = 44100
   BUFFER_SIZE = 128  # Parameters will undergo automation at this block size

   # Make an engine. We typically only need one.
   engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

**Parameters:**

* ``SAMPLE_RATE``: Audio sample rate in Hz (e.g., 44100, 48000)
* ``BUFFER_SIZE``: Processing block size in samples (e.g., 128, 256, 512)

The block size determines the granularity of parameter automation. Smaller block sizes provide finer control but may increase CPU usage.

Setting BPM
-----------

Static BPM
~~~~~~~~~~

By default, the BPM (beats per minute) is 120, but you can change it:

.. code-block:: python

   # Set a fixed BPM
   engine.set_bpm(130.)

This static BPM value is used when rendering time in beats:

.. code-block:: python

   engine.render(4., beats=True)  # Render 4 beats at 130 BPM

Dynamic BPM Automation
~~~~~~~~~~~~~~~~~~~~~~

The BPM can also be set as a numpy array that is interpreted with a fixed PPQN (Pulses Per Quarter Note). This allows for tempo changes during rendering.

.. code-block:: python

   import numpy as np

   def make_sine(freq: float, duration: float, sr=SAMPLE_RATE):
       """Return sine wave based on freq in Hz and duration in seconds"""
       N = int(duration * sr)  # Number of samples
       return np.sin(np.pi * 2. * freq * np.arange(N) / sr)

   PPQN = 960  # Pulses per quarter note
   DURATION = 10.  # seconds

   # Create BPM automation that alternates between 120 and 150 BPM
   bpm_automation = make_sine(1./2., DURATION, sr=PPQN)
   bpm_automation = 120. + 30 * (bpm_automation > 0).astype(np.float32)

   engine.set_bpm(bpm_automation, ppqn=PPQN)

**How it works:**

* If you choose ``ppqn=960`` and the numpy array abruptly changes values every 960 samples, the tempo will abruptly change "on the beat"
* Higher PPQN values provide finer tempo resolution
* Common PPQN values: 24, 96, 480, 960

Rendering Audio
---------------

Render by Duration
~~~~~~~~~~~~~~~~~~

Render a specific duration in seconds:

.. code-block:: python

   engine.render(4.0)  # Render 4 seconds
   audio = engine.get_audio()

Render by Beats
~~~~~~~~~~~~~~~

Render a specific number of beats at the current BPM:

.. code-block:: python

   engine.set_bpm(120.)
   engine.render(8., beats=True)  # Render 8 beats (4 seconds at 120 BPM)
   audio = engine.get_audio()

Getting Audio Output
--------------------

After rendering, retrieve the audio as a NumPy array:

.. code-block:: python

   audio = engine.get_audio()
   # Shape: (num_channels, num_samples)
   # dtype: float32

Save the audio to a file:

.. code-block:: python

   from scipy.io import wavfile
   wavfile.write('output.wav', SAMPLE_RATE, audio.transpose())

Graph Management
----------------

Loading a Graph
~~~~~~~~~~~~~~~

Load a processor graph before rendering:

.. code-block:: python

   graph = [
       (processor_a, []),
       (processor_b, ["processor_a"])
   ]
   engine.load_graph(graph)

See :doc:`index` for more details on graph construction.

Re-rendering
~~~~~~~~~~~~

You can modify processor parameters and re-render without reloading the graph:

.. code-block:: python

   engine.render(4.)
   audio1 = engine.get_audio()

   # Modify parameters
   processor.set_parameter("gain", 0.5)

   # Re-render with new parameters
   engine.render(4.)
   audio2 = engine.get_audio()

Timing and Synchronization
---------------------------

Audio-Rate Automation
~~~~~~~~~~~~~~~~~~~~~

Automation data is interpreted at audio rate (one value per sample):

.. code-block:: python

   duration = 4.0
   num_samples = int(duration * SAMPLE_RATE)
   automation = np.linspace(100, 1000, num_samples)
   processor.set_automation("frequency", automation)
   engine.render(duration)

PPQN-Rate Automation
~~~~~~~~~~~~~~~~~~~~

Automation data is interpreted at PPQN rate (pulses per quarter note):

.. code-block:: python

   ppqn = 960
   beats = 4
   num_pulses = beats * ppqn
   automation = np.linspace(100, 1000, num_pulses)
   processor.set_automation("frequency", automation, ppqn=ppqn)
   engine.set_bpm(120.)
   engine.render(beats, beats=True)

PPQN-rate automation is ideal for musical timing because it automatically adapts to tempo changes.

Best Practices
--------------

**Choose appropriate block size**
   * Smaller (128-256): Lower latency, finer automation, higher CPU usage
   * Larger (512-1024): Higher latency, coarser automation, lower CPU usage
   * 512 is a good default for offline rendering

**BPM and beats**
   * Use beats=True when working with musical timing
   * Use beats=False (seconds) for sound design or fixed-duration rendering

**Sample rate**
   * 44100 Hz: CD quality, good for most applications
   * 48000 Hz: Professional video/film standard
   * 96000 Hz: High-resolution audio (higher CPU usage)

**Re-rendering**
   * The engine reuses internal buffers for efficiency
   * You can render multiple times without recreating the engine
   * Useful for batch processing or iterative parameter exploration

Performance Considerations
--------------------------

* **Graph complexity**: More processors = more CPU usage
* **Block size**: Affects real-time performance and automation granularity
* **Processor types**: Some processors (Faust, VST plugins) are more CPU-intensive than others
* **Automation**: Audio-rate automation is more expensive than static parameters

Example: Complete Workflow
---------------------------

.. code-block:: python

   import dawdreamer as daw
   import numpy as np
   from scipy.io import wavfile

   # Setup
   SAMPLE_RATE = 44100
   BUFFER_SIZE = 512
   engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

   # Create processors
   synth = engine.make_faust_processor("synth")
   synth.set_dsp_string("process = os.osc(440) * 0.1;")

   # Load graph
   engine.load_graph([(synth, [])])

   # Render with static BPM
   engine.set_bpm(120.)
   engine.render(4., beats=True)
   audio1 = engine.get_audio()

   # Render with dynamic BPM
   ppqn = 960
   bpm_ramp = np.linspace(120, 180, 4 * ppqn)
   engine.set_bpm(bpm_ramp, ppqn=ppqn)
   engine.render(4., beats=True)
   audio2 = engine.get_audio()

   # Save outputs
   wavfile.write('static_bpm.wav', SAMPLE_RATE, audio1.transpose())
   wavfile.write('dynamic_bpm.wav', SAMPLE_RATE, audio2.transpose())

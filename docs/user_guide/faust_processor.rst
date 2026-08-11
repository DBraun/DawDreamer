Faust Processor
===============

The ``FaustProcessor`` enables real-time compilation and execution of Faust DSP code. Faust is a functional programming language for sound synthesis and audio processing.

New to Faust?
-------------

**Resources:**

* `Documentation and Resources <https://github.com/grame-cncm/faust#documentation-and-resources>`_
* `Online IDE <https://faustide.grame.fr/>`_ - Develop and test code
* `Libraries <https://faustlibraries.grame.fr/>`_ - Browse the standard library
* `Syntax Manual <https://faustdoc.grame.fr/manual/syntax/>`_
* `Communities <https://faust.grame.fr/community/help/>`_

Basic Usage
-----------

Creating a Faust Processor
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import dawdreamer as daw

   engine = daw.RenderEngine(44100, 512)
   faust_processor = engine.make_faust_processor("faust")

Loading DSP from File
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   DSP_PATH = "/absolute/path/to/faust_reverb.dsp"
   faust_processor.set_dsp(DSP_PATH)  # Must be absolute path

   # Compile early to catch errors
   faust_processor.compile()  # Raises RuntimeError for invalid Faust code

**faust_reverb.dsp:**

.. code-block:: text

   process = dm.zita_light;

.. note::
   The standard library (``stdfaust.lib``) is always imported automatically. You don't need to ``import("stdfaust.lib");`` in your code.

Loading DSP from String
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   faust_processor.set_dsp_string(
       """
       declare name "MySine";
       freq = hslider("freq", 440, 0, 20000, 0);
       gain = hslider("vol[unit:dB]", 0, -120, 20, 0) : ba.db2linear;
       process = freq : os.osc : _*gain <: si.bus(2);
       """
   )

Parameters
----------

Listing Parameters
~~~~~~~~~~~~~~~~~~

Get a description of all available parameters:

.. code-block:: python

   print(faust_processor.get_parameters_description())

Setting Parameters
~~~~~~~~~~~~~~~~~~

Parameters can be set by address (string) or by index (integer):

.. code-block:: python

   # By address
   faust_processor.set_parameter("/Zita_Light/Dry/Wet_Mix", 1.0)

   # By index
   faust_processor.set_parameter(0, 1.0)

.. note::
   Unlike VST plugins, Faust parameters aren't necessarily normalized to 0-1. For example, a filter cutoff might accept values from 20 to 20000 Hz naturally:

   .. code-block:: python

      faust_processor.set_parameter("/MyFilter/cutoff", 15000)

Getting Parameter Values
~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   val = faust_processor.get_parameter("/Zita_Light/Dry/Wet_Mix")
   print('Value:', val)

Example: Stereo Reverb
----------------------

.. code-block:: python

   import dawdreamer as daw
   from scipy.io import wavfile
   import librosa

   DSP_PATH = "/absolute/path/to/faust_reverb.dsp"
   INPUT_AUDIO_PATH = "/path/to/piano.wav"
   DURATION = 10.
   SAMPLE_RATE = 44100

   engine = daw.RenderEngine(SAMPLE_RATE, 512)
   faust_processor = engine.make_faust_processor("faust")
   faust_processor.set_dsp(DSP_PATH)

   # Compile early to catch errors
   faust_processor.compile()

   print(faust_processor.get_parameters_description())

   # Set parameters
   faust_processor.set_parameter("/Zita_Light/Dry/Wet_Mix", 1.)

   # Load input audio
   audio, _ = librosa.load(INPUT_AUDIO_PATH, sr=SAMPLE_RATE, mono=False)
   playback = engine.make_playback_processor("piano", audio)

   # Build graph
   graph = [
       (playback, []),
       (faust_processor, ["piano"])
   ]

   engine.load_graph(graph)
   engine.render(DURATION)

   output_audio = engine.get_audio()
   wavfile.write('reverb_output.wav', SAMPLE_RATE, output_audio.transpose())

Multi-Channel Processing
------------------------

Here's an example that mixes two stereo inputs into one stereo output with a low-pass filter:

**dsp_4_channels.dsp:**

.. code-block:: text

   declare name "MyEffect";
   import("stdfaust.lib");
   myFilter = fi.lowpass(10, hslider("cutoff", 15000., 20, 20000, .01));
   process = si.bus(4) :> sp.stereoize(myFilter);

**Python code:**

.. code-block:: python

   import dawdreamer as daw
   from scipy.io import wavfile
   import librosa
   import numpy as np

   INPUT_AUDIO_PATH1 = "piano.wav"
   INPUT_AUDIO_PATH2 = "vocals.wav"
   DURATION = 10.
   SAMPLE_RATE = 44100

   def make_sine(freq, duration, sr=SAMPLE_RATE):
       N = int(duration * sr)
       return np.sin(np.pi * 2. * freq * np.arange(N) / sr)

   engine = daw.RenderEngine(SAMPLE_RATE, 512)
   faust_processor = engine.make_faust_processor("faust")
   faust_processor.set_dsp("/absolute/path/to/dsp_4_channels.dsp")

   print(faust_processor.get_parameters_description())

   # Set static parameter
   faust_processor.set_parameter("/MyEffect/cutoff", 7000.0)

   # Or use automation
   cutoff_automation = 15000 + 5000 * make_sine(2, DURATION)
   faust_processor.set_automation("/MyEffect/cutoff", cutoff_automation)

   # Load audio
   audio1, _ = librosa.load(INPUT_AUDIO_PATH1, sr=SAMPLE_RATE, mono=False)
   audio2, _ = librosa.load(INPUT_AUDIO_PATH2, sr=SAMPLE_RATE, mono=False)

   playback1 = engine.make_playback_processor("piano", audio1)
   playback2 = engine.make_playback_processor("vocals", audio2)

   graph = [
       (playback1, []),
       (playback2, []),
       (faust_processor, ["piano", "vocals"])  # 4-channel input
   ]

   engine.load_graph(graph)
   engine.render(DURATION)

   output = engine.get_audio()
   wavfile.write('mixed_output.wav', SAMPLE_RATE, output.transpose())

Polyphony
---------

Faust processors support polyphonic synthesis. You must provide DSP code that refers to standard parameters:

* ``freq`` or ``note``: MIDI note frequency
* ``gain``: Note velocity/amplitude
* ``gate``: Note on/off trigger

For more information, see the `Faust MIDI manual <https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters>`_.

Enabling Polyphony
~~~~~~~~~~~~~~~~~~

Set the number of voices to 1 or higher (default is 0, which disables polyphony):

.. code-block:: python

   faust_processor.num_voices = 8  # 8-voice polyphony

Tuning Polyphony
~~~~~~~~~~~~~~~~

Three more properties control polyphonic behavior:

.. code-block:: python

   # If grouped, all voices share the same parameter values.
   faust_processor.group_voices = True

   # If enabled (default), voices are dynamically enabled and disabled
   # to save computation.
   faust_processor.dynamic_voices = True

   # Specifying the release length (seconds) accurately helps avoid
   # warnings about voices being stolen.
   faust_processor.release_length = 0.5

Example
~~~~~~~

**poly_synth.dsp:**

.. code-block:: text

   import("stdfaust.lib");

   freq = nentry("freq", 440, 20, 20000, 1);
   gain = nentry("gain", 0.5, 0, 1, 0.01);
   gate = button("gate");

   envelope = gain * en.adsr(0.01, 0.1, 0.8, 0.3, gate);
   process = os.sawtooth(freq) * envelope <: _, _;

**Python code:**

.. code-block:: python

   faust_processor.set_dsp("/path/to/poly_synth.dsp")
   faust_processor.num_voices = 8

   # Add MIDI notes (note, velocity, start_time, duration)
   faust_processor.add_midi_note(60, 100, 0.0, 1.0)  # C4
   faust_processor.add_midi_note(64, 80, 0.5, 1.0)   # E4
   faust_processor.add_midi_note(67, 90, 1.0, 1.0)   # G4

   engine.load_graph([(faust_processor, [])])
   engine.render(3.0)

See `tests/test_faust_poly*.py <https://github.com/DBraun/DawDreamer/tree/main/tests>`_ for more examples.

Compilation Options
-------------------

Several properties configure how DSP code is compiled. Set them before calling ``compile()``:

.. code-block:: python

   # Prepended to your DSP code. Default: 'import("stdfaust.lib");'
   faust_processor.auto_import = 'import("stdfaust.lib");'

   # Extra Faust compiler flags.
   faust_processor.compile_flags = ["-vec"]

   # LLVM IR optimization level, -1 (maximum) through 4.
   faust_processor.opt_level = -1

   # Extra search paths for your custom .lib files.
   faust_processor.faust_libraries_paths = ["/path/to/my/libs"]

   # Search paths for audio used by the soundfile primitive.
   faust_processor.faust_assets_paths = ["/path/to/audio"]

After compiling, two read-only properties are useful:

.. code-block:: python

   faust_processor.compiled  # True if the most recent compile succeeded
   faust_processor.code      # the most recently compiled DSP code

Compiling from the Box and Signal APIs
--------------------------------------

Instead of DSP source code, a processor can compile a ``Box`` or list of ``Signal`` objects built programmatically with the :doc:`Box and Signal APIs <../api_reference/index>`:

.. code-block:: python

   from dawdreamer.faust import FaustContext
   from dawdreamer.faust.box import boxMul, boxPar, boxReal, boxWire

   faust_processor = engine.make_faust_processor("faust")

   with FaustContext():
       box_gain_half = boxMul(boxWire(), boxReal(0.5))
       box = boxPar(box_gain_half, box_gain_half)
       faust_processor.compile_box(box)

``compile_signals`` works the same way for signals. Both accept an optional list of compiler arguments.

A non-polyphonic processor can also be restored from LLVM bitcode without recompiling from source. The bitcode is available in the processor's pickle state, and pickling uses this internally (see :doc:`pickling`):

.. code-block:: python

   state = faust_processor.__getstate__()
   other_processor.compile_from_bitcode(state["bitcode"])

Soundfiles
----------

Faust code in DawDreamer can use the `soundfile <https://faustdoc.grame.fr/manual/syntax/#soundfile-primitive>`_ primitive. Normally, ``soundfile`` loads ``.wav`` files, but DawDreamer uses it to receive data from NumPy arrays.

This is useful for sample-based instruments or convolution reverbs.

Example
~~~~~~~

**Python code:**

.. code-block:: python

   # Suppose audio1, audio2, and audio3 are np.array shaped (channels, samples)
   soundfiles = {
       'mySound': [audio1, audio2, audio3]
   }
   faust_processor.set_soundfiles(soundfiles)

**soundfile_test.dsp:**

.. code-block:: text

   soundChoice = nentry("soundChoice", 0, 0, 2, 1);  // choose between 0, 1, 2
   process = soundChoice, _ ~ +(1) : soundfile("mySound", 2) : !, !, _, _;

.. note::
   The second argument to ``soundfile("mySound", 2)`` is a hint that the audio is stereo. It's unrelated to the Python side where ``mySound``'s dictionary value has 3 NumPy arrays.

Advanced Example: 88-Key Piano
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

See `tests/test_faust_soundfile.py <https://github.com/DBraun/DawDreamer/blob/main/tests/test_faust_soundfile.py>`_ for a complete example of loading 88 piano samples and playing them with polyphony.

Parameter Automation
--------------------

Faust processors support both audio-rate and PPQN-rate parameter automation using parameter addresses.

.. seealso::
   See :ref:`Parameter Automation <parameter-automation>` in the User Guide for complete documentation on automation modes and recording.

**Quick example:**

.. code-block:: python

   import numpy as np

   # Audio-rate automation using parameter address
   duration = 4.0
   sample_rate = 44100
   num_samples = int(duration * sample_rate)

   # Sweep frequency from 200 Hz to 2000 Hz
   frequency_sweep = np.linspace(200, 2000, num_samples)
   faust_processor.set_automation("/MySynth/freq", frequency_sweep)

   # PPQN-rate automation (musically-synced)
   ppqn = 960
   beats = 4
   num_pulses = beats * ppqn
   freq_automation = np.linspace(200, 2000, num_pulses)
   faust_processor.set_automation("/MySynth/freq", freq_automation, ppqn=ppqn)

   engine.set_bpm(120.)
   engine.render(beats, beats=True)

Tips and Best Practices
------------------------

**Development Workflow**
   1. Develop and test Faust code in the `Online IDE <https://faustide.grame.fr/>`_
   2. Save to a ``.dsp`` file or copy to a Python string
   3. Load into DawDreamer and test

**Debugging**
   * Use ``compile()`` early to catch syntax errors
   * Check ``get_parameters_description()`` to verify parameter names
   * Test with simple DSP first (e.g., ``process = _;`` for passthrough)

**Performance**
   * Faust compiles to optimized C++ at runtime
   * More complex DSP = longer compile time and higher CPU usage
   * Consider caching compiled processors for batch operations

**Parameter Naming**
   * Use descriptive names with units: ``hslider("freq[unit:Hz]", ...)``
   * Group parameters: ``hgroup("Oscillator", ...)``
   * Makes automation and UI more intuitive

**Channel Handling**
   * Use ``si.bus(N)`` to handle N channels
   * Use ``<:`` to split signals, ``:>`` to mix them
   * Example: ``process = _ <: _, _;`` duplicates mono to stereo

Common Issues
-------------

**"Parameter not found"**
   * Check parameter address with ``get_parameters_description()``
   * Verify spelling and path (e.g., ``/MyEffect/cutoff`` vs ``/cutoff``)

**"Compilation failed"**
   * Test code in Faust Online IDE first
   * Check for syntax errors or missing libraries
   * Ensure absolute path for ``.dsp`` files

**Audio is silent**
   * Check parameter values (especially gain/volume)
   * Verify DSP logic (test with simple sine wave first)
   * Check for ``gate`` parameter in polyphonic instruments

**Clicking or artifacts**
   * Decrease buffer size for higher fidelity parameter automation
   * Check for parameter discontinuities
   * Use smoothing in Faust code: ``si.smoo``

Further Reading
---------------

* :doc:`render_engine` - BPM and timing
* :doc:`index` - Graph construction
* :doc:`../examples` - Real-world examples
* `Faust Box API Example <https://github.com/DBraun/DawDreamer/tree/main/examples/Box_API>`_
* `Faust to JAX Transpilation <https://github.com/DBraun/DawDreamer/tree/main/examples/Faust_to_JAX>`_

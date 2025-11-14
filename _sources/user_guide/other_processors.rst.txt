Built-In Effect Processors
===========================

DawDreamer includes several built-in processors for common audio tasks: mixing, filtering, dynamics, spatial effects, and delays.

.. seealso::
   * :doc:`playback` - Basic audio playback
   * :doc:`playback_warp` - Time-stretching and pitch-shifting

Add Processor (Mixer)
---------------------

The ``AddProcessor`` sums (mixes) multiple audio inputs with optional gain control.

Basic Usage
~~~~~~~~~~~

.. code-block:: python

   # Create add processor
   add_proc = engine.make_add_processor("mixer")

   # Or with initial gain levels
   add_proc = engine.make_add_processor("mixer", [0.25, 0.42, 0.30])

Setting Gain Levels
~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   # Adjust gain for each input
   add_proc.gain_levels = [0.5, 0.5, 0.5]  # 50% gain for 3 inputs

   # Or set a single gain for all inputs
   add_proc.set_gain(0.5)

Complete Example
~~~~~~~~~~~~~~~~

.. code-block:: python

   import dawdreamer as daw
   import librosa

   engine = daw.RenderEngine(44100, 512)

   # Load audio files
   vocals = engine.make_playback_processor("vocals", librosa.load("vocals.wav", sr=44100, mono=False)[0])
   piano = engine.make_playback_processor("piano", librosa.load("piano.wav", sr=44100, mono=False)[0])
   guitar = engine.make_playback_processor("guitar", librosa.load("guitar.wav", sr=44100, mono=False)[0])

   # Create mixer
   mixer = engine.make_add_processor("mixer", [0.4, 0.4, 0.4])

   # Build graph
   graph = [
       (vocals, []),
       (piano, []),
       (guitar, []),
       (mixer, ["vocals", "piano", "guitar"])
   ]

   engine.load_graph(graph)
   engine.render(10.0)
   mixed = engine.get_audio()

Filter Processor
----------------

The ``FilterProcessor`` provides basic IIR filtering from JUCE.

Filter Modes
~~~~~~~~~~~~

* ``"low"``: Low-pass filter
* ``"high"``: High-pass filter
* ``"band"``: Band-pass filter
* ``"low_shelf"``: Low shelf filter
* ``"high_shelf"``: High shelf filter
* ``"notch"``: Notch (band-reject) filter

Basic Usage
~~~~~~~~~~~

.. code-block:: python

   filter_mode = "high"
   freq = 1000.0     # Cutoff frequency in Hz
   q = 0.707107      # Q factor (1/√2 is a safe choice)
   gain = 1.0        # Gain (only for shelf modes)

   filter_proc = engine.make_filter_processor("my_filter", filter_mode, freq, q, gain)

Setting Parameters
~~~~~~~~~~~~~~~~~~

.. code-block:: python

   # Adjust parameters
   filter_proc.mode = "low"
   filter_proc.freq = 500.0
   filter_proc.q = 2.0
   filter_proc.gain = 1.5  # Only affects shelf modes

Parameter Automation
~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import numpy as np

   def make_sine(freq, duration, sr=44100):
       N = int(duration * sr)
       return np.sin(np.pi * 2. * freq * np.arange(N) / sr)

   # Sweep filter from 2 kHz to 12 kHz
   duration = 10.0
   freq_automation = make_sine(0.5, duration) * 5000. + 7000.
   filter_proc.set_automation("freq", freq_automation)

Compressor Processor
--------------------

The ``CompressorProcessor`` provides dynamic range compression.

Parameters
~~~~~~~~~~

* ``threshold``: dB level where compression starts
* ``ratio``: Compression ratio (≥ 1.0)
* ``attack``: Attack time in milliseconds
* ``release``: Release time in milliseconds

Basic Usage
~~~~~~~~~~~

.. code-block:: python

   threshold = -10.0   # Compress signals above -10 dB
   ratio = 4.0         # 4:1 compression ratio
   attack = 2.0        # 2 ms attack
   release = 50.0      # 50 ms release

   comp = engine.make_compressor_processor("my_comp", threshold, ratio, attack, release)

Adjusting Parameters
~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   comp.threshold = -15.0
   comp.ratio = 3.0
   comp.attack = 5.0
   comp.release = 100.0

.. tip::
   * Lower threshold = more compression
   * Higher ratio = more aggressive compression
   * Faster attack = tighter transient control
   * Faster release = more pumping effect

Reverb Processor
----------------

The ``ReverbProcessor`` uses JUCE's built-in reverb algorithm.

Parameters
~~~~~~~~~~

* ``room_size``: Room size (0.0 - 1.0)
* ``damping``: High-frequency damping (0.0 - 1.0)
* ``wet_level``: Wet signal level (0.0 - 1.0)
* ``dry_level``: Dry signal level (0.0 - 1.0)
* ``width``: Stereo width (0.0 - 1.0)

Basic Usage
~~~~~~~~~~~

.. code-block:: python

   room_size = 0.5
   damping = 0.5
   wet_level = 0.33
   dry_level = 0.4
   width = 1.0

   reverb = engine.make_reverb_processor("my_reverb", room_size, damping, wet_level, dry_level, width)

Adjusting Parameters
~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   reverb.room_size = 0.8   # Larger room
   reverb.damping = 0.7     # More damping
   reverb.wet_level = 0.5   # More reverb
   reverb.dry_level = 0.3   # Less dry signal
   reverb.width = 0.9       # Slightly narrower stereo

Panner Processor
----------------

The ``PannerProcessor`` provides stereo panning.

Panning Rules
~~~~~~~~~~~~~

* ``"linear"``: Linear panning
* ``"balanced"``: Balance law panning
* ``"sin3dB"``: Sine 3dB panning
* ``"sin4p5dB"``: Sine 4.5dB panning
* ``"sin6dB"``: Sine 6dB panning
* ``"squareRoot3dB"``: Square root 3dB panning
* ``"squareRoot4p5dB"``: Square root 4.5dB panning

Basic Usage
~~~~~~~~~~~

.. code-block:: python

   panner = engine.make_panner_processor("my_panner", "linear", 0.0)

   # Pan value: -1.0 (full left) to 1.0 (full right)
   panner.pan = -0.5  # Pan left

Adjusting Parameters
~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   panner.rule = "balanced"
   panner.pan = 0.75  # Pan right

.. note::
   Different panning rules affect the perceived loudness and stereo image. ``"balanced"`` is a good default for music.

Delay Processor
---------------

The ``DelayProcessor`` provides a simple delay effect.

Parameters
~~~~~~~~~~

* ``rule``: Delay rule (currently only ``"linear"`` is supported)
* ``delay``: Delay time in milliseconds
* ``wet``: Wet/dry mix (0.0 = all dry, 1.0 = all wet)

Basic Usage
~~~~~~~~~~~

.. code-block:: python

   delay_rule = "linear"
   delay_ms = 200.0
   delay_wet = 0.3

   delay = engine.make_delay_processor("my_delay", delay_rule, delay_ms, delay_wet)

Adjusting Parameters
~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   delay.delay = 500.0  # 500 ms delay
   delay.wet = 0.5      # 50% wet signal

.. warning::
   Modifying ``delay.rule`` is not currently supported. It must be set at creation time.

Processor-Specific Tips
------------------------

**Filter Processor**
   * Automate cutoff frequency for filter sweeps
   * Use higher Q values for more resonant filters
   * Low-pass filters at 0.707 Q sound natural
   * Reduce Q factor if filter sounds too harsh
   * Use gentler filter modes (shelf instead of band) for subtlety

**Compressor Processor**
   * Start with moderate ratios (2:1 to 4:1)
   * Use faster attack for limiting transients
   * Use slower attack to preserve punch
   * Adjust threshold to taste (typically -20 dB to -6 dB)
   * Check threshold is below signal level if compressor seems inactive
   * Verify ratio is > 1.0

**Reverb Processor**
   * Balance wet/dry for natural space
   * Increase damping for warmer reverb
   * Reduce width for mono-compatible mixes
   * Reduce wet level or increase damping if reverb sounds muddy
   * Consider high-passing the reverb input

**Delay Processor**
   * Current delay processor doesn't support feedback
   * Use Faust processor for feedback delays
   * Or chain multiple delay processors for echo effects

**Add Processor (Mixer)**
   * Use to control relative levels in multi-track mixes
   * Watch for clipping (peaks > 1.0 or < -1.0)
   * Reduce gain levels if output is clipping
   * Apply compression on the master bus for cohesive dynamics

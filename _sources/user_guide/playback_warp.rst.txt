Playback Warp Processor
=======================

The ``PlaybackWarpProcessor`` provides time-stretching and pitch-shifting using the Rubber Band Library. It also supports Ableton Live warp markers for beat-matching.

.. seealso::
   For warp marker manipulation, see the companion project `AbletonParsing <https://github.com/DBraun/AbletonParsing>`_.

Basic Usage
-----------

Time-Stretching and Pitch-Shifting
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import dawdreamer as daw
   import librosa

   SAMPLE_RATE = 44100
   BUFFER_SIZE = 512

   engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

   # Load audio
   audio, _ = librosa.load("drums.wav", sr=SAMPLE_RATE, mono=False)

   # Create playback warp processor
   playback = engine.make_playbackwarp_processor("drums", audio)

   # Time-stretch: play in twice the amount of time (slowed down)
   playback.time_ratio = 2.0

   # Pitch-shift: down 5 semitones
   playback.transpose = -5.0

   # Load graph and render
   engine.load_graph([(playback, [])])
   engine.render(10.0)
   audio_out = engine.get_audio()

**Parameters:**

* ``time_ratio``: Time-stretching factor. Values > 1 slow down, < 1 speed up.
* ``transpose``: Pitch shift in semitones. Positive = up, negative = down.

Advanced Options
~~~~~~~~~~~~~~~~

The Rubber Band Library provides many quality and behavior options:

.. code-block:: python

   # Set advanced Rubber Band options
   playback.set_options(
       daw.PlaybackWarpProcessor.option.OptionTransientsSmooth |
       daw.PlaybackWarpProcessor.option.OptionPitchHighQuality |
       daw.PlaybackWarpProcessor.option.OptionChannelsTogether
   )

**Common options:**

* ``OptionTransientsSmooth`` / ``OptionTransientsCrisp``: Transient handling
* ``OptionPitchHighQuality`` / ``OptionPitchHighSpeed``: Quality/speed tradeoff
* ``OptionChannelsTogether`` / ``OptionChannelsApart``: Multi-channel handling

See the `Rubber Band API documentation <https://breakfastquay.com/rubberband/code-doc/classRubberBand_1_1RubberBandStretcher.html>`_ for all available options.

Ableton Live Warp Markers
--------------------------

Loading `.asd` Files
~~~~~~~~~~~~~~~~~~~~

Ableton Live stores warp marker information in ``.asd`` files. DawDreamer can load these files to perform beat-matched time-stretching:

.. code-block:: python

   engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

   # Suppose the Ableton clip info thinks the input audio is 120 BPM,
   # but we want to play it back at 130 BPM
   engine.set_bpm(130.)

   audio, _ = librosa.load("drum_loop.wav", sr=SAMPLE_RATE, mono=False)
   playback = engine.make_playbackwarp_processor("drums", audio)

   # Load warp markers from .asd file
   playback.set_clip_file("drum_loop.wav.asd")

   engine.load_graph([(playback, [])])
   engine.render(8., beats=True)  # Render 8 beats at 130 BPM

The ``.asd`` file automatically sets several clip properties.

Clip Properties
~~~~~~~~~~~~~~~

After loading an ``.asd`` file, the following properties are set (and can be modified):

.. code-block:: python

   # Warp markers: array of (time in seconds, time in beats) pairs
   playback.warp_markers  # np.array shape [N, 2]

   # Timeline markers (in beats relative to 1.1.1)
   playback.start_marker = 0.0   # Clip start
   playback.end_marker = 4.0     # Clip end
   playback.loop_start = 0.0     # Loop start
   playback.loop_end = 4.0       # Loop end

   # Flags
   playback.warp_on = True   # Enable warping
   playback.loop_on = False  # Enable looping

.. note::
   When ``warp_on`` is ``True``, any value set by ``time_ratio`` is ignored. The warp markers control time-stretching instead.

Manual Warp Markers
~~~~~~~~~~~~~~~~~~~

You can set warp markers manually without an ``.asd`` file:

.. code-block:: python

   import numpy as np

   # Define warp markers: (seconds, beats)
   warp_markers = np.array([
       [0.0, 0.0],      # 0 seconds = 0 beats
       [1.0, 2.0],      # 1 second = 2 beats (faster tempo)
       [3.0, 4.0]       # 3 seconds = 4 beats (slower tempo)
   ])

   playback.warp_markers = warp_markers
   playback.warp_on = True

Multiple Clip Positions
------------------------

Use ``set_clip_positions()`` to place the same audio clip at multiple timeline positions:

.. code-block:: python

   # Each tuple: (global start, global end, local offset)
   playback.set_clip_positions([
       [0., 4., 0.],   # First instance: beats 0-4, no offset
       [5., 9., 1.]    # Second instance: beats 5-9, 1 beat offset
   ])

**Parameters:**

* **Global start/end**: Position on the global timeline (in beats)
* **Local offset**: Offset relative to the clip's start marker (in beats)

This is like dragging the same clip onto an arrangement view multiple times with different offsets.

Complete Example
----------------

.. code-block:: python

   import dawdreamer as daw
   import librosa
   from scipy.io import wavfile

   SAMPLE_RATE = 44100
   BUFFER_SIZE = 512

   engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
   engine.set_bpm(130.)

   # Load audio
   audio, _ = librosa.load("drum_loop.wav", sr=SAMPLE_RATE, mono=False)

   # Create processor
   playback = engine.make_playbackwarp_processor("drums", audio)

   # Load Ableton warp markers
   playback.set_clip_file("drum_loop.wav.asd")

   # Adjust clip properties
   playback.loop_on = True
   playback.loop_start = 0.0
   playback.loop_end = 4.0

   # Place clip twice on timeline
   playback.set_clip_positions([
       [0., 4., 0.],    # First loop
       [4., 8., 0.]     # Second loop
   ])

   # Set Rubber Band options
   playback.set_options(
       daw.PlaybackWarpProcessor.option.OptionTransientsCrisp |
       daw.PlaybackWarpProcessor.option.OptionPitchHighQuality
   )

   # Render
   engine.load_graph([(playback, [])])
   engine.render(8., beats=True)

   # Save
   audio_out = engine.get_audio()
   wavfile.write('warped_drums.wav', SAMPLE_RATE, audio_out.transpose())

Tips and Best Practices
------------------------

**Time-stretching quality**
   * Use ``OptionPitchHighQuality`` for better quality at the cost of CPU
   * ``OptionTransientsCrisp`` for percussive material, ``OptionTransientsSmooth`` for melodic

**Warp markers**
   * Warp markers define tempo changes across the clip
   * More markers = more flexible tempo mapping, but harder to manage
   * Use Ableton Live to visually place warp markers, then export ``.asd``

**Beat-matching**
   * Set engine BPM to target tempo
   * Load ``.asd`` file with ``set_clip_file()``
   * Enable ``warp_on`` to use warp markers instead of ``time_ratio``

**Multiple instances**
   * Use ``set_clip_positions()`` for repeated patterns
   * More efficient than creating multiple processors for the same audio

**Performance**
   * Time-stretching is CPU-intensive, especially at high quality settings
   * Larger ``time_ratio`` changes (e.g., 2x or 0.5x) are more expensive
   * Consider pre-rendering complex warp scenarios

Common Issues
-------------

**Artifacts or glitches**
   * Try different Rubber Band options (crisp vs smooth, quality vs speed)
   * Increase buffer size for more stable processing
   * Check for extreme time ratios (< 0.5 or > 2.0)

**Warp markers not working**
   * Ensure ``warp_on = True``
   * Verify warp marker format: shape [N, 2], (seconds, beats)
   * Check that ``.asd`` file corresponds to the correct audio file

**Timing mismatch**
   * Verify engine BPM matches intended playback tempo
   * Check start/end markers align with clip boundaries
   * Ensure clip positions don't overlap unexpectedly

**Loop not working**
   * Set ``loop_on = True``
   * Verify ``loop_start`` and ``loop_end`` are within clip bounds
   * Check that render duration is long enough to hear the loop

Further Reading
---------------

* :doc:`render_engine` - BPM and tempo control
* :doc:`other_processors` - Standard playback processor
* `Rubber Band Library Homepage <https://breakfastquay.com/rubberband/>`_
* `AbletonParsing Project <https://github.com/DBraun/AbletonParsing>`_ - Warp marker tools

Playback Processor
==================

The ``PlaybackProcessor`` plays back audio from memory. It's the simplest way to load audio into a processing graph.

Basic Usage
-----------

.. code-block:: python

   import dawdreamer as daw
   import librosa

   def load_audio_file(file_path, duration=None):
       sig, rate = librosa.load(file_path, duration=duration, mono=False, sr=44100)
       assert rate == 44100
       return sig

   engine = daw.RenderEngine(44100, 512)

   # Create playback processor
   vocals = engine.make_playback_processor("vocals", load_audio_file("vocals.wav"))

   # Load graph and render
   engine.load_graph([(vocals, [])])
   engine.render(5.0)
   audio = engine.get_audio()

Changing Playback Data
-----------------------

You can change the audio data without recreating the processor:

.. code-block:: python

   vocals.set_data(load_audio_file("other_vocals.wav"))
   engine.render(5.0)

This is useful for:

* Batch processing multiple files with the same graph
* Dynamically switching between audio sources
* A/B testing different audio files

Multi-Channel Audio
-------------------

The playback processor automatically handles mono, stereo, or multi-channel audio:

.. code-block:: python

   # Load mono audio (1 channel)
   mono_audio, _ = librosa.load("mono.wav", sr=44100, mono=True)
   mono_playback = engine.make_playback_processor("mono", mono_audio.reshape(1, -1))

   # Load stereo audio (2 channels)
   stereo_audio, _ = librosa.load("stereo.wav", sr=44100, mono=False)
   stereo_playback = engine.make_playback_processor("stereo", stereo_audio)

   # Load 5.1 surround audio (6 channels)
   surround_audio, _ = librosa.load("surround.wav", sr=44100, mono=False)
   surround_playback = engine.make_playback_processor("surround", surround_audio)

.. note::
   Audio data should be shaped as ``(channels, samples)``. For mono audio loaded with ``librosa``, reshape to ``(1, samples)``.

Looping
-------

Playback processors play the audio once by default. For looping, use :doc:`playback_warp` or render multiple times:

.. code-block:: python

   # Render the audio multiple times
   for i in range(3):
       engine.render(duration)
       audio = engine.get_audio()
       # Save or process each loop

Working with Different Sample Rates
------------------------------------

If your audio file has a different sample rate than the engine, resample it first:

.. code-block:: python

   import librosa

   engine = daw.RenderEngine(44100, 512)

   # Load audio at original sample rate
   audio, original_sr = librosa.load("file.wav", sr=None, mono=False)

   # Resample to engine's sample rate
   if original_sr != 44100:
       audio = librosa.resample(audio, orig_sr=original_sr, target_sr=44100)

   playback = engine.make_playback_processor("audio", audio)

Complete Example
----------------

.. code-block:: python

   import dawdreamer as daw
   import librosa
   from scipy.io import wavfile

   SAMPLE_RATE = 44100
   engine = daw.RenderEngine(SAMPLE_RATE, 512)

   # Load multiple audio files
   vocals, _ = librosa.load("vocals.wav", sr=SAMPLE_RATE, mono=False)
   drums, _ = librosa.load("drums.wav", sr=SAMPLE_RATE, mono=False)
   bass, _ = librosa.load("bass.wav", sr=SAMPLE_RATE, mono=False)

   # Create playback processors
   vocals_pb = engine.make_playback_processor("vocals", vocals)
   drums_pb = engine.make_playback_processor("drums", drums)
   bass_pb = engine.make_playback_processor("bass", bass)

   # Create mixer
   mixer = engine.make_add_processor("mixer", [0.4, 0.4, 0.4])

   # Build graph
   graph = [
       (vocals_pb, []),
       (drums_pb, []),
       (bass_pb, []),
       (mixer, ["vocals", "drums", "bass"])
   ]

   engine.load_graph(graph)

   # Render
   duration = max(vocals.shape[1], drums.shape[1], bass.shape[1]) / SAMPLE_RATE
   engine.render(duration)

   # Save result
   audio = engine.get_audio()
   wavfile.write('mix.wav', SAMPLE_RATE, audio.transpose())

Tips and Best Practices
------------------------

**Loading Audio**
   * Use ``librosa.load()`` for flexible audio loading
   * Always specify ``mono=False`` to preserve stereo/multi-channel audio
   * Set ``sr=SAMPLE_RATE`` to resample during loading
   * Use ``duration=`` parameter to load only part of a file

**Memory Management**
   * Large audio files consume memory - consider loading only needed portions
   * Use ``set_data()`` to reuse processors rather than creating new ones
   * Pre-load all audio before rendering for better performance

**Audio Format**
   * Ensure audio is shaped ``(channels, samples)``
   * Check channel count with ``audio.shape[0]``
   * Check sample count with ``audio.shape[1]``

**Timing**
   * Render duration is independent of audio length
   * Shorter render = audio gets cut off
   * Longer render = silence after audio ends
   * Calculate duration: ``duration = audio.shape[1] / SAMPLE_RATE``

Common Issues
-------------

**Audio is cut off**
   * Render duration is too short
   * Calculate proper duration from audio length
   * Example: ``duration = audio.shape[1] / sample_rate``

**Wrong audio shape error**
   * Audio must be ``(channels, samples)`` not ``(samples,)`` or ``(samples, channels)``
   * For mono: ``audio.reshape(1, -1)``
   * Use ``librosa.load(..., mono=False)`` to get correct shape

**Sample rate mismatch**
   * Resample audio to match engine sample rate
   * Use ``librosa.resample()`` or load with ``sr=SAMPLE_RATE``

**No audio output**
   * Check that audio data is not all zeros
   * Verify processor is in the graph
   * Check render duration > 0

Advanced: Time-Stretching and Pitch-Shifting
---------------------------------------------

For advanced time and pitch manipulation, use :doc:`playback_warp` instead:

.. code-block:: python

   # For basic playback: PlaybackProcessor
   playback = engine.make_playback_processor("audio", audio_data)

   # For time-stretch/pitch-shift: PlaybackWarpProcessor
   warp = engine.make_playbackwarp_processor("audio", audio_data)
   warp.time_ratio = 2.0      # Slow down 2x
   warp.transpose = -5.0      # Down 5 semitones

See Also
--------

* :doc:`playback_warp` - Time-stretching and pitch-shifting
* :doc:`other_processors` - Add (mixer) and other processors
* :doc:`render_engine` - Rendering and timing

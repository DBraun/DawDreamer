Pickling and Serialization
==========================

``RenderEngine`` and every processor type support Python's ``pickle`` module. A pickled engine captures the whole session: the graph structure, every processor's state and parameters, automation curves, MIDI events, audio data (as numpy arrays), plugin state blobs, and Faust code.

Basic Usage
-----------

.. code-block:: python

   import pickle

   import dawdreamer as daw

   engine = daw.RenderEngine(44100, 512)
   # ... configure processors and load a graph ...

   # Serialize and restore in memory
   restored_engine = pickle.loads(pickle.dumps(engine))

   # Or save and load a session file
   with open("my_session.pkl", "wb") as f:
       pickle.dump(engine, f)

   with open("my_session.pkl", "rb") as f:
       restored_engine = pickle.load(f)

Processors can also be pickled individually, and ``copy.deepcopy`` works wherever pickling does.

What Gets Preserved
-------------------

* **RenderEngine**: sample rate, block size, BPM (constant or automation array with PPQN), the graph structure, and all processors.
* **PlaybackProcessor / SamplerProcessor**: audio data as numpy arrays.
* **FaustProcessor**: DSP code, parameters, polyphony settings, MIDI, and automation. The compiled state is restored from LLVM bitcode, avoiding recompilation from source.
* **PluginProcessor**: the plugin path, the plugin's full state blob, parameters, and MIDI events. Restoring requires the same plugin to be installed at the pickled path.

Format Versioning
-----------------

Every processor's pickle state contains a ``pickle_version`` entry (currently 1). Unpickling requires an exact version match and raises a ``RuntimeError`` for data pickled by an incompatible DawDreamer version.

.. note::
   Pickled sessions are snapshots, not an archival format. For long-term storage, keep the source assets (audio files, presets, Faust code) and the script that builds the session.

Caveats
-------

* Do not pickle an engine while another thread is rendering with it.
* Large embedded audio makes large pickles; no compression is applied, so compress externally if needed.
* Plugin state restoration is only as reliable as the plugin's own state handling.

The complete field-by-field format is documented in `PICKLE_FORMAT.md <https://github.com/DBraun/DawDreamer/blob/main/PICKLE_FORMAT.md>`_. The test suite's `test_pickle.py <https://github.com/DBraun/DawDreamer/blob/main/tests/test_pickle.py>`_ has round-trip examples for every processor type.

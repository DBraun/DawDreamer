Multithreading and Multiprocessing
==================================

DawDreamer releases Python's Global Interpreter Lock (GIL) during its expensive operations, so multiple engines can do real work in parallel on ordinary Python threads. The GIL is released during:

* ``RenderEngine.render``
* ``RenderEngine.make_plugin_processor`` (plugin scanning and instantiation)
* ``PluginProcessor.save_state`` / ``load_state`` / ``load_preset`` / ``load_vst3_preset``
* All ``FaustProcessor`` compile methods (``compile``, ``compile_box``, ``compile_signals``, ``compile_from_bitcode``)

Everything else (``get_audio``, ``set_parameter``, ``add_midi_note``, graph loading, and so on) holds the GIL, but those calls are fast.

Rendering on a Thread Pool
--------------------------

The recommended pattern is **one engine per thread**. Each worker owns a private ``RenderEngine`` and its processors, and workers never share objects:

.. code-block:: python

   from concurrent.futures import ThreadPoolExecutor

   import dawdreamer as daw

   SAMPLE_RATE = 44100
   BLOCK_SIZE = 512

   def render_preset(preset_path: str):
       engine = daw.RenderEngine(SAMPLE_RATE, BLOCK_SIZE)
       synth = engine.make_plugin_processor("synth", "/path/to/synth.vst3")
       synth.load_preset(preset_path)
       synth.add_midi_note(60, 100, 0.0, 1.0)
       engine.load_graph([(synth, [])])
       engine.render(3.0)
       return engine.get_audio()

   with ThreadPoolExecutor(max_workers=8) as pool:
       results = list(pool.map(render_preset, preset_paths))

For long batches, create the engine once per worker and reuse it across items instead of rebuilding it per item. The `parallel plugin rendering example <https://github.com/DBraun/DawDreamer/tree/main/examples/multiprocessing_plugins>`_ shows this pattern with a shared work queue.

Internal Serialization
----------------------

Two operations are serialized internally with a process-wide mutex because the underlying libraries are not thread-safe:

* **Faust compilation** (libfaust DSP factory creation)
* **Plugin loading** (JUCE plugin scanning and instantiation)

Concurrent calls are safe, but they run one at a time. Rendering is not serialized, so compile or load once per worker up front and then render in parallel.

Thread-Safety Rules
-------------------

* **Do not share one engine (or its processors) across threads.** Calling methods on an engine while another thread is rendering with it is a data race. Give each thread its own engine.
* Multiple plugin instances of the same plugin in one process is the normal DAW situation and works with well-behaved plugins. A plugin that keeps global state across instances can misbehave; isolate such plugins with ``multiprocessing`` instead.
* ``open_editor()`` runs a GUI event loop and is not intended for worker threads.

When to Use Multiprocessing
---------------------------

Threads are the better default: lower memory (one Python process), faster startup (the plugin binary loads into one process), and no pickling of work items. Prefer ``multiprocessing`` when:

* A plugin misbehaves with multiple instances in one process.
* You want crash isolation: a plugin that segfaults takes down only its worker process.

The worker structure is the same in both cases; only the pool and queue types change.

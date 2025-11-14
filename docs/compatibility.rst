Plugin Compatibility
====================

This page lists known compatibility information for VST/AU plugins with DawDreamer. Compatibility can vary by operating system and plugin format.

.. note::
   This is a community-maintained list. If you've tested a plugin not listed here, please `open an issue <https://github.com/DBraun/DawDreamer/issues>`_ to share your results.

Legend
------

* ✔️ = Works
* ❌ = Does not work
* ❔ = Unknown/untested

Effect Plugins
--------------

.. list-table::
   :header-rows: 1
   :widths: 25 15 15 20 15 30

   * - Plugin Name
     - OS
     - Format
     - Support?
     - Preset Support
     - Notes
   * - `Bloom <https://www.fxpansion.com/products/bloom/>`_
     - Windows
     - .dll
     - ✔️
     - ❔
     -
   * - `CHOWTapeModel <https://github.com/jatinchowdhury18/AnalogTapeModel>`_
     - Windows
     - .dll
     - ✔️
     - ❔
     -
   * - `Dimension Expander <https://xferrecords.com/freeware>`_
     - Windows
     - .dll
     - ✔️
     - N/A
     -
   * - `Goodhertz <https://goodhertz.com/>`_
     - Windows
     - .vst3, .dll
     - ✔️
     - ❔
     -
   * - `IEM Plugins <https://plugins.iem.at/>`_
     - Windows
     - .dll
     - ✔️
     - ❔
     - `.vst3 doesn't work <https://github.com/DBraun/DawDreamer/issues/46#issuecomment-1103945516>`_
   * - `IEM Plugins <https://plugins.iem.at/>`_
     - Linux
     - ❔
     - ❌
     - ❔
     - `Incorrect output <https://github.com/DBraun/DawDreamer/issues/46#issuecomment-1103945516>`_
   * - `RoughRider3 <https://www.audiodamage.com/pages/free-downloads>`_
     - macOS
     - .vst3, .vst, .component
     - ✔️
     - ❔
     -
   * - `TAL-Chorus-LX <https://tal-software.com/products/tal-chorus-lx>`_
     - Windows
     - .vst3, .dll
     - ✔️
     - ❔
     -
   * - `Szechuan Saturator <https://codalabs.io/szechuan/>`_
     - Windows
     - .vst3
     - ✔️
     - ❔
     -
   * - `Valhalla Freq Echo <https://valhalladsp.com/shop/delay/valhalla-freq-echo/>`_
     - macOS
     - .component
     - ✔️
     - ❔
     -
   * - `Valhalla Freq Echo <https://valhalladsp.com/shop/delay/valhalla-freq-echo/>`_
     - macOS
     - .vst, .vst3
     - ❌
     - ❔
     - Usually outputs NaN

Instrument/Synthesizer Plugins
-------------------------------

.. list-table::
   :header-rows: 1
   :widths: 25 15 15 20 15 30

   * - Plugin Name
     - OS
     - Format
     - Support?
     - Preset Type
     - Notes
   * - `Addictive Drums 2 <https://www.xlnaudio.com/products/addictive_drums_2>`_
     - ❔
     - ❔
     - ❌
     - ❔
     - `Issue #67 <https://github.com/DBraun/DawDreamer/issues/67>`_
   * - `Dexed 0.9.4 <https://asb2m10.github.io/dexed/>`_
     - Windows
     - .dll
     - ✔️
     - ❔
     -
   * - `Dexed 0.9.6 <https://asb2m10.github.io/dexed/>`_
     - Windows
     - .vst3
     - ❌
     - ❔
     - `Issue #149 <https://github.com/DBraun/DawDreamer/issues/149>`_
   * - `Dexed <https://asb2m10.github.io/dexed/>`_
     - macOS
     - .vst3
     - ✔️
     - ❔
     -
   * - `Diva <https://u-he.com/products/diva/>`_
     - Linux
     - .so
     - ✔️
     - ❔
     - `Issue #120 <https://github.com/DBraun/DawDreamer/issues/120#issuecomment-1424223753>`_
   * - `Halion <https://www.steinberg.net/vst-instruments/halion/>`_
     - Windows
     - .dll
     - ❌
     - ❔
     - `Issue #41 <https://github.com/DBraun/DawDreamer/issues/41>`_
   * - `Helm <https://tytel.org/helm/>`_
     - macOS
     - .vst3, .vst, .component
     - ❌
     - ❔
     -
   * - `Kontakt <https://www.native-instruments.com/en/products/komplete/samplers/kontakt-6/>`_
     - Windows
     - .dll
     - ✔️
     - .nkm
     - `Issue #23 <https://github.com/DBraun/DawDreamer/issues/23#issuecomment-783549253>`_
   * - `LABS - Spitfire <https://labs.spitfireaudio.com/>`_
     - macOS
     - .vst, .vst3
     - ✔️
     - ❔
     - `Might need time.sleep(1) - Issue #78 <https://github.com/DBraun/DawDreamer/issues/78>`_
   * - `LABS - Spitfire <https://labs.spitfireaudio.com/>`_
     - Windows
     - .dll
     - ✔️
     - ❔
     - `Issue #86 <https://github.com/DBraun/DawDreamer/issues/86>`_
   * - `Repro <https://u-he.com/products/repro/>`_
     - Linux
     - .so
     - ✔️
     - ❔
     - `Issue #120 <https://github.com/DBraun/DawDreamer/issues/120#issuecomment-1424223753>`_
   * - `Serum <https://xferrecords.com/products/serum/>`_
     - Windows
     - .dll
     - ✔️
     - .fxp
     - Presets work
   * - `Sylenth1 <https://www.lennardigital.com/sylenth1/>`_
     - Windows
     - .dll
     - ✔️
     - .fxp
     - Presets work
   * - `TAL-Noisemaker <https://tal-software.com/products/tal-noisemaker>`_
     - Windows
     - .vst3, .dll
     - ✔️
     - ❔
     -
   * - `TAL-Noisemaker <https://tal-software.com/products/tal-noisemaker>`_
     - macOS
     - .vst
     - ✔️
     - ❔
     -
   * - `Upright Piano <https://plugins4free.com/plugin/3556/>`_
     - Windows
     - .dll
     - ✔️
     - ❔
     -

General Compatibility Notes
----------------------------

Format Compatibility
~~~~~~~~~~~~~~~~~~~~

* **VST3**: Generally well-supported across platforms, but some specific plugins have issues
* **VST2 (.dll, .vst)**: Widely compatible, especially on Windows
* **Audio Units (.component)**: macOS-specific format, generally works well
* **Linux (.so)**: Limited testing, but many plugins work

Common Issues
~~~~~~~~~~~~~

**Plugin doesn't load**
   * Ensure plugin path is absolute
   * Check plugin format matches OS
   * Some plugins require authorization/installation before use

**NaN (Not a Number) output**
   * Known issue with some VST3 plugins (e.g., Valhalla Freq Echo .vst3)
   * Try VST2 or AU format if available
   * Report issue on GitHub with plugin details

**UI-dependent plugins**
   * Some plugins require GUI interaction for initialization
   * May need ``open_editor()`` before use
   * Can cause issues in headless environments

**Preset loading fails**
   * Check preset format matches plugin (.fxp, .nkm, .vstpreset, etc.)
   * Use ``load_state()`` for complete plugin state
   * Some plugins don't support programmatic preset loading

Platform-Specific Notes
~~~~~~~~~~~~~~~~~~~~~~~

**macOS**
   * Audio Units (.component) generally work well
   * Some .vst3 plugins have issues (try .vst or .component instead)
   * May need to grant security permissions for unsigned plugins

**Windows**
   * Best compatibility overall
   * .dll (VST2) format most reliable
   * Some .vst3 plugins work, some don't (test individually)

**Linux**
   * Limited plugin ecosystem compared to Windows/macOS
   * .so format support varies
   * Consider using native Faust processors as alternative

Testing Your Plugins
---------------------

To test a plugin's compatibility:

.. code-block:: python

   import dawdreamer as daw

   engine = daw.RenderEngine(44100, 512)

   try:
       plugin = engine.make_plugin_processor("test", "/path/to/plugin.dll")
       print(f"✔️ Plugin loaded successfully")
       print(f"Inputs: {plugin.get_num_input_channels()}")
       print(f"Outputs: {plugin.get_num_output_channels()}")

       # Try rendering
       engine.load_graph([(plugin, [])])
       engine.render(1.0)
       audio = engine.get_audio()

       # Check for NaN
       if np.isnan(audio).any():
           print("❌ Output contains NaN values")
       else:
           print("✔️ Rendering successful")

   except Exception as e:
       print(f"❌ Error: {e}")

Contributing Compatibility Info
--------------------------------

If you test a plugin not listed here:

1. Test basic loading and rendering
2. Test preset loading (if applicable)
3. Check for NaN output or other issues
4. `Open an issue <https://github.com/DBraun/DawDreamer/issues>`_ with your findings:

   * Plugin name and version
   * Operating system
   * Plugin format (.dll, .vst3, .component, etc.)
   * Whether it works (✔️ or ❌)
   * Any special notes or workarounds

Alternative to VST Plugins
---------------------------

If you encounter plugin compatibility issues, consider:

* **Faust Processor**: Write custom DSP code (see :doc:`user_guide/faust_processor`)
* **Built-in processors**: Use DawDreamer's native effects (see :doc:`user_guide/other_processors`)
* **Different plugin format**: Try .vst instead of .vst3, or .component on macOS

See Also
--------

* :doc:`user_guide/plugin_processor` - Plugin Processor documentation
* :doc:`user_guide/faust_processor` - Faust as an alternative to plugins
* `GitHub Issues <https://github.com/DBraun/DawDreamer/issues>`_ - Report and track compatibility problems

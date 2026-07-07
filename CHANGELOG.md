# Changelog

All notable changes to DawDreamer are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
DawDreamer uses effort-based versioning: version numbers reflect the scope of
changes rather than strict semantic versioning rules. Breaking changes are
called out explicitly in each release's notes.

## [Unreleased]

### Added

- Linux aarch64 and macOS Intel (x86_64) wheels, published alongside the
  existing Linux x86_64, macOS arm64, and Windows x86_64 wheels.
- Python 3.13 and 3.14 support, tested in CI.
- Pickling support for `RenderEngine` and all processors (see
  [PICKLE_FORMAT.md](PICKLE_FORMAT.md)).
- Comprehensive Sphinx documentation: user guide, Faust Box/Signal API
  reference, and content migrated from the GitHub Wiki.
- Pre-commit configuration (ruff, clang-format, codespell, shellcheck).

### Changed

- Python bindings switched from pybind11 to [nanobind](https://github.com/wjakob/nanobind)
  (v2.13.0).
- Faust and libfaust updated to 2.85.9, with the matching faustlibraries.
- The Sampler processor was updated for JUCE 8.
- Minimum Python version is now 3.11.
- Deprecated JUCE APIs were replaced; the C++ build compiles without warnings.
- Requesting a plugin parameter name or text for an out-of-range index now
  raises an exception instead of returning an empty string.

### Fixed

- `make_panner_processor` did not clamp pan values below -1.
- `load_graph` now validates input names immediately instead of failing later
  during render.

### Removed

- Wheels no longer bundle faustlibraries documentation, tests, and mesh2faust
  CAD sources (`modalmodels`), or Faust architecture app-project scaffolding.
  Architecture wrapper files usable with `-a` (such as `jax/minimal.py`) are
  still included.

## [0.8.3] - 2024-09-09

### Added

- `PluginProcessor.get_parameter_range(index, search_steps=1000, convert=True)`
  extracts parameter ranges with optional text-to-numerical conversion.
- `FaustProcessor.opt_level` property to control the LLVM optimization level.
- `FaustProcessor.faust_libraries_paths` and `faust_assets_paths` (plural)
  for multiple search paths.
- `boxFFun` and `sigFFun` in the Box and Signal APIs.

### Changed

- libfaust updated to 2.70.3.
- macOS builds require macOS 12 (Monterey) or later.
- All platforms require Python 3.10 or higher.

### Fixed

- `boxVGroup` incorrectly referenced `boxHGroup`.

## [0.8.0] - 2023-11-17

### Changed

- **Breaking:** `boxFromDSP` returns just a `box` instead of the tuple
  `(box, inputs, outputs)`. Use the new `.inputs`/`.outputs` properties.
- libfaust updated to 2.69.3; JUCE updated to 7.0.8.

### Added

- Box objects gained `.valid`, `.inputs`, and `.outputs` properties.
- `FaustProcessor.compile_box` searches for library files in
  `.faust_libraries_path`.
- Python 3.12 wheels on PyPI.

### Removed

- **Breaking:** `getBoxType` was removed. Use the Box properties instead.

## [0.7.4] - 2023-09-26

### Fixed

- MIDI notes now turn off properly when a render concludes before a Note Off
  event.
- Off-by-one sample error in the output size when the block size is 2.

### Changed

- JUCE updated to 7.0.7; libfaust updated to 2.68.1; pybind11, rubberband, and
  libsamplerate updated.
- Python 3.8+ required for Windows/Linux wheels; Python 3.9+ for macOS.

### Added

- The Faust `soundfile` primitive can load directly from the filesystem, with
  automatic fallback to the `set_soundfiles()` dictionary.
- Python numeric types implicitly convert in `boxReal`/`boxInt` and
  `sigReal`/`sigInt` contexts.

## [0.7.3] - 2023-09-18

### Fixed

- MIDI note handling between renders (notes turn off if a render ends before a
  Note Off event).
- Output render size calculation (off-by-one sample when the block size is 2).

### Changed

- JUCE updated to 7.0.7; libfaust updated to 2.68.1; pybind11 updated.
- Refactored the Sampler processor plugin project and the libfaust Box/Signal
  API files.

## [0.7.1] - 2023-06-01

### Added

- Multiprocessing support, with tests and examples.

### Changed

- Removed an unnecessary mutex in `PluginProcessor`.
- The JUCE `MessageManager` instance is created before the
  `AudioProcessorGraph`.

## [0.7.0] - 2023-05-18

### Changed

- **Breaking:** macOS wheels require Python 3.9+ and macOS 11.0+.
- Faust updated to 2.59.6 with the associated Faust libraries.
- libfaust is downloaded at build time instead of being vendored in the
  repository.

### Fixed

- Crash in `PluginProcessor.record_automation`.
- Several `isBox` functions in the Faust Box API.

## [0.6.3] - 2022-04-19

### Added

- `PluginProcessor.open_editor()`, `load_state(filepath)`,
  `save_state(filepath)`, `can_set_bus(inputs, outputs)`, and
  `set_bus(inputs, outputs)`.

## [0.6.1] - 2022-04-04

### Added

- `PlaybackWarpProcessor.reset_warp_markers(bpm)` and the `warp_markers`
  property ([#73](https://github.com/DBraun/DawDreamer/issues/73)).

### Fixed

- `PlaybackWarpProcessor` behavior when warping is disabled
  ([#81](https://github.com/DBraun/DawDreamer/issues/81)).

## [0.6.0] - 2022-03-24

### Changed

- **Breaking:** functions like `load_graph()` throw exceptions instead of
  returning bools.
- The render engine warns instead of erroring when too many signals connect to
  a node.
- JUCE, Faust, and pybind11 upgraded.

### Added

- `all_events` keyword argument for `PluginProcessor.load_midi()` (default
  True for backwards compatibility).
- `FaustProcessor.faust_libraries_path` for custom `.lib` file locations.
- Faust release-length property to avoid false-positive voice stealing
  warnings.

### Fixed

- Faust processor MIDI file loading.
- Add processor channel count (now always stereo).

## [0.5.8.1] - 2022-01-13

### Added

- Multi-channel support for VSTs, Faust, and other processors, with
  `get_num_input_channels()` and `get_num_output_channels()`.

### Changed

- Better macOS support for Apple Silicon and Intel, and an improved
  `pip install dawdreamer` experience.

## [0.5.7.8] - 2021-10-09

### Added

- The Faust `soundfile` primitive, with `set_soundfiles()` for passing NumPy
  audio from Python.

## [0.5.7] - 2021-08-14

### Changed

- **Breaking:** `PluginProcessor.set_automation` identifies parameters by
  index instead of name, because parameter names aren't always unique.

## [0.5.6] - 2021-05-09

### Added

- The Faust processor: real-time Faust DSP compilation and execution.

### Removed

- JUCE from the global namespace.

## [0.5.0] - 2021-06-18

### Added

- Linux support (thanks [@guillaumephd](https://github.com/guillaumephd)).

## [0.4.0] - 2020-12-08

### Added

- Parameter automation at audio rate and PPQN rate.

## [0.1.0] - 2020-08-14

### Added

- First public release: core RenderEngine, processor graph system, VST plugin
  hosting.

[Unreleased]: https://github.com/DBraun/DawDreamer/compare/v0.8.3...HEAD
[0.8.3]: https://github.com/DBraun/DawDreamer/compare/v0.8.0...v0.8.3
[0.8.0]: https://github.com/DBraun/DawDreamer/compare/v0.7.4...v0.8.0
[0.7.4]: https://github.com/DBraun/DawDreamer/compare/v0.7.3...v0.7.4
[0.7.3]: https://github.com/DBraun/DawDreamer/compare/v0.7.1...v0.7.3
[0.7.1]: https://github.com/DBraun/DawDreamer/compare/v0.7.0...v0.7.1
[0.7.0]: https://github.com/DBraun/DawDreamer/compare/v0.6.16...v0.7.0
[0.6.3]: https://github.com/DBraun/DawDreamer/compare/v0.6.1...v0.6.3
[0.6.1]: https://github.com/DBraun/DawDreamer/compare/v0.6.0...v0.6.1
[0.6.0]: https://github.com/DBraun/DawDreamer/compare/v0.5.8.2...v0.6.0
[0.5.8.1]: https://github.com/DBraun/DawDreamer/compare/v0.5.7.9...v0.5.8.1
[0.5.7.8]: https://github.com/DBraun/DawDreamer/compare/v0.5.7.7...v0.5.7.8
[0.5.7]: https://github.com/DBraun/DawDreamer/releases
[0.5.6]: https://github.com/DBraun/DawDreamer/releases
[0.5.0]: https://github.com/DBraun/DawDreamer/releases/tag/v0.5.0
[0.4.0]: https://github.com/DBraun/DawDreamer/releases/tag/v0.4.0
[0.1.0]: https://github.com/DBraun/DawDreamer/releases

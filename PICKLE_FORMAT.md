# DawDreamer Pickle Format Documentation

## Overview

DawDreamer supports pickling (serialization) of `RenderEngine` and all processor types using Python's `pickle` module. This allows you to save and restore complete audio processing graphs, including all processor state, parameters, automation curves, and MIDI events.

## Basic Usage

```python
import pickle
import dawdreamer as daw

# Create and configure a RenderEngine
engine = daw.RenderEngine(44100, 512)
# ... configure processors and graph ...
engine.render(duration)

# Serialize to bytes
pickled_bytes = pickle.dumps(engine)

# Deserialize
restored_engine = pickle.loads(pickled_bytes)

# Or save/load from file
with open("my_session.pkl", "wb") as f:
    pickle.dump(engine, f)

with open("my_session.pkl", "rb") as f:
    restored_engine = pickle.load(f)
```

## Supported Processor Types

All processor types support pickling:

- **RenderEngine** - Complete audio graph with all processors
- **PlaybackProcessor** - Audio data preserved as numpy arrays
- **FaustProcessor** - DSP code, parameters, polyphony settings, MIDI, automation
- **PluginProcessor** - Plugin path, VST state blob, MIDI events
- **SamplerProcessor** - Sample data, parameters, MIDI events
- **OscillatorProcessor** - Frequency setting
- **FilterProcessor** - Type, frequency, Q, gain
- **CompressorProcessor** - Threshold, ratio, attack, release
- **ReverbProcessor** - Room size, damping, wet/dry, width, freeze
- **PannerProcessor** - Panning rule, pan value
- **DelayProcessor** - Mode, delay time, feedback
- **AddProcessor** - Gain levels for each input

## What Gets Preserved

### RenderEngine State
- Sample rate
- Buffer size
- BPM (single value or automation array)
- PPQN (pulses per quarter note)
- Audio processing graph structure
- All processor instances and their connections

### Processor State (Common to All)
- Unique name
- Sample rate
- Parameter values
- Parameter automation curves (numpy arrays)

### Processor-Specific State

#### PlaybackProcessor
- Complete audio data as numpy array (shape: [channels, samples])

#### FaustProcessor
- DSP source code string
- Faust library paths
- Compiled state (automatically recompiled on restore)
- Polyphony settings: `num_voices`, `group_voices`, `dynamic_voices`, `release_length`
- All parameter values and automation curves
- MIDI events (both beat-based and second-based)

#### PluginProcessor
- Plugin file path
- Plugin state blob (VST/AU internal state via `getStateInformation()`)
- Parameter values (restored from plugin state)
- MIDI events (both beat-based and second-based)

#### SamplerProcessor
- Original sample data (non-upsampled)
- All sampler parameters (attack, decay, sustain, release, etc.)
- MIDI events (both beat-based and second-based)

## MIDI Serialization Format

MIDI events are serialized using a binary format for efficiency:

### Binary Format (per MIDI message)
```
[Sample Position: 4 bytes, big-endian]
[Message Size: 2 bytes, big-endian]
[Message Data: variable length]
```

- **Sample Position** (int32): Sample-accurate timing position
- **Message Size** (uint16): Number of bytes in MIDI message
- **Message Data**: Raw MIDI bytes (typically 3 bytes for note on/off)

### MIDI Buffer Types
Each processor with MIDI support maintains two separate buffers:
- **myMidiBufferQN**: Beat-based events (quarter notes)
- **myMidiBufferSec**: Second-based events (absolute time)

Both buffers are preserved independently during pickling.

## Implementation Details

### Placement New Pattern
DawDreamer uses nanobind's placement new pattern for reconstruction:

```cpp
void setPickleState(nb::dict state) {
    // Extract parameters from state dictionary
    std::string name = nb::cast<std::string>(state["unique_name"]);
    // ... extract other parameters ...

    // Reconstruct object in-place using placement new
    new (this) ProcessorType(name, ...);

    // Restore additional state
    // ... restore parameters, MIDI, etc. ...
}
```

This pattern is required by nanobind to properly reconstruct C++ objects within the Python object lifecycle.

### Parameter Automation Restoration
Automation curves are restored **after** the audio graph is compiled:

1. During pickle: Save automation arrays with processor name and parameter name
2. During unpickle: Build processor map during graph restoration
3. After graph compilation: Apply saved automation to processors by name

This deferred restoration is necessary because:
- Processors may be reconstructed during unpickling
- Graph compilation creates the final processor instances
- Automation must be applied to the compiled processor instances

### VST/AU Plugin State
Plugins use JUCE's binary state format:
- `getStateInformation()` creates a binary blob of plugin state
- `setStateInformation()` restores plugin from binary blob
- This format is plugin-specific and opaque to DawDreamer
- Parameter values are automatically restored from plugin state

## Limitations and Caveats

### 1. Platform and Architecture
- **Plugin paths must be valid** on the restore system
- VST/AU plugins must be installed at the same paths
- Plugin state blobs are generally portable but plugin-specific
- Cross-platform compatibility depends on plugin implementation

### 2. Faust Processors
- DSP code is preserved and **recompiled on restore**
- Faust libraries must be available at restore time
- `faust_libraries_paths` should point to valid locations
- Compilation may fail if Faust version differs significantly

### 3. Sample Rate and Buffer Size
- Sample rate is preserved and enforced
- Buffer size changes are supported but may affect timing
- Audio data is NOT resampled automatically

### 4. File References
- **PlaybackProcessor**: Audio data is embedded (can be large)
- **PluginProcessor**: Only plugin path is stored (plugin must exist)
- **SamplerProcessor**: Sample data is embedded
- No automatic path resolution or file tracking

### 5. Automation Timing
- Automation arrays are sample-accurate
- Changing sample rate after restore will affect timing
- BPM automation is preserved as-is (no time-stretching)

### 6. MIDI Timing
- MIDI events use sample positions (sample-accurate)
- Beat-based MIDI depends on BPM at restore time
- No MIDI time-stretching or quantization is applied

### 7. Real-time State
- Audio processing state (buffers, delay lines) is **NOT** preserved
- Processors are reset to initial state after restore
- No continuation of reverb tails, delay feedback, etc.

### 8. Graph Topology
- Graph structure is preserved exactly
- Processor order matters (serialized in graph order)
- Cyclic graphs are not supported (limitation of DawDreamer, not pickle)

### 9. Memory Considerations
- **Large audio data can create large pickle files**
- PlaybackProcessor and SamplerProcessor embed full audio
- Consider external file storage for large audio datasets
- No compression is applied (use `pickle.HIGHEST_PROTOCOL` and compress externally)

### 10. Thread Safety
- Pickling is **not thread-safe**
- Do not pickle while rendering
- Ensure exclusive access during serialization

## Versioning Considerations

**Current Status**: No explicit version tracking (as of v0.8.4)

Future versions may add:
- Version number in pickle state dictionaries
- Backward compatibility checks
- Migration code for format changes

**Recommendation**: Store the DawDreamer version alongside pickled data:
```python
import dawdreamer as daw
data = {
    'version': daw.__version__,
    'engine': pickle.dumps(engine)
}
```

## Best Practices

### 1. Version Tracking
Always store the DawDreamer version used to create pickles:
```python
metadata = {
    'dawdreamer_version': daw.__version__,
    'created_date': datetime.now().isoformat(),
    'sample_rate': SAMPLE_RATE,
}
```

### 2. Validation After Restore
Always verify restored state:
```python
restored_engine = pickle.loads(pickled_bytes)
# Verify graph structure
assert len(restored_engine.get_audio().shape) == 2
# Test render
restored_engine.render(0.1)  # Short test render
```

### 3. Plugin Path Handling
Store plugin paths separately for cross-platform support:
```python
# Before pickle
plugin_paths = {
    'effect': plugin.plugin_path,
    'instrument': instrument.plugin_path
}

# After restore, verify paths exist
if not os.path.exists(plugin_paths['effect']):
    # Handle missing plugin
```

### 4. Large Audio Data
For large projects, consider hybrid approach:
```python
# Option 1: Separate audio storage
audio_data = playback.get_audio()
np.save('audio.npy', audio_data)
# ... pickle without audio data ...

# Option 2: Compressed pickle
import gzip
with gzip.open('session.pkl.gz', 'wb') as f:
    pickle.dump(engine, f)
```

### 5. Error Handling
Always wrap unpickling in try-except:
```python
try:
    engine = pickle.loads(pickled_bytes)
except Exception as e:
    print(f"Failed to restore session: {e}")
    # Handle error (use backup, notify user, etc.)
```

## Testing

The test suite (`tests/test_pickle.py`) includes:
- Round-trip tests for all processor types
- MIDI preservation tests (empty, normal, large buffers)
- Automation curve preservation
- Complex graph structures
- Edge cases and error conditions

Run tests:
```bash
pytest tests/test_pickle.py -v
```

## Technical References

- **Nanobind Documentation**: https://nanobind.readthedocs.io/
- **Python Pickle Protocol**: https://docs.python.org/3/library/pickle.html
- **JUCE VST State**: Uses `AudioProcessor::getStateInformation()`
- **MIDI Format**: Based on JUCE `MidiBuffer` iteration

## Future Enhancements

Potential improvements for future versions:

1. **Explicit versioning** - Add version field to all pickle states
2. **Compression** - Optional built-in compression for audio data
3. **Incremental updates** - Patch format for parameter changes
4. **Validation** - Checksum/hash verification of restored state
5. **Migration** - Automatic format migration for older versions
6. **Streaming** - Support for lazy loading of large audio data
7. **Metadata** - Standardized metadata fields (author, date, description)

## Contributing

When adding new processors or extending pickle support:

1. Implement `getPickleState()` returning `nb::dict`
2. Implement `setPickleState(nb::dict)` using placement new
3. Add comprehensive tests to `test_pickle.py`
4. Update this documentation
5. Consider backward compatibility impact

See existing processors for implementation patterns.

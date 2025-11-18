#pragma once

/**
 * Pickle format version for DawDreamer serialization.
 *
 * This version number tracks changes to the pickle format across DawDreamer releases.
 * It should be incremented whenever the pickle format changes in a way that breaks
 * backward compatibility.
 *
 * Version History:
 * - Version 1 (0.8.4): Initial versioned pickle format
 *   - All processor types support pickling
 *   - MIDI events preserved for FaustProcessor, PluginProcessor, SamplerProcessor
 *   - Parameter automation preserved
 *   - RenderEngine graph structure preserved
 */
#define DAWDREAMER_PICKLE_VERSION 1

namespace DawDreamerPickle
{
/**
 * Get the current pickle format version.
 */
inline int getVersion()
{
    return DAWDREAMER_PICKLE_VERSION;
}

/**
 * Check if a pickle version is compatible with the current version.
 *
 * @param version The version to check
 * @return true if compatible, false otherwise
 */
inline bool isCompatibleVersion(int version)
{
    // For now, we only support exact version match
    // Future versions may support backward compatibility
    return version == DAWDREAMER_PICKLE_VERSION;
}

/**
 * Get a human-readable error message for incompatible versions.
 */
inline std::string getVersionErrorMessage(int found_version)
{
    return "Incompatible pickle version: found version " + std::to_string(found_version) +
           ", expected version " + std::to_string(DAWDREAMER_PICKLE_VERSION) +
           ". This pickle was created with a different version of DawDreamer.";
}

} // namespace DawDreamerPickle

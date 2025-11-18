#pragma once

#include "custom_nanobind_wrappers.h"
#include <JuceHeader.h>
#include <vector>

/**
 * Shared MIDI serialization utilities for DawDreamer processors.
 *
 * These helpers provide a consistent way to serialize and deserialize
 * JUCE MidiBuffer objects to/from nanobind bytes for pickling support.
 */
namespace MidiSerialization
{
/**
 * Serialize a JUCE MidiBuffer to bytes.
 *
 * Format: For each MIDI message:
 *   - Sample position (4 bytes, big-endian)
 *   - Message size (2 bytes, big-endian)
 *   - Message data (variable length)
 *
 * @param buffer The JUCE MidiBuffer to serialize
 * @return nanobind bytes containing the serialized MIDI data
 */
inline nb::bytes serializeMidiBuffer(const juce::MidiBuffer& buffer)
{
    std::vector<uint8_t> data;

    // Iterate through all MIDI messages in the buffer
    for (const auto metadata : buffer)
    {
        auto message = metadata.getMessage();
        int samplePosition = metadata.samplePosition;

        // Store sample position (4 bytes, big-endian)
        data.push_back((samplePosition >> 24) & 0xFF);
        data.push_back((samplePosition >> 16) & 0xFF);
        data.push_back((samplePosition >> 8) & 0xFF);
        data.push_back(samplePosition & 0xFF);

        // Store message size (2 bytes for safety, though MIDI messages are small)
        int numBytes = message.getRawDataSize();
        data.push_back((numBytes >> 8) & 0xFF);
        data.push_back(numBytes & 0xFF);

        // Store message data
        const uint8_t* rawData = message.getRawData();
        for (int i = 0; i < numBytes; i++)
        {
            data.push_back(rawData[i]);
        }
    }

    return nb::bytes((const char*)data.data(), data.size());
}

/**
 * Deserialize bytes to a JUCE MidiBuffer.
 *
 * Reconstructs MIDI messages from the format created by serializeMidiBuffer().
 * Clears the buffer before adding deserialized messages.
 *
 * @param buffer The JUCE MidiBuffer to populate (will be cleared)
 * @param data nanobind bytes containing serialized MIDI data
 */
inline void deserializeMidiBuffer(juce::MidiBuffer& buffer, nb::bytes data)
{
    buffer.clear();

    const uint8_t* bytes = (const uint8_t*)data.c_str();
    size_t size = data.size();
    size_t pos = 0;

    while (pos + 6 <= size) // Need at least 6 bytes (4 for position + 2 for size)
    {
        // Read sample position (4 bytes, big-endian)
        int samplePosition =
            (bytes[pos] << 24) | (bytes[pos + 1] << 16) | (bytes[pos + 2] << 8) | bytes[pos + 3];
        pos += 4;

        // Read message size (2 bytes)
        int numBytes = (bytes[pos] << 8) | bytes[pos + 1];
        pos += 2;

        // Read message data
        if (pos + numBytes <= size)
        {
            juce::MidiMessage message(bytes + pos, numBytes, samplePosition);
            buffer.addEvent(message, samplePosition);
            pos += numBytes;
        }
        else
        {
            break; // Corrupted data
        }
    }
}

} // namespace MidiSerialization

/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

AudioFormatReaderSource::AudioFormatReaderSource (AudioFormatReader* const r,
                                                  const bool deleteReaderWhenThisIsDeleted)
    : reader (r, deleteReaderWhenThisIsDeleted),
      nextPlayPos (0),
      looping (false)
{
    jassert (reader != nullptr);
}

AudioFormatReaderSource::~AudioFormatReaderSource() {}

int64 AudioFormatReaderSource::getTotalLength() const                   { return reader->lengthInSamples; }
void AudioFormatReaderSource::setNextReadPosition (int64 newPosition)   { nextPlayPos = newPosition; }
void AudioFormatReaderSource::setLooping (bool shouldLoop)              { looping = shouldLoop; }

int64 AudioFormatReaderSource::getNextReadPosition() const
{
    return looping ? nextPlayPos % reader->lengthInSamples
                   : nextPlayPos;
}

void AudioFormatReaderSource::prepareToPlay (int /*samplesPerBlockExpected*/, double /*sampleRate*/) {}
void AudioFormatReaderSource::releaseResources() {}

void AudioFormatReaderSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    if (info.numSamples > 0)
    {
        const int64 start = nextPlayPos;

        if (looping)
        {
            const int64 newStart = start % reader->lengthInSamples;
            const int64 newEnd = (start + info.numSamples) % reader->lengthInSamples;

            if (newEnd > newStart)
            {
                reader->read (info.buffer, info.startSample,
                              (int) (newEnd - newStart), newStart, true, true);
            }
            else
            {
                const int endSamps = (int) (reader->lengthInSamples - newStart);

                reader->read (info.buffer, info.startSample,
                              endSamps, newStart, true, true);

                reader->read (info.buffer, info.startSample + endSamps,
                              (int) newEnd, 0, true, true);
            }

            nextPlayPos = newEnd;
        }
        else
        {
            const auto samplesToRead = jlimit (int64{},
                                               (int64) info.numSamples,
                                               reader->lengthInSamples - start);

            reader->read (info.buffer, info.startSample, (int) samplesToRead, start, true, true);
            info.buffer->clear ((int) (info.startSample + samplesToRead),
                                (int) (info.numSamples - samplesToRead));

            nextPlayPos += info.numSamples;
        }
    }
}

} // namespace juce

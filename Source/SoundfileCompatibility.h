#pragma once

namespace dawdreamer
{
// Older Faust compilers index duplicated channel pointers directly. Newer
// compilers wrap by fChannels and their Soundfile no longer has shareBuffers.
template <typename SoundfileType>
auto shareSoundfileBuffers(SoundfileType* soundfile, int channels, int maxChannels, int)
    -> decltype(soundfile->shareBuffers(channels, maxChannels), void())
{
    soundfile->shareBuffers(channels, maxChannels);
}

template <typename SoundfileType> void shareSoundfileBuffers(SoundfileType*, int, int, ...) {}
} // namespace dawdreamer

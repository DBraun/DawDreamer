// Compile with -I../Source and -I<faust>/architecture, against both old and new
// Faust headers. No JUCE or Python extension is needed for this runtime check.
#include "faust/gui/Soundfile.h"
#include "SoundfileCompatibility.h"
#include <cassert>

template <typename SoundfileType>
auto checkLegacyAliases(SoundfileType& soundfile, int)
    -> decltype(soundfile.shareBuffers(2, MAX_CHAN), void())
{
    auto buffers = static_cast<float**>(soundfile.fBuffers);
    for (int channel = 2; channel < MAX_CHAN; ++channel)
        assert(buffers[channel] == buffers[channel % 2]);
}

template <typename SoundfileType> void checkLegacyAliases(SoundfileType&, ...) {}

int main()
{
    Soundfile soundfile(2, 8, MAX_CHAN, 1, false);
    auto buffers = static_cast<float**>(soundfile.fBuffers);
    buffers[0][0] = 0.25f;
    buffers[1][0] = 0.5f;
    dawdreamer::shareSoundfileBuffers(&soundfile, 2, MAX_CHAN, 0);
    assert(soundfile.fChannels == 2);
    assert(buffers[0][0] == 0.25f && buffers[1][0] == 0.5f);
    checkLegacyAliases(soundfile, 0);
}

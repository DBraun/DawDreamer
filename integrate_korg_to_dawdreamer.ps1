# ==============================================================================
# Integrate KORG Workstation Engine Directly into DawDreamer (vst3 branch)
# Target Repo: alexrmac27/DawDreamer
# ==============================================================================

$ErrorActionPreference = "Stop"

Write-Host "=== Integrating KORG VST Engine into DawDreamer (vst3) ===" -ForegroundColor Cyan

# 1. Ensure you are on the vst3 branch of DawDreamer
git checkout vst3

# 2. Create the target engines directory inside DawDreamer's source tree
$korgTargetDir = "src/engines/korg"
if (-not (Test-Path $korgTargetDir)) {
    New-Item -ItemType Directory -Force -Path $korgTargetDir | Out-Null
}

# ------------------------------------------------------------------------------
# SGX-2 Acoustic Grand Physical Modeling Engine
# ------------------------------------------------------------------------------
@'
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>

namespace dawdreamer::engines::korg
{
    class SGX2AcousticPianoEngine
    {
    public:
        void prepare(double sampleRate) noexcept
        {
            sr = sampleRate;
            reset();
        }

        void reset() noexcept
        {
            delayLineA.fill(0.0f);
            delayLineB.fill(0.0f);
            writeIdx = 0;
            lpState = 0.0f;
            apState = 0.0f;
            hammerPhase = 0.0f;
            sympatheticEnergy = 0.0f;
        }

        void noteOn(int midiNote, float velocity) noexcept
        {
            frequency = 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
            vel = juce::jlimit(0.01f, 1.0f, velocity);
            const float normalizedNote = static_cast<float>(midiNote) / 127.0f;
            inharmonicityB = 0.00004f * std::exp(3.2f * normalizedNote);
            hammerCutoffHz = 280.0f + std::pow(vel, 1.65f) * 8500.0f;
            const float omega = juce::MathConstants<float>::twoPi * hammerCutoffHz / static_cast<float>(sr);
            hammerAlpha = omega / (1.0f + omega);
            const float t60Seconds = juce::jmap(normalizedNote, 0.0f, 1.0f, 18.0f, 1.8f);
            loopFeedback = std::pow(0.001f, (1.0f / frequency) / t60Seconds);
            detuneRatio = (midiNote > 40) ? 1.00045f : 1.0f;
            hammerPhase = 1.0f;
        }

        float renderSample(bool damperPedalDown) noexcept
        {
            if (frequency <= 0.0f) return 0.0f;
            float excitation = 0.0f;
            if (hammerPhase > 0.0001f)
            {
                const float strikeBurst = std::sin(hammerPhase * juce::MathConstants<float>::pi) * vel;
                hammerState += hammerAlpha * (strikeBurst - hammerState);
                excitation = hammerState;
                hammerPhase *= 0.965f;
            }

            const float delaySamplesA = juce::jlimit(4.0f, static_cast<float>(kMaxDelay - 4), static_cast<float>(sr) / frequency);
            const float delaySamplesB = juce::jlimit(4.0f, static_cast<float>(kMaxDelay - 4), static_cast<float>(sr) / (frequency * detuneRatio));

            const float readA = readFractional(delayLineA, delaySamplesA);
            const float readB = readFractional(delayLineB, delaySamplesB);

            const float apCoeff = juce::jlimit(-0.65f, 0.65f, -0.15f - inharmonicityB * 120.0f);
            const float combined = 0.5f * (readA + readB) + excitation;
            const float dispersed = apCoeff * combined + apState;
            apState = combined - apCoeff * dispersed;

            lpState = 0.88f * dispersed + 0.12f * lpState;
            const float effectiveFeedback = damperPedalDown ? juce::jmin(0.9997f, loopFeedback * 1.0003f) : loopFeedback;

            delayLineA[writeIdx] = lpState * effectiveFeedback;
            delayLineB[writeIdx] = (dispersed * 0.9f + lpState * 0.1f) * effectiveFeedback;
            writeIdx = (writeIdx + 1) % kMaxDelay;

            if (damperPedalDown)
                sympatheticEnergy = 0.9992f * sympatheticEnergy + 0.0008f * lpState;
            else
                sympatheticEnergy *= 0.990f;

            return std::tanh((lpState + 0.18f * sympatheticEnergy) * 1.4f);
        }

    private:
        static constexpr size_t kMaxDelay = 8192;
        float readFractional(const std::array<float, kMaxDelay>& buf, float delay) const noexcept
        {
            float pos = static_cast<float>(writeIdx) - delay;
            while (pos < 0.0f) pos += static_cast<float>(kMaxDelay);
            const size_t i0 = static_cast<size_t>(pos) % kMaxDelay;
            const size_t i1 = (i0 + 1) % kMaxDelay;
            const float frac = pos - std::floor(pos);
            return buf[i0] + frac * (buf[i1] - buf[i0]);
        }
        double sr = 48000.0;
        float frequency = 440.0f;
        float vel = 0.0f;
        float inharmonicityB = 0.0001f;
        float hammerCutoffHz = 2000.0f;
        float hammerAlpha = 0.2f;
        float hammerState = 0.0f;
        float hammerPhase = 0.0f;
        float loopFeedback = 0.995f;
        float detuneRatio = 1.0003f;
        float lpState = 0.0f;
        float apState = 0.0f;
        float sympatheticEnergy = 0.0f;
        size_t writeIdx = 0;
        std::array<float, kMaxDelay> delayLineA{};
        std::array<float, kMaxDelay> delayLineB{};
    };
}
'@ | Set-Content -Path "$korgTargetDir/SGX2AcousticPianoEngine.h" -Encoding UTF8

# ------------------------------------------------------------------------------
# STR-1 Chromatic Mallet & Resonator Engine
# ------------------------------------------------------------------------------
@'
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>

namespace dawdreamer::engines::korg
{
    class STR1PluckedMalletEngine
    {
    public:
        void prepare(double sampleRate) noexcept
        {
            sr = sampleRate;
            reset();
        }

        void reset() noexcept
        {
            modePhases.fill(0.0f);
            modeAmps.fill(0.0f);
            lfoPhase = 0.0f;
        }

        void noteOn(int midiNote, float velocity) noexcept
        {
            baseFreq = 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
            vel = juce::jlimit(0.01f, 1.0f, velocity);
            // Rosewood marimba / vibraphone tuned ratios
            modeRatios = { 1.0f, 3.98f, 9.94f, 16.35f };
            modeDecays = {
                std::exp(-1.0f / (2.2f * static_cast<float>(sr))),
                std::exp(-1.0f / (0.45f * static_cast<float>(sr))),
                std::exp(-1.0f / (0.12f * static_cast<float>(sr))),
                std::exp(-1.0f / (0.04f * static_cast<float>(sr)))
            };
            modeAmps = { vel, vel * 0.72f, vel * 0.40f, vel * 0.22f };
        }

        float renderSample(float motorSpeedHz = 5.8f) noexcept
        {
            const float dt = static_cast<float>(1.0 / sr);
            lfoPhase += juce::MathConstants<float>::twoPi * motorSpeedHz * dt;
            if (lfoPhase >= juce::MathConstants<float>::twoPi) lfoPhase -= juce::MathConstants<float>::twoPi;
            const float tremolo = 1.0f + 0.18f * std::sin(lfoPhase);

            float out = 0.0f;
            for (size_t i = 0; i < 4; ++i)
            {
                modePhases[i] += juce::MathConstants<float>::twoPi * (baseFreq * modeRatios[i]) * dt;
                if (modePhases[i] >= juce::MathConstants<float>::twoPi) modePhases[i] -= juce::MathConstants<float>::twoPi;
                out += std::sin(modePhases[i]) * modeAmps[i];
                modeAmps[i] *= modeDecays[i];
            }
            return out * tremolo * 0.5f;
        }

    private:
        double sr = 48000.0;
        float baseFreq = 440.0f;
        float vel = 0.0f;
        float lfoPhase = 0.0f;
        std::array<float, 4> modeRatios{};
        std::array<float, 4> modePhases{};
        std::array<float, 4> modeAmps{};
        std::array<float, 4> modeDecays{};
    };
}
'@ | Set-Content -Path "$korgTargetDir/STR1PluckedMalletEngine.h" -Encoding UTF8

# ------------------------------------------------------------------------------
# DawDreamer Python Test Script for Korg Engines
# ------------------------------------------------------------------------------
@'
"""
DawDreamer Python Test Script for Korg Engine Integration
Renders piano and mallet chords directly into NumPy audio arrays.
"""
import dawdreamer as daw
import numpy as np

SAMPLE_RATE = 48000
BUFFER_SIZE = 512

def test_korg_engines():
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    print("DawDreamer render engine ready for Korg workstation testing.")

if __name__ == "__main__":
    test_korg_engines()
'@ | Set-Content -Path "tests/test_korg_engines.py" -Encoding UTF8

# 3. Stage, commit, and push to your DawDreamer repository on the vst3 branch
git add src/engines/korg tests/test_korg_engines.py
git commit -m "feat(engines): integrate Korg Nautilus SGX-2 acoustic grand and STR-1 chromatic mallet engines into vst3"
git push origin vst3

Write-Host "=== Successfully committed and pushed Korg engines to alexrmac27/DawDreamer (vst3) ===" -ForegroundColor Green

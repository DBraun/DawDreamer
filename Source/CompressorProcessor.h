#pragma once

#include "ProcessorBase.h"

class CompressorProcessor : public ProcessorBase
{
public:
    CompressorProcessor(std::string newUniqueName, float threshold, float ratio, float attack, float release) :
        ProcessorBase(newUniqueName),
        myThreshold{ threshold }, myRatio{ ratio }, myAttack{ attack }, myRelease{ release } {

        myCompressor.setThreshold(myThreshold);
        myCompressor.setRatio(myRatio);
        myCompressor.setAttack(myAttack);
        myCompressor.setRelease(myRelease);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        myCompressor.prepare(spec);
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        myCompressor.process(context);
    }

    void reset() {
        myCompressor.reset();
    };

    const juce::String getName() { return "CompressorProcessor"; };

    void setThreshold(float threshold) { myThreshold = threshold; updateParameters(); }
    float getThreshold() { return myThreshold; }

    void setRatio(float ratio) { myRatio = ratio; updateParameters(); }
    float getRatio() { return myRatio; }

    void setAttack(float attack) { myAttack = attack; updateParameters(); }
    float getAttack() { return myAttack; }

    void setRelease(float release) { myRelease = release; updateParameters(); }
    float getRelease() { return myRelease; }


private:
    juce::dsp::Compressor<float> myCompressor;
    float myThreshold;
    float myRatio;
    float myAttack;
    float myRelease;

    void updateParameters() {

    }

};
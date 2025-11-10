#pragma once

#include "custom_pybind_wrappers.h"
#include "ProcessorBase.h"

class PlaybackProcessor : public ProcessorBase
{
  public:
    PlaybackProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData)
        : ProcessorBase{newUniqueName}
    {
        m_numChannels = (int)inputData.size();
        auto numSamples = (int)inputData.at(0).size();
        myPlaybackData.setSize(m_numChannels, numSamples);
        for (int chan = 0; chan < m_numChannels; chan++)
        {
            myPlaybackData.copyFrom(chan, 0, inputData.at(chan).data(), numSamples);
        }

        setMainBusInputsAndOutputs(0, m_numChannels);
    }

    PlaybackProcessor(std::string newUniqueName, nb::ndarray<> input) : ProcessorBase{newUniqueName}
    {
        setData(input);
    }

    void prepareToPlay(double, int) override {}

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override
    {
        auto posInfo = getPlayHead()->getPosition();

        buffer.clear();

        int numSamples = std::min(buffer.getNumSamples(), myPlaybackData.getNumSamples() -
                                                              (int)(*posInfo->getTimeInSamples()));

        for (int chan = 0; chan < m_numChannels; chan++)
        {
            auto srcPtr = myPlaybackData.getReadPointer(chan);
            srcPtr += (int)*posInfo->getTimeInSamples();
            buffer.copyFrom(chan, 0, srcPtr, numSamples);
        }
        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    void reset() override { ProcessorBase::reset(); }

    const juce::String getName() const override { return "PlaybackProcessor"; }

    void setData(nb::ndarray<> input)
    {
        m_numChannels = (int)input.shape(0);
        auto numSamples = (int)input.shape(1);

        myPlaybackData.setSize(m_numChannels, numSamples);

        // Use byte-based addressing to handle different dtypes
        uint8_t* input_ptr = (uint8_t*)input.data();
        int64_t chan_stride_bytes = input.stride(0) * input.itemsize();
        int64_t sample_stride_bytes = input.stride(1) * input.itemsize();
        bool is_float64 = (input.dtype() == nb::dtype<double>());

        for (int chan = 0; chan < m_numChannels; chan++)
        {
            for (int samp = 0; samp < numSamples; samp++)
            {
                uint8_t* ptr = input_ptr + chan * chan_stride_bytes + samp * sample_stride_bytes;
                float val = is_float64 ? (float)(*(double*)ptr) : *(float*)ptr;
                myPlaybackData.setSample(chan, samp, val);
            }
        }

        setMainBusInputsAndOutputs(0, m_numChannels);
    }

  private:
    juce::AudioSampleBuffer myPlaybackData;
    int m_numChannels = 0;
};

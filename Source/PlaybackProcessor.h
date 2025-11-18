#pragma once

#include "custom_nanobind_wrappers.h"
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

    PlaybackProcessor(std::string newUniqueName, nb::ndarray<float> input)
        : ProcessorBase{newUniqueName}
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

    nb::dict getPickleState()
    {
        nb::dict state;
        state["unique_name"] = getUniqueName();
        state["audio_data"] = bufferToPyArray(myPlaybackData);
        return state;
    }

    void setPickleState(nb::dict state)
    {
        // Extract state
        std::string name = nb::cast<std::string>(state["unique_name"]);
        nb::ndarray<float> audio_data = nb::cast<nb::ndarray<float>>(state["audio_data"]);

        // Use placement new to construct the object in-place
        new (this) PlaybackProcessor(name, audio_data);
    }

    void setData(nb::ndarray<float> input)
    {
        float* input_ptr = (float*)input.data();

        m_numChannels = (int)input.shape(0);
        auto numSamples = (int)input.shape(1);

        myPlaybackData.setSize(m_numChannels, numSamples);

        // Get strides - nanobind returns ELEMENT strides, not byte strides
        size_t elem_stride_ch = input.stride(0);     // stride for channel dimension (in elements)
        size_t elem_stride_sample = input.stride(1); // stride for sample dimension (in elements)

        // Check if C-contiguous (row-major): channels x samples
        bool is_c_contiguous = (elem_stride_sample == 1 && elem_stride_ch == numSamples);

        if (is_c_contiguous)
        {
            // Fast path for C-contiguous arrays
            for (int chan = 0; chan < m_numChannels; chan++)
            {
                myPlaybackData.copyFrom(chan, 0, input_ptr, numSamples);
                input_ptr += numSamples;
            }
        }
        else
        {
            // General path using strides (handles F-contiguous and other layouts)
            for (int chan = 0; chan < m_numChannels; chan++)
            {
                float* chan_ptr = input_ptr + (chan * elem_stride_ch);
                float* dest = myPlaybackData.getWritePointer(chan);
                for (int samp = 0; samp < numSamples; samp++)
                {
                    dest[samp] = chan_ptr[samp * elem_stride_sample];
                }
            }
        }

        setMainBusInputsAndOutputs(0, m_numChannels);
    }

  private:
    juce::AudioSampleBuffer myPlaybackData;
    int m_numChannels = 0;
};

#pragma once

#include "ProcessorBase.h"
#include "custom_pybind_wrappers.h"

class PlaybackProcessor : public ProcessorBase {
 public:
  PlaybackProcessor(std::string newUniqueName,
                    std::vector<std::vector<float>> inputData)
      : ProcessorBase{newUniqueName} {
    m_numChannels = (int)inputData.size();
    auto numSamples = (int)inputData.at(0).size();
    myPlaybackData.setSize(m_numChannels, numSamples);
    for (int chan = 0; chan < m_numChannels; chan++) {
      myPlaybackData.copyFrom(chan, 0, inputData.at(chan).data(), numSamples);
    }

    setMainBusInputsAndOutputs(0, m_numChannels);
  }

  PlaybackProcessor(
      std::string newUniqueName,
      py::array_t<float, py::array::c_style | py::array::forcecast> input)
      : ProcessorBase{newUniqueName} {
    setData(input);
  }

  void prepareToPlay(double, int) override {}

  void processBlock(juce::AudioSampleBuffer& buffer,
                    juce::MidiBuffer& midiBuffer) override {
    auto posInfo = getPlayHead()->getPosition();

    buffer.clear();

    int numSamples = std::min(
        buffer.getNumSamples(),
        myPlaybackData.getNumSamples() - (int)(*posInfo->getTimeInSamples()));

    for (int chan = 0; chan < m_numChannels; chan++) {
      auto srcPtr = myPlaybackData.getReadPointer(chan);
      srcPtr += (int)*posInfo->getTimeInSamples();
      buffer.copyFrom(chan, 0, srcPtr, numSamples);
    }
    ProcessorBase::processBlock(buffer, midiBuffer);
  }

  void reset() override { ProcessorBase::reset(); }

  const juce::String getName() const override { return "PlaybackProcessor"; }

  void setData(
      py::array_t<float, py::array::c_style | py::array::forcecast> input) {
    float* input_ptr = (float*)input.data();

    m_numChannels = (int)input.shape(0);
    auto numSamples = (int)input.shape(1);

    myPlaybackData.setSize(m_numChannels, numSamples);

    for (int chan = 0; chan < m_numChannels; chan++) {
      myPlaybackData.copyFrom(chan, 0, input_ptr, numSamples);
      input_ptr += numSamples;
    }

    setMainBusInputsAndOutputs(0, m_numChannels);
  }

 private:
  juce::AudioSampleBuffer myPlaybackData;
  int m_numChannels = 0;
};

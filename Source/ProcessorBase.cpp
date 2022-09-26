#include "ProcessorBase.h"

void
ProcessorBase::numChannelsChanged() {    
    m_isConnectedInGraph = false;
}

void
ProcessorBase::getStateInformation(juce::MemoryBlock& destData)
{
}

void
ProcessorBase::setStateInformation(const void* data, int sizeInBytes)
{
}

bool ProcessorBase::setAutomation(std::string& parameterName, py::array input, std::uint32_t ppqn) {

    for (auto& uncastedParameter : this->getParameterTree().getParameters(true)) {
    if (uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH)
            .toStdString() == parameterName) {
          auto parameter = (AutomateParameterFloat*)
              uncastedParameter;
          parameter->setAutomation(input, ppqn);
          return true;
        }
	}

    throw std::runtime_error("Failed to set parameter: " + parameterName);
    return false;
}

bool ProcessorBase::setAutomationByIndex(int& index, py::array input,
                                  std::uint32_t ppqn) {

	auto parameters = this->getParameterTree().getParameters(true);
  if (index < 0 || index >= parameters.size()) {
    throw std::runtime_error("Failed to set automation for parameter at index " +
                             std::to_string(index));
  }

	auto parameter = (AutomateParameterFloat*)parameters.getUnchecked(index);
	parameter->setAutomation(input, ppqn);
	return true;
}

bool ProcessorBase::setAutomationVal(const char* parameterName, float val) {
  return setAutomationValByStr(std::string(parameterName), val);
}

bool ProcessorBase::setAutomationValByStr(std::string& parameterName, float val) {

	std::cout << "looking for param: " << parameterName << std::endl;
    for (auto& uncastedParameter : this->getParameterTree().getParameters(true)) {
		if (uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH)
				.toStdString() == parameterName) {
		  auto parameter = (AutomateParameterFloat*)
			  uncastedParameter;
		  parameter->setAutomation(val);
                  std::cout << "   and found it" << std::endl;
		  return true;
                } else {
                  std::cout << "   but found: "
                            << uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH)
                                   .toStdString()
                            << std::endl;        
		}
	}

  throw std::runtime_error("Failed to set parameter: " + parameterName);
  return false;
}

bool ProcessorBase::setAutomationValByIndex(int index, float val) {

	auto parameters = this->getParameterTree().getParameters(true);
  if (index < 0 || index >= parameters.size()) {
      throw std::runtime_error("Failed to set parameter at index " + std::to_string(index));
  }

	auto parameter = (AutomateParameterFloat*)parameters.getUnchecked(index);
  parameter->setAutomation(val);
  return true;
}

std::vector<float> ProcessorBase::getAutomation(std::string& parameterName) {

    for (auto& uncastedParameter : this->getParameterTree().getParameters(true)) {
    if (uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH)
            .toStdString() == parameterName) {
      auto parameter = (AutomateParameterFloat*) uncastedParameter;
      return parameter->getAutomation();
    }
  }

  throw std::runtime_error("Failed to get automation values for parameter: " +
                           parameterName);
}

std::vector<float> ProcessorBase::getAutomationByIndex(int& index) {

	auto parameters = this->getParameterTree().getParameters(true);
  if (index < 0 || index >= parameters.size()) {
    throw std::runtime_error("Failed to get automation for parameter at index " +
                             std::to_string(index));
  }

  auto parameter = (AutomateParameterFloat*)parameters.getUnchecked(index);
  return parameter->getAutomation();
}

float ProcessorBase::getAutomationVal(const char* parameterName,
	AudioPlayHead::PositionInfo& posInfo) {
  return getAutomationVal(std::string(parameterName), posInfo);
}

float ProcessorBase::getAutomationVal(std::string& parameterName, AudioPlayHead::PositionInfo& posInfo) {

    for (auto& uncastedParameter : this->getParameterTree().getParameters(true)) {
    if (uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString() == parameterName) {
      auto parameter = (AutomateParameterFloat*)
          uncastedParameter;  // todo: why do we have to cast to
                              // AutomateParameterFloat instead of
                              // AutomateParameter
      return parameter->sample(posInfo);
    }
  }

  throw std::runtime_error("Failed to get automation value for parameter: " +
                           parameterName);
}

float ProcessorBase::getAutomationAtZeroByIndex(int& index) {
  auto parameters = this->getParameters();

  if (index < 0 || index >= parameters.size()) {
    throw std::runtime_error("Failed to get automation value for parameter at index: " + index);
  }

    auto parameter = (AutomateParameterFloat*) parameters.getUnchecked(index);
    AudioPlayHead::PositionInfo posInfo;
    posInfo.setTimeInSamples(0.);
    posInfo.setTimeInSeconds(0.);
    return parameter->sample(posInfo);
}

float ProcessorBase::getAutomationAtZero(std::string parameterName) {

	auto parameters = this->getParameters();
  for (auto& uncastedParameter : parameters) {
    if (uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH)
            .toStdString() == parameterName) {
      auto parameter = (AutomateParameterFloat*)uncastedParameter;
      AudioPlayHead::PositionInfo posInfo;
      posInfo.setTimeInSamples(0.);
      posInfo.setTimeInSeconds(0.);
      return parameter->sample(posInfo);
    }
  }

throw std::runtime_error("Failed to get automation value for parameter: " +
                           parameterName);
}

py::array_t<float> ProcessorBase::getAutomationNumpy(std::string& parameterName) {
    std::vector<float> data = getAutomation(parameterName);

    py::array_t<float, py::array::c_style> arr({ (int)data.size() });

    auto ra = arr.mutable_unchecked();

    for (size_t i = 0; i < data.size(); i++)
    {
        ra(i) = data[i];
    }

    return arr;
}


py::dict ProcessorBase::getAutomationAll() {
    
    py::dict outDict;
    
    for (auto it = m_recordedAutomationDict.begin(); it != m_recordedAutomationDict.end(); it++)
    {
        outDict[it->first.c_str()] = this->bufferToPyArray(it->second);
    }

    return outDict;
}


void ProcessorBase::recordAutomation(AudioPlayHead::PositionInfo& posInfo, int numSamples) {
    
    if (m_recordAutomation) {
        const Array<AudioProcessorParameter*>& processorParams = this->getParameters();
        
        int j = 0;
        
        for (int i = 0; i < this->getNumParameters(); i++) {
            
            auto theParameter = (AutomateParameterFloat*)processorParams[i];
            
            // Note that we don't use label because it's sometimes blank. The same choice must be made in reset()
            std::string name = (processorParams[i])->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString();
            
            if (name.compare("") == 0) {
                continue;
            }
            
            auto val = theParameter->sample(posInfo);
            auto writePtr = m_recordedAutomationDict[name].getWritePointer(0, (int) (*posInfo.getTimeInSamples()));
            
            for (j=0; j < numSamples; j++) {
                *writePtr++ = val;
            }
        }
    }
}

void
ProcessorBase::reset() {
    m_recordedAutomationDict.clear();
    
    const Array<AudioProcessorParameter*>& processorParams = this->getParameters();
    
    for (int i = 0; i < this->getNumParameters(); i++) {
        // Note that we don't use label because it's sometimes blank. The same choice must be made in recordAutomation()
        std::string name = (processorParams[i])->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString();
        if (name.compare("") == 0) {
            continue;
        }
        juce::AudioSampleBuffer buffer;
        buffer.setSize(1, m_recordAutomation ? m_expectedRecordNumSamples : 0);
        
        m_recordedAutomationDict[name] = buffer;
    }
}

void
ProcessorBase::processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) {

    if (!m_recordEnable) {
        return;
    }
    auto posInfo = getPlayHead()->getPosition();

    const int numberChannels = myRecordBuffer.getNumChannels();
    int numSamplesToCopy = std::min(buffer.getNumSamples(), myRecordBuffer.getNumSamples() -int(*posInfo->getTimeInSamples()));
    int writePos = int(*posInfo->getTimeInSamples());

    for (int chan = 0; chan < numberChannels; chan++) {
        // Write the sample to the engine's history for the correct channel.
        myRecordBuffer.copyFrom(chan, writePos, buffer.getReadPointer(chan), numSamplesToCopy);
    }
}


py::array_t<float>
ProcessorBase::bufferToPyArray(juce::AudioSampleBuffer& buffer) {
    size_t num_channels = buffer.getNumChannels();
    size_t num_samples = buffer.getNumSamples();

    py::array_t<float, py::array::c_style> arr({ (int)num_channels, (int)num_samples });

    auto ra = arr.mutable_unchecked();

    auto chans = buffer.getArrayOfReadPointers();
    for (size_t i = 0; i < num_channels; i++)
    {
        auto chanPtr = chans[i];

        for (size_t j = 0; j < num_samples; j++)
        {
            ra(i, j) = *(chanPtr++);
        }
    }

    return arr;
}

py::array_t<float>
ProcessorBase::getAudioFrames()
{
    return bufferToPyArray(myRecordBuffer);
}

void
ProcessorBase::setRecorderLength(int numSamples) {
    
    m_expectedRecordNumSamples = numSamples;
    
    int numChannels = this->getTotalNumOutputChannels();
    
    myRecordBuffer.setSize(numChannels, m_recordEnable ? numSamples : 0);
    
}

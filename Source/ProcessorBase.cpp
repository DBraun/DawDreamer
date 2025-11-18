#include "ProcessorBase.h"

void ProcessorBase::numChannelsChanged()
{
    m_isConnectedInGraph = false;
}

void ProcessorBase::getStateInformation(juce::MemoryBlock& destData) {}

void ProcessorBase::setStateInformation(const void* data, int sizeInBytes) {}

bool ProcessorBase::setAutomation(std::string& parameterName, nb::ndarray<float> input,
                                  std::uint32_t ppqn)
{
    for (auto& uncastedParameter : this->getParameters())
    {
        if (uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString() ==
            parameterName)
        {
            auto parameter = static_cast<AutomateParameterFloat*>(uncastedParameter);
            parameter->setAutomation(input, ppqn);
            return true;
        }
    }

    throw std::runtime_error("Failed to set parameter: " + parameterName);
    return false;
}

bool ProcessorBase::setAutomationByIndex(int& index, nb::ndarray<float> input, std::uint32_t ppqn)
{
    auto parameters = this->getParameters();
    if (index < 0 || index >= parameters.size())
    {
        throw std::runtime_error("Failed to set automation for parameter at index " +
                                 std::to_string(index));
    }

    auto parameter = static_cast<AutomateParameterFloat*>(parameters.getUnchecked(index));
    parameter->setAutomation(input, ppqn);
    return true;
}

bool ProcessorBase::setAutomationVal(const char* parameterName, float val)
{
    auto s = std::string(parameterName);
    return setAutomationValByStr(s, val);
}

bool ProcessorBase::setAutomationValByStr(std::string& parameterName, float val)
{
    for (auto& uncastedParameter : this->getParameters())
    {
        if (uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString() ==
            parameterName)
        {
            auto parameter = static_cast<AutomateParameterFloat*>(uncastedParameter);
            parameter->setAutomation(val);
            return true;
        }
    }

    throw std::runtime_error("Failed to set parameter: " + parameterName);
    return false;
}

bool ProcessorBase::setAutomationValByIndex(int index, float val)
{
    auto parameters = this->getParameters();
    if (index < 0 || index >= parameters.size())
    {
        throw std::runtime_error("Failed to set parameter at index " + std::to_string(index));
    }

    auto parameter = static_cast<AutomateParameterFloat*>(parameters.getUnchecked(index));
    parameter->setAutomation(val);
    return true;
}

std::vector<float> ProcessorBase::getAutomation(const std::string& parameterName)
{
    for (auto& uncastedParameter : this->getParameters())
    {
        if (uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString() ==
            parameterName)
        {
            auto parameter = static_cast<AutomateParameterFloat*>(uncastedParameter);
            return parameter->getAutomation();
        }
    }

    throw std::runtime_error("Failed to get automation values for parameter: " + parameterName);
}

std::vector<float> ProcessorBase::getAutomationByIndex(const int& index)
{
    auto parameters = this->getParameters();
    if (index < 0 || index >= parameters.size())
    {
        throw std::runtime_error("Failed to get automation for parameter at index " +
                                 std::to_string(index));
    }

    auto parameter = static_cast<AutomateParameterFloat*>(parameters.getUnchecked(index));
    return parameter->getAutomation();
}

float ProcessorBase::getAutomationVal(const char* parameterName,
                                      AudioPlayHead::PositionInfo& posInfo)
{
    std::string s = std::string(parameterName);
    return getAutomationVal(s, posInfo);
}

float ProcessorBase::getAutomationVal(const std::string& parameterName,
                                      AudioPlayHead::PositionInfo& posInfo)
{
    for (auto& uncastedParameter : this->getParameters())
    {
        if (uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString() ==
            parameterName)
        {
            auto parameter = static_cast<AutomateParameterFloat*>(
                uncastedParameter); // todo: why do we have to cast to
                                    // AutomateParameterFloat instead of
                                    // AutomateParameter
            return parameter->sample(posInfo);
        }
    }

    throw std::runtime_error("Failed to get automation value for parameter: " + parameterName);
}

float ProcessorBase::getAutomationAtZeroByIndex(const int& index) const
{
    auto parameters = this->getParameters();

    if (index < 0 || index >= parameters.size())
    {
        throw std::runtime_error("Failed to get automation value for parameter at index: " +
                                 std::to_string(index));
    }

    auto parameter = static_cast<AutomateParameterFloat*>(parameters.getUnchecked(index));
    AudioPlayHead::PositionInfo posInfo;
    posInfo.setTimeInSamples(0.);
    posInfo.setTimeInSeconds(0.);
    return parameter->sample(posInfo);
}

float ProcessorBase::getAutomationAtZero(const std::string& parameterName) const
{
    auto parameters = this->getParameters();
    for (auto& uncastedParameter : parameters)
    {
        if (uncastedParameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString() ==
            parameterName)
        {
            auto parameter = static_cast<AutomateParameterFloat*>(uncastedParameter);
            AudioPlayHead::PositionInfo posInfo;
            posInfo.setTimeInSamples(0.);
            posInfo.setTimeInSeconds(0.);
            return parameter->sample(posInfo);
        }
    }

    throw std::runtime_error("Failed to get automation value for parameter: " + parameterName);
}

nb::ndarray<nb::numpy, float> ProcessorBase::getAutomationNumpy(const std::string& parameterName)
{
    std::vector<float> data = getAutomation(parameterName);

    size_t size = data.size();
    size_t shape[1] = {size};
    float* array_data = new float[size];

    std::memcpy(array_data, data.data(), size * sizeof(float));

    auto capsule =
        nb::capsule(array_data, [](void* p) noexcept { delete[] static_cast<float*>(p); });

    return nb::ndarray<nb::numpy, float>(array_data, 1, shape, capsule);
}

nb::dict ProcessorBase::getAutomationAll()
{
    nb::dict outDict;

    for (auto it = m_recordedAutomationDict.begin(); it != m_recordedAutomationDict.end(); it++)
    {
        outDict[it->first.c_str()] = this->bufferToPyArray(it->second);
    }

    return outDict;
}

void ProcessorBase::recordAutomation(AudioPlayHead::PositionInfo& posInfo, int numSamples)
{
    if (m_recordAutomation)
    {
        const Array<AudioProcessorParameter*>& processorParams = this->getParameters();

        int j = 0;

        for (int i = 0; i < this->getNumParameters(); i++)
        {
            auto theParameter = static_cast<AutomateParameterFloat*>(processorParams[i]);

            // Note that we don't use label because it's sometimes blank. The same
            // choice must be made in reset()
            std::string name =
                (processorParams[i])->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString();

            if (name.compare("") == 0)
            {
                continue;
            }

            float val = theParameter->sample(posInfo);
            j = (int)(*posInfo.getTimeInSamples());
            int jMax = std::min(j + numSamples, m_recordedAutomationDict[name].getNumSamples());
            float* writePtr = m_recordedAutomationDict[name].getWritePointer(0, j);
            for (; j < jMax; j++)
            {
                *writePtr++ = val;
            }
        }
    }
}

void ProcessorBase::reset()
{
    m_recordedAutomationDict.clear();

    int i = 0;
    for (juce::AudioProcessorParameter* parameter : this->getParameters())
    {
        // Note that we don't use label because it's sometimes blank. The same
        // choice must be made in recordAutomation()
        std::string name = parameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString();
        if (name.empty())
        {
            // todo: surface this as warning
            // std::cerr << "Warning: parameter name at index " << i
            //           << " has an empty name." << std::endl;
            i++;
            continue;
        }
        juce::AudioSampleBuffer buffer;
        buffer.setSize(1, m_recordAutomation ? m_expectedRecordNumSamples : 0);

        m_recordedAutomationDict[name] = buffer;
        i++;
    }
}

void ProcessorBase::processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
{
    if (!m_recordEnable)
    {
        return;
    }
    auto posInfo = getPlayHead()->getPosition();
    const bool isRecording = posInfo->getIsRecording();
    if (!isRecording)
    {
        return;
    }

    const int numberChannels = myRecordBuffer.getNumChannels();
    int numSamplesToCopy = std::min(buffer.getNumSamples(), myRecordBuffer.getNumSamples() -
                                                                int(*posInfo->getTimeInSamples()));
    int writePos = int(*posInfo->getTimeInSamples());

    for (int chan = 0; chan < numberChannels; chan++)
    {
        // Write the sample to the engine's history for the correct channel.
        myRecordBuffer.copyFrom(chan, writePos, buffer.getReadPointer(chan), numSamplesToCopy);
    }
}

nb::ndarray<nb::numpy, float> ProcessorBase::bufferToPyArray(juce::AudioSampleBuffer& buffer)
{
    size_t num_channels = buffer.getNumChannels();
    size_t num_samples = buffer.getNumSamples();

    size_t shape[2] = {num_channels, num_samples};
    float* array_data = new float[num_channels * num_samples];

    auto chans = buffer.getArrayOfReadPointers();
    for (size_t i = 0; i < num_channels; i++)
    {
        auto chanPtr = chans[i];
        float* row = array_data + (i * num_samples);

        for (size_t j = 0; j < num_samples; j++)
        {
            row[j] = *(chanPtr++);
        }
    }

    auto capsule =
        nb::capsule(array_data, [](void* p) noexcept { delete[] static_cast<float*>(p); });

    return nb::ndarray<nb::numpy, float>(array_data, 2, shape, capsule);
}

nb::ndarray<nb::numpy, float> ProcessorBase::getAudioFrames()
{
    return bufferToPyArray(myRecordBuffer);
}

void ProcessorBase::setRecorderLength(int numSamples)
{
    m_expectedRecordNumSamples = numSamples;

    int numChannels = this->getTotalNumOutputChannels();

    myRecordBuffer.setSize(numChannels, m_recordEnable ? numSamples : 0);
}

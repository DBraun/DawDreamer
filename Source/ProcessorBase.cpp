#include "ProcessorBase.h"

void
ProcessorBase::numChannelsChanged() {    
    m_isConnectedInGraph = false;
}

void
ProcessorBase::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = myParameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void
ProcessorBase::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(myParameters.state.getType()))
            myParameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

bool ProcessorBase::setAutomation(std::string parameterName, py::array input, std::uint32_t ppqn) {

    try
    {
        auto parameter = (AutomateParameterFloat*)myParameters.getParameter(parameterName);  // todo: why do we have to cast to AutomateParameterFloat instead of AutomateParameter

        if (parameter) {
            return parameter->setAutomation(input, ppqn);
        }
        else {
            throw std::runtime_error("Failed to find parameter: " + parameterName);
        }
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to set '" + parameterName + "' automation: " + e.what());
    }

    return true;
}

bool ProcessorBase::setAutomationVal(std::string parameterName, float val) {

    try
    {
        auto parameter = (AutomateParameterFloat*)myParameters.getParameter(parameterName);  // todo: why do we have to cast to AutomateParameterFloat instead of AutomateParameter
        if (parameter) {
            parameter->setAutomation(val);
        }
        else {
            throw std::runtime_error("Failed to find parameter: " + parameterName);
        }
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Failed to set '" + parameterName + "' automation: " + e.what());
    }

    return true;
}

std::vector<float> ProcessorBase::getAutomation(std::string parameterName) {
    auto parameter = (AutomateParameterFloat*)myParameters.getParameter(parameterName);  // todo: why do we have to cast to AutomateParameterFloat instead of AutomateParameter

    if (parameter) {
        return parameter->getAutomation();
    }
    else {
        throw std::runtime_error("Failed to get automation values for parameter: " + parameterName);
    }
}

float ProcessorBase::getAutomationVal(std::string parameterName, AudioPlayHead::PositionInfo& posInfo) {
    auto parameter = (AutomateParameterFloat*)myParameters.getParameter(parameterName);  // todo: why do we have to cast to AutomateParameterFloat instead of AutomateParameter

    if (parameter) {
        return parameter->sample(posInfo);
    }
    else {
        throw std::runtime_error("Failed to get automation value for parameter: " + parameterName);
    }
}

float ProcessorBase::getAutomationAtZero(std::string parameterName) {
    auto parameter = (AutomateParameterFloat*)myParameters.getParameter(parameterName);  // todo: why do we have to cast to AutomateParameterFloat instead of AutomateParameter

    if (parameter) {
        AudioPlayHead::PositionInfo posInfo;
        posInfo.setTimeInSamples(0.);
        posInfo.setTimeInSeconds(0.);
        return parameter->sample(posInfo);
    }
    else {
        throw std::runtime_error("Failed to get automation value for parameter: " + parameterName);
    }
}

py::array_t<float> ProcessorBase::getAutomationNumpy(std::string parameterName) {
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

        int maximumStringLength = 64;
        
        int j = 0;
        
        for (int i = 0; i < this->getNumParameters(); i++) {
            
            auto theParameter = (AutomateParameterFloat*)processorParams[i];
            
            // Note that we don't use label because it's sometimes blank. The same choice must be made in reset()
            std::string name = (processorParams[i])->getName(maximumStringLength).toStdString();
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
    
    int maximumStringLength = 64;
    for (int i = 0; i < this->getNumParameters(); i++) {
        // Note that we don't use label because it's sometimes blank. The same choice must be made in recordAutomation()
        std::string name = (processorParams[i])->getName(maximumStringLength).toStdString();
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

    for (int chan = 0; chan < numberChannels; chan++) {
        // Write the sample to the engine's history for the correct channel.
        myRecordBuffer.copyFrom(chan, int(*posInfo->getTimeInSamples()), buffer.getReadPointer(chan), numSamplesToCopy);
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

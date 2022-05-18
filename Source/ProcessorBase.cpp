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

float ProcessorBase::getAutomationVal(std::string parameterName, AudioPlayHead::CurrentPositionInfo& posInfo) {
    auto parameter = (AutomateParameterFloat*)myParameters.getParameter(parameterName);  // todo: why do we have to cast to AutomateParameterFloat instead of AutomateParameter

    if (parameter) {
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

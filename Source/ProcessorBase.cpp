#include "ProcessorBase.h"

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

bool ProcessorBase::setAutomation(std::string parameterName, py::array input) {

    try
    {
        auto parameter = (AutomateParameterFloat*)myParameters.getParameter(parameterName);  // todo: why do we have to cast to AutomateParameterFloat instead of AutomateParameter

        if (parameter) {
            return parameter->setAutomation(input);
        }
        else {
            std::cout << "Failed to find parameter: " << parameterName << std::endl;
            return false;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Failed to set '" << parameterName << "' automation: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool ProcessorBase::setParameter(std::string parameterName, float val) {

    try
    {
        auto parameter = (AutomateParameterFloat*)myParameters.getParameter(parameterName);  // todo: why do we have to cast to AutomateParameterFloat instead of AutomateParameter
        if (parameter) {
            parameter->setAutomation(val);
        }
        else {
            std::cout << "Failed to find parameter: " << parameterName << std::endl;
            return false;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Failed to set '" << parameterName << "' automation: " << e.what() << std::endl;
        return false;
    }

    return true;
}

std::vector<float> ProcessorBase::getAutomation(std::string parameterName, int maxSamples) {
    auto parameter = (AutomateParameter*)myParameters.getParameter(parameterName);

    return parameter->getAutomation();
}
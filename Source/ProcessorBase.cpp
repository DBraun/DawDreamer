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

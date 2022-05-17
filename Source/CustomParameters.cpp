#include "CustomParameters.h"
#include "ProcessorBase.h"

#include <algorithm>

bool
AutomateParameter::setAutomation(py::array_t<float> input, bool is_audio_rate) {

    m_is_audio_rate = is_audio_rate;

    try
    {
        myAutomation.clear();

        auto numSamples = input.shape(0);

        myAutomation = std::vector<float>(numSamples, 0.f);

        memcpy(myAutomation.data(), (float*)input.data(), numSamples * sizeof(float));
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("Error: setAutomation: ") + e.what());
        return false;
    }

    return true;
}

void
AutomateParameter::setAutomation(const float val) {
    myAutomation.clear();
    myAutomation.push_back(val);
}

std::vector<float>
AutomateParameter::getAutomation() {
    return myAutomation;
}

float
AutomateParameter::sample(juce::AudioPlayHead::CurrentPositionInfo& posInfo) {

    size_t i = std::min(myAutomation.size() - 1,
        size_t(this->m_is_audio_rate ?
            posInfo.timeInSamples :
            posInfo.ppqPosition * ProcessorBase::PPQN));

    i = std::max((size_t)0, i);
    return myAutomation.at(i);

}
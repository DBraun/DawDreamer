#include "CustomParameters.h"
#include "ProcessorBase.h"

#include <algorithm>

bool
AutomateParameter::setAutomation(py::array_t<float> input, std::uint32_t newPPQN) {

    if (newPPQN < 0) {
        throw std::runtime_error("The PPQN must be greater than or equal to zero. Received: " + std::to_string(newPPQN));
    }

    m_ppqn = newPPQN;

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

    size_t i;
    auto numSamples = myAutomation.size();
    if (numSamples == 0) {
        throw std::runtime_error("Can't sample parameter with no samples.");
    }
    if (m_ppqn > 0) {
        i = std::min(numSamples - 1, size_t(posInfo.ppqPosition * m_ppqn));
    }
    else {
        i = std::min(numSamples - 1, size_t(posInfo.timeInSamples));
    }

    i = std::max((size_t)0, i);
    return myAutomation.at(i);

}

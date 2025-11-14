#include "CustomParameters.h"

#include <algorithm>

#include "ProcessorBase.h"

bool AutomateParameter::setAutomation(nb::ndarray<float> input, std::uint32_t newPPQN)
{
    if (newPPQN < 0)
    {
        throw std::runtime_error("The PPQN must be greater than or equal to zero. Received: " +
                                 std::to_string(newPPQN));
    }

    m_ppqn = newPPQN;

    try
    {
        auto numSamples = input.shape(0);

        myAutomation = std::vector<float>(numSamples, 0.f);

        memcpy(myAutomation.data(), input.data(), numSamples * sizeof(float));
        m_hasAutomation = numSamples > 1;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("Error: setAutomation: ") + e.what());
        return false;
    }

    return true;
}

void AutomateParameter::setAutomation(const float val)
{
    myAutomation.clear();
    myAutomation.push_back(val);
    m_hasAutomation = false;
}

std::vector<float> AutomateParameter::getAutomation()
{
    return myAutomation;
}

float AutomateParameter::sample(AudioPlayHead::PositionInfo& posInfo)
{
    size_t i;
    auto numSamples = myAutomation.size();
    if (numSamples == 0)
    {
        throw std::runtime_error("Can't sample parameter with no samples.");
    }
    if (m_ppqn > 0)
    {
        i = std::min(numSamples - 1, size_t(*posInfo.getPpqPosition() * m_ppqn));
    }
    else
    {
        i = std::min(numSamples - 1, size_t(*posInfo.getTimeInSamples()));
    }

    i = std::max((size_t)0, i);
    return myAutomation.at(i);
}

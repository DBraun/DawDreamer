#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AutomateParameter
{

public:

    AutomateParameter() {
        //std::cout << "init AutomateParameter()" << std::endl;
    }

    void setAutomation(std::vector<float> automation) {
        myAutomation = automation;
    }

    void setAutomation(float automation) {
        myAutomation = { automation };
    }

    std::vector<float> getAutomation() {
        return myAutomation;
    }

    float sample(size_t index) {
        //std::cout << " sample: " << std::endl;
        //std::cout << " sample! " << myAutomation.size() << std::endl;
        auto i = std::min(myAutomation.size() - 1, index);
        i = std::max((size_t)0, i);
        return myAutomation.at(i);
    }

//private:

    std::vector<float> myAutomation;

    ~AutomateParameter()
    {
    }

};

class AutomateParameterFloat : public AutomateParameter, public AudioParameterFloat {

public:
    AutomateParameterFloat(const String& parameterID,
        const String& parameterName,
        NormalisableRange<float> normalisableRange,
        float defaultValue,
        const String& parameterLabel = juce::String(),
        Category parameterCategory = AudioProcessorParameter::genericParameter,
        std::function<String(float value, int maximumStringLength)> stringFromValue = nullptr,
        std::function<float(const String& text)> valueFromString = nullptr) :

        AutomateParameter(), AudioParameterFloat(parameterID,
            parameterName,
            normalisableRange,
            defaultValue,
            parameterLabel,
            parameterCategory,
            stringFromValue,
            valueFromString) {
        //std::cout << "init AutomateParameter() 1" << std::endl;
    }

    AutomateParameterFloat(String parameterID,
        String parameterName,
        float minValue,
        float maxValue,
        float defaultValue) :
        AutomateParameter(), AudioParameterFloat(
            parameterID,
            parameterName,
            minValue,
            maxValue,
            defaultValue
        ) {
        //std::cout << "init AutomateParameter() 2" << std::endl;
    }

};
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AutomateParameter
{

public:

    AutomateParameter() {}

    bool setAutomation(py::array input) {

        try
        {
            float* input_ptr = (float*)input.data();
            myAutomation.clear();

            myAutomation = std::vector<float>(input.shape(0), 0.f);

            for (int x = 0; x < input.shape(0); x++) {
                myAutomation[x] = *(input_ptr++);
            }

        }
        catch (const std::exception& e)
        {
            std::cout << "Error: setAutomation: " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    void setAutomation(const float val) {
        myAutomation.clear();
        myAutomation.push_back(val);

    }

    std::vector<float> getAutomation() {
        return myAutomation;
    }

    float sample(size_t index) {
        auto i = std::min(myAutomation.size() - 1, index);
        i = std::max((size_t)0, i);
        return myAutomation.at(i);
    }

protected:

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
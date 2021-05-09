#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

using juce::ADSR;
using juce::AbstractFifo;
using juce::AbstractFifo;
using juce::AlertWindow;
using juce::Array;
using juce::AudioBuffer;
using juce::AudioChannelSet;
using juce::AudioFormatManager;
using juce::AudioFormatReader;
using juce::AudioParameterBool;
using juce::AudioParameterChoice;
using juce::AudioParameterFloat;
using juce::AudioParameterInt;
using juce::AudioPlayHead;
using juce::AudioPluginFormat;
using juce::AudioPluginFormatManager;
using juce::AudioPluginInstance;
using juce::AudioProcessor;
using juce::AudioProcessorEditor;
using juce::AudioProcessorEditorListener;
using juce::AudioProcessorGraph;
using juce::AudioProcessorParameter;
using juce::AudioProcessorValueTreeState;
using juce::AudioSampleBuffer;
using juce::AudioThumbnail;
using juce::AudioThumbnailCache;
using juce::Base64;
using juce::Button;
using juce::CachedValue;
using juce::ChangeBroadcaster;
using juce::ChangeListener;
using juce::CodeEditorComponent;
using juce::Colour;
using juce::ColourGradient;
using juce::ComboBox;
using juce::Component;
using juce::Drawable;
using juce::File;
using juce::FileBrowserComponent;
using juce::FileChooser;
using juce::FileDragAndDropTarget;
using juce::FileInputStream;
using juce::GenericScopedTryLock;
using juce::Graphics;
using juce::Identifier;
using juce::Image;
using juce::ImageCache;
using juce::ImageFileFormat;
using juce::InputStream;
using juce::InputStream;
using juce::Justification;
using juce::KeyPress;
using juce::KnownPluginList;
using juce::Label;
using juce::LagrangeInterpolator;
using juce::Line;
using juce::ListenerList;
using juce::LocalisedStrings;
using juce::LookAndFeel;
using juce::LookAndFeel_V4;
using juce::MPENote;
using juce::MPESynthesiser;
using juce::MPESynthesiserBase;
using juce::MPESynthesiserVoice;
using juce::MPEZoneLayout;
using juce::MemoryBlock;
using juce::MemoryInputStream;
using juce::MemoryOutputStream;
using juce::MidiBuffer;
using juce::MidiBufferIterator;
using juce::MidiFile;
using juce::MidiMessage;
using juce::MidiMessageSequence;
using juce::ModifierKeys;
using juce::MouseCursor;
using juce::MouseEvent;
using juce::MouseListener;
using juce::NormalisableRange;
using juce::NotificationType;
using juce::OwnedArray;
using juce::Path;
using juce::PluginDescription;
using juce::Point;
using juce::Random;
using juce::Range;
using juce::RangedAudioParameter;
//using juce::Rectangle;
using juce::RectangleList;
using juce::ReferenceCountedArray;
using juce::ReferenceCountedObject;
using juce::ReferenceCountedObjectPtr;
using juce::Slider;
using juce::SmoothedValue;
using juce::SpinLock;
using juce::String;
using juce::StringArray;
using juce::TabbedButtonBar;
using juce::TabbedComponent;
using juce::TextButton;
using juce::Time;
using juce::Timer;
using juce::ToggleButton;
using juce::UndoManager;
using juce::VST3PluginFormat;
using juce::VSTPluginFormat;
using juce::ValueTree;
using juce::WildcardFileFilter;
using juce::dontSendNotification;
using juce::ignoreUnused;
using juce::int64;
using juce::jmax;
using juce::jmin;
using juce::roundFloatToInt;
using juce::roundToInt;
using juce::uint8;

class AutomateParameter
{

public:

    AutomateParameter() {}

    bool setAutomation(py::array_t<float> input) {

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
            valueFromString) {}

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
        ) {}

};
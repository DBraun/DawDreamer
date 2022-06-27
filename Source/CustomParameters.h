#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "custom_pybind_wrappers.h"

using juce::ADSR;
using juce::AbstractFifo;
using juce::AbstractFifo;
using juce::AlertWindow;
using juce::Array;
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
using juce::Base64;
using juce::Button;
using juce::CachedValue;
using juce::ChangeBroadcaster;
using juce::ChangeListener;
using juce::CodeEditorComponent;
using juce::Colour;
using juce::ColourGradient;
using juce::ComboBox;
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
using juce::Random;
using juce::Range;
using juce::RangedAudioParameter;
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

    bool setAutomation(py::array_t<float> input, std::uint32_t newPPQN);

    void setAutomation(const float val);

    std::vector<float> getAutomation();

    float sample(AudioPlayHead::PositionInfo& posInfo);

    ~AutomateParameter() {}

protected:

    std::vector<float> myAutomation;
    std::uint32_t m_ppqn = 0;

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

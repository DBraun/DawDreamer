#pragma once

#include "PluginProcessor.h"

#include <filesystem>

PluginProcessor::PluginProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock, std::string path) : ProcessorBase{ newUniqueName }
{
    myPluginPath = path;

    loadPlugin(sampleRate, samplesPerBlock);
}

bool
PluginProcessor::loadPlugin(double sampleRate, int samplesPerBlock) {
    OwnedArray<PluginDescription> pluginDescriptions;
    KnownPluginList pluginList;
    AudioPluginFormatManager pluginFormatManager;

    pluginFormatManager.addDefaultFormats();

    for (int i = pluginFormatManager.getNumFormats(); --i >= 0;)
    {
        pluginList.scanAndAddFile(String(myPluginPath),
            true,
            pluginDescriptions,
            *pluginFormatManager.getFormat(i));
    }

    if (myPlugin.get())
    {
        myPlugin->releaseResources();
        myPlugin.release();
    }

    // If there is a problem here first check the preprocessor definitions
    // in the projucer are sensible - is it set up to scan for plugin's?
    if (pluginDescriptions.size() <= 0) {
        std::cerr << "Unable to load plugin. The path should be absolute.\n";
        return false;
    }

    String errorMessage;

    myPlugin = pluginFormatManager.createPluginInstance(*pluginDescriptions[0],
        sampleRate,
        samplesPerBlock,
        errorMessage);

    if (myPlugin.get() != nullptr)
    {
        // Success so set up plugin, then set up features and get all available
        // parameters from this given plugin.
        
        // For VST3 on macOS, it seems that the main bus isn't setup,
        //   but if we run enableAllBuses on VST2 it breaks them...
        if (myPlugin->getMainBusNumInputChannels() == 0 && myPlugin->getMainBusNumOutputChannels() == 0) {
            myPlugin->enableAllBuses();
        }
  
        auto inputs = myPlugin->getTotalNumInputChannels();
        auto outputs = myPlugin->getTotalNumOutputChannels();
        myPlugin->setPlayConfigDetails(inputs, outputs, sampleRate, samplesPerBlock);
        this->setPlayConfigDetails(inputs, outputs, sampleRate, samplesPerBlock);
        myPlugin->setNonRealtime(true);
        mySampleRate = sampleRate;
        
        createParameterLayout();

        return true;
    }

    std::cerr << "PluginProcessor::loadPlugin error: " << errorMessage.toStdString() << std::endl;
    return false;

}

PluginProcessor::~PluginProcessor() {
    if (myPlugin.get())
    {
        myPlugin->releaseResources();
        myPlugin.release();
    }
}

void PluginProcessor::setPlayHead(AudioPlayHead* newPlayHead)
{
    AudioProcessor::setPlayHead(newPlayHead);
    if (myPlugin.get()) {
        myPlugin->setPlayHead(newPlayHead);
    }
}

bool
PluginProcessor::canApplyBusesLayout(const juce::AudioProcessor::BusesLayout& layout) {

    if (!myPlugin.get()) {
        return false;
    }

    return myPlugin->checkBusesLayoutSupported(layout);
}

void
PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    if (myPlugin.get()) {
        myPlugin->prepareToPlay(sampleRate, samplesPerBlock);
    }
}

void
PluginProcessor::processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer)
{
    if (!myPlugin.get()) {
        buffer.clear();
        return;
    }
    
    AudioPlayHead::CurrentPositionInfo posInfo;
    getPlayHead()->getCurrentPosition(posInfo);

    automateParameters();

    long long int start = posInfo.timeInSamples;
    long long int end = start + buffer.getNumSamples();
    myIsMessageBetween = myMidiMessagePosition >= start && myMidiMessagePosition < end;
    do {
        if (myIsMessageBetween) {
            myRenderMidiBuffer.addEvent(myMidiMessage, int(myMidiMessagePosition - start));
            myMidiEventsDoRemain = myMidiIterator->getNextEvent(myMidiMessage, myMidiMessagePosition);
            myIsMessageBetween = myMidiMessagePosition >= start && myMidiMessagePosition < end;
        }
    } while (myIsMessageBetween && myMidiEventsDoRemain);

    myPlugin->processBlock(buffer, myRenderMidiBuffer);
    
    ProcessorBase::processBlock(buffer, midiBuffer);
}

void
PluginProcessor::automateParameters() {

    AudioPlayHead::CurrentPositionInfo posInfo;
    getPlayHead()->getCurrentPosition(posInfo);

    if (myPlugin.get()) {

        for (int i = 0; i < myPlugin->AudioProcessor::getNumParameters(); i++) {

            auto paramID = std::to_string(i);

            auto theParameter = ((AutomateParameterFloat*)myParameters.getParameter(paramID));
            if (theParameter) {
                // todo: change to setParameterNotifyingHost?
                myPlugin->setParameter(i, theParameter->sample(posInfo.timeInSamples));
            }
            else {
                std::cout << "Error automateParameters: " << myPlugin->getParameterName(i) << std::endl;
            }
        }
    }
}

void
PluginProcessor::reset()
{
    if (myPlugin.get()) {
        myPlugin->reset();
    }

    if (myMidiIterator) {
        delete myMidiIterator;
    }

    myMidiIterator = new MidiBuffer::Iterator(myMidiBuffer); // todo: deprecated.
    myMidiEventsDoRemain = myMidiIterator->getNextEvent(myMidiMessage, myMidiMessagePosition);
    myRenderMidiBuffer.clear();
}

#include <pluginterfaces/vst/ivstcomponent.h>
#include <public.sdk/source/common/memorystream.h>

bool setVST3PluginStateDirect(AudioPluginInstance* instance, const MemoryBlock& rawData)
{
    auto funknown = static_cast<Steinberg::FUnknown*> (instance->getPlatformSpecificData());
    Steinberg::Vst::IComponent* vstcomponent = nullptr;

    if (funknown->queryInterface(Steinberg::Vst::IComponent_iid, (void**)&vstcomponent) == 0
        && vstcomponent != nullptr)
    {
        void* data = (void*)rawData.getData();

        auto* memoryStream = new Steinberg::MemoryStream(data, (Steinberg::TSize)rawData.getSize());
        vstcomponent->setState(memoryStream);
        memoryStream->release();
        vstcomponent->release();

        return true;
    }

    return false;
    
}

bool
PluginProcessor::loadPreset(const std::string& path)
{
    if (!myPlugin.get()) {
        std::cout << "You must load a plugin before loading a preset." << std::endl;
        return false;
    }

    try {
        if (!std::filesystem::exists(path.c_str())) {
            std::cout << "File not found: " << path.c_str() << std::endl;
            return false;
        }

        MemoryBlock mb;
        File file = File(path);
        file.loadFileAsData(mb);

        // The VST2 way of loading preset.
        bool result = VSTPluginFormat::loadFromFXBFile(myPlugin.get(), mb.getData(), mb.getSize());

        for (int i = 0; i < myPlugin->getNumParameters(); i++) {
            // set the values on the layout.
            setParameter(i, myPlugin.get()->getParameter(i));
        }

        return result;
    }
    catch (std::exception& e) {
        std::cout << "Error: (PluginProcessor::loadPreset) " << e.what() << std::endl;
        return false;
    }

}

bool
PluginProcessor::loadVST3Preset(const std::string& path)
{
    if (!myPlugin.get()) {
        std::cout << "You must load a plugin before loading a preset." << std::endl;
        return false;
    }

    try {
        if (!std::filesystem::exists(path.c_str())) {
            std::cout << "File not found: " << path.c_str() << std::endl;
            return false;
        }

        MemoryBlock mb;
        File file = File(path);
        file.loadFileAsData(mb);
        bool result = setVST3PluginStateDirect(myPlugin.get(), mb);

        for (int i = 0; i < myPlugin->getNumParameters(); i++) {
            // set the values on the layout.
            setParameter(i, myPlugin.get()->getParameter(i));
        }

        return result;
    }
    catch (std::exception& e) {
        std::cout << "Error: (PluginProcessor::loadVST3Preset) " << e.what() << std::endl;
        return false;
    }

}

void
PluginProcessor::createParameterLayout()
{

    juce::AudioProcessorValueTreeState::ParameterLayout blankLayout;

    // clear existing parameters in the layout?
    ValueTree blankState;
    myParameters.replaceState(blankState);

    for (int i = 0; i < myPlugin->getNumParameters(); ++i)
    {
        auto parameterName = myPlugin->getParameterName(i);
        std::string paramID = std::to_string(i);
        myParameters.createAndAddParameter(std::make_unique<AutomateParameterFloat>(paramID, parameterName, NormalisableRange<float>(0.f, 1.f), 0.f));
        // give it a valid single sample of automation.
        ProcessorBase::setAutomationVal(paramID, myPlugin->getParameter(i));
    }
}

void
PluginProcessor::setPatch(const PluginPatch patch)
{

    for (auto pair : patch) {

        if (pair.first < myPlugin->getNumParameters()) {
            setParameter(pair.first, pair.second);
        }
        else {
            std::cout << "RenderEngine::setPatch error: Incorrect parameter index!" <<
                "\n- Current index:  " << pair.first <<
                "\n- Max index: " << myPlugin->getNumParameters() - 1 << std::endl;
        }
    }

}

//==============================================================================
std::string
PluginProcessor::getParameterAsText(const int parameter)
{
    if (!myPlugin) {
        std::cout << "Please load the plugin first!" << std::endl;
        return "";
    }
    return myPlugin->getParameterText(parameter).toStdString();
}

//==============================================================================
void
PluginProcessor::setParameter(const int paramIndex, const float value)
{
    if (!myPlugin) {
        std::cout << "Please load the plugin first!" << std::endl;
        return;
    }

    std::string paramID = std::to_string(paramIndex);

    ProcessorBase::setAutomationVal(paramID, value);
}

//==============================================================================
const PluginPatch
PluginProcessor::getPatch() {

    PluginPatch params;

    if (!myPlugin) {
        std::cout << "Please load the plugin first!" << std::endl;
        return params;
    }

    params.clear();
    params.reserve(myPlugin->getNumParameters());

    for (int i = 0; i < myPlugin->AudioProcessor::getNumParameters(); i++) {

        auto theName = myPlugin->getParameterName(i);

        if (theName == "Param") {
            continue;
        }

        auto parameter = ((AutomateParameterFloat*)myParameters.getParameter(theName));
        if (parameter) {
            float val = parameter->sample(0);
            if (parameter) {
                params.push_back(std::make_pair(i, val));
            }
            else {
                std::cout << "Error getPatch: " << theName << std::endl;
            }
        }
        else {
            std::cout << "Error getPatch with parameter: " << theName << std::endl;
        }

    }

    params.shrink_to_fit();

    return params;
}

const size_t
PluginProcessor::getPluginParameterSize()
{
    if (!myPlugin) {
        std::cout << "Please load the plugin first!" << std::endl;
        return 0;
    }

    return myPlugin->getNumParameters();
}

int
PluginProcessor::getNumMidiEvents() {
    return myMidiBuffer.getNumEvents();
};

bool
PluginProcessor::loadMidi(const std::string& path)
{
    File file = File(path);
    FileInputStream fileStream(file);
    MidiFile midiFile;
    midiFile.readFrom(fileStream);
    midiFile.convertTimestampTicksToSeconds();
    myMidiBuffer.clear();

    for (int t = 0; t < midiFile.getNumTracks(); t++) {
        const MidiMessageSequence* track = midiFile.getTrack(t);
        for (int i = 0; i < track->getNumEvents(); i++) {
            MidiMessage& m = track->getEventPointer(i)->message;
            int sampleOffset = (int)(mySampleRate * m.getTimeStamp());
            myMidiBuffer.addEvent(m, sampleOffset);
        }
    }

    return true;
}

void
PluginProcessor::clearMidi() {
    myMidiBuffer.clear();
}

bool
PluginProcessor::addMidiNote(uint8  midiNote,
    uint8  midiVelocity,
    const double noteStart,
    const double noteLength) {

    if (midiNote > 255) midiNote = 255;
    if (midiNote < 0) midiNote = 0;
    if (midiVelocity > 255) midiVelocity = 255;
    if (midiVelocity < 0) midiVelocity = 0;
    if (noteLength <= 0) {
        return false;
    }

    // Get the note on midiBuffer.
    MidiMessage onMessage = MidiMessage::noteOn(1,
        midiNote,
        midiVelocity);

    MidiMessage offMessage = MidiMessage::noteOff(1,
        midiNote,
        midiVelocity);

    auto startTime = noteStart * mySampleRate;
    onMessage.setTimeStamp(startTime);
    offMessage.setTimeStamp(startTime + noteLength * mySampleRate);
    myMidiBuffer.addEvent(onMessage, (int)onMessage.getTimeStamp());
    myMidiBuffer.addEvent(offMessage, (int)offMessage.getTimeStamp());

    return true;
}

//==============================================================================

PluginProcessorWrapper::PluginProcessorWrapper(std::string newUniqueName, double sampleRate, int samplesPerBlock, std::string path) :
    PluginProcessor(newUniqueName, sampleRate, samplesPerBlock, path)
{
}

void
PluginProcessorWrapper::wrapperSetPatch(py::list listOfTuples)
{
    PluginPatch patch = customBoost::listOfTuplesToPluginPatch(listOfTuples);
    PluginProcessor::setPatch(patch);
}

py::list
PluginProcessorWrapper::wrapperGetPatch()
{
    return customBoost::pluginPatchToListOfTuples(PluginProcessor::getPatch());
}

float
PluginProcessorWrapper::wrapperGetParameter(int parameterIndex)
{
    if (!myPlugin) {
        std::cout << "Please load the plugin first!" << std::endl;
        return 0.;
    }

    if (parameterIndex >= myPlugin->AudioProcessor::getNumParameters()) {
        std::cout << "Parameter not found for index: " << parameterIndex << std::endl;
        return 0.;
    }

    return ProcessorBase::getAutomationVal(std::to_string(parameterIndex), 0);
}

std::string
PluginProcessorWrapper::wrapperGetParameterName(int parameter)
{
    return myPlugin->getParameterName(parameter).toStdString();
}

bool
PluginProcessorWrapper::wrapperSetParameter(int parameter, float value)
{
    if (!myPlugin) {
        std::cout << "Please load the plugin first!" << std::endl;
        return false;
    }

    std::string paramID = std::to_string(parameter);

    return ProcessorBase::setAutomationVal(paramID, value);
}

bool
PluginProcessorWrapper::wrapperSetAutomation(int parameterIndex, py::array input) {
    return PluginProcessorWrapper::setAutomation(std::to_string(parameterIndex), input);
}

int
PluginProcessorWrapper::wrapperGetPluginParameterSize()
{
    return int(PluginProcessor::getPluginParameterSize());
}

py::list
PluginProcessorWrapper::getPluginParametersDescription()
{
    py::list myList;

    if (myPlugin != nullptr) {

        //get the parameters as an AudioProcessorParameter array
        const Array<AudioProcessorParameter*>& processorParams = myPlugin->getParameters();
        for (int i = 0; i < myPlugin->AudioProcessor::getNumParameters(); i++) {

            int maximumStringLength = 64;

            std::string theName = (processorParams[i])->getName(maximumStringLength).toStdString();
            std::string currentText = processorParams[i]->getText(processorParams[i]->getValue(), maximumStringLength).toStdString();
            std::string label = processorParams[i]->getLabel().toStdString();

            py::dict myDictionary;
            myDictionary["index"] = i;
            myDictionary["name"] = theName;
            myDictionary["numSteps"] = processorParams[i]->getNumSteps();
            myDictionary["isDiscrete"] = processorParams[i]->isDiscrete();
            myDictionary["label"] = label;
            myDictionary["text"] = currentText;

            myList.append(myDictionary);
        }
    }
    else
    {
        std::cout << "Please load the plugin first!" << std::endl;
    }

    return myList;
}

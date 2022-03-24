#pragma once

#include "PluginProcessor.h"

#include <filesystem>

using juce::ExtensionsVisitor;

struct PresetVisitor : public ExtensionsVisitor {
    const std::string presetFilePath;

    PresetVisitor(const std::string presetFilePath) : presetFilePath(presetFilePath) { }

    void visitVST3Client(const ExtensionsVisitor::VST3Client& client) override {
        juce::File presetFile(presetFilePath);
        juce::MemoryBlock presetData;

        if (!presetFile.loadFileAsData(presetData)) {
            throw std::runtime_error("Failed to read preset file: " + presetFilePath);
        }

        if (!client.setPreset(presetData)) {
            throw std::runtime_error("Failed to set preset file: " + presetFilePath);
        }
    }
};


PluginProcessor::PluginProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock, std::string path) : ProcessorBase{ newUniqueName }
{
    myPluginPath = path;

    isLoaded = loadPlugin(sampleRate, samplesPerBlock);
}

bool
PluginProcessor::loadPlugin(double sampleRate, int samplesPerBlock) {
    OwnedArray<PluginDescription> pluginDescriptions;
    KnownPluginList pluginList;
    AudioPluginFormatManager pluginFormatManager;

    pluginFormatManager.addDefaultFormats();
    
    juce::MessageManager::getInstance(); // to avoid runtime jassert(false) thrown by JUCE

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
        throw std::runtime_error("Unable to load plugin.");
        return false;
    }

    String errorMessage;

    myPlugin = pluginFormatManager.createPluginInstance(*pluginDescriptions[0],
        sampleRate,
        samplesPerBlock,
        errorMessage);

    if (myPlugin.get() == nullptr)
    {
        throw std::runtime_error("PluginProcessor::loadPlugin error: " + errorMessage.toStdString());
        return false;
    }

    myPlugin->enableAllBuses();

    auto inputs = myPlugin->getTotalNumInputChannels();
    auto outputs = myPlugin->getTotalNumOutputChannels();
    this->setPlayConfigDetails(inputs, outputs, sampleRate, samplesPerBlock);
    myPlugin->setPlayConfigDetails(inputs, outputs, sampleRate, samplesPerBlock);
    myPlugin->setNonRealtime(true);
    mySampleRate = sampleRate;
    
    createParameterLayout();

    return true;
}

PluginProcessor::~PluginProcessor() {
    if (myPlugin.get())
    {
        myPlugin->releaseResources();
        myPlugin.release();
    }
    delete myMidiIterator;
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
    AudioPlayHead::CurrentPositionInfo posInfo;
    getPlayHead()->getCurrentPosition(posInfo);
    myRenderMidiBuffer.clear();
    
    if (!myPlugin.get() || !isLoaded) {
        buffer.clear();
        
        if (posInfo.ppqPosition == 0) {
            std::cerr << "Error: no plugin was processed." << std::endl;
        }
        return;
    }
    
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
                std::cerr << "Error automateParameters: " << myPlugin->getParameterName(i) << std::endl;
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

    delete myMidiIterator;
    myMidiIterator = new MidiBuffer::Iterator(myMidiBuffer); // todo: deprecated.
    myMidiEventsDoRemain = myMidiIterator->getNextEvent(myMidiMessage, myMidiMessagePosition);
    myRenderMidiBuffer.clear();
}

bool
PluginProcessor::loadPreset(const std::string& path)
{
    if (!myPlugin.get()) {
        throw std::runtime_error("You must load a plugin before loading a preset.");
        return false;
    }

    try {
        if (!std::filesystem::exists(path.c_str())) {
            throw std::runtime_error("File not found: " + path);
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
        throw std::runtime_error(std::string("Error: (PluginProcessor::loadPreset) ") + e.what());
        return false;
    }

}

bool
PluginProcessor::loadVST3Preset(const std::string& path)
{
    if (!myPlugin.get()) {
        throw std::runtime_error("You must load a plugin before loading a preset.");
        return false;
    }

    juce::File fPath(path);

    if (!fPath.getFileExtension().equalsIgnoreCase(".VSTPRESET")) {
        std::cerr << "The file extension is not .vstpreset for file: " << path.c_str() << std::endl;
    }

    if (!std::filesystem::exists(path.c_str())) {
        throw std::runtime_error("Preset file not found: " + path);
        return false;
    }

    PresetVisitor presetVisitor{ path };

    try
    {
        myPlugin->getExtensions(presetVisitor);
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("PluginProcessor::loadVST3Preset: unknown error.");
        return false;
    }
    
    for (int i = 0; i < myPlugin->getNumParameters(); i++) {
        // set the values on the layout.
        setParameter(i, myPlugin.get()->getParameter(i));
    }

    return true;
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
            std::cerr << "RenderEngine::setPatch error: Incorrect parameter index!" <<
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
        std::cerr << "Please load the plugin first!" << std::endl;
        return "";
    }
    return myPlugin->getParameterText(parameter).toStdString();
}

//==============================================================================
void
PluginProcessor::setParameter(const int paramIndex, const float value)
{
    if (!myPlugin) {
        std::cerr << "Please load the plugin first!" << std::endl;
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
        std::cerr << "Please load the plugin first!" << std::endl;
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
                std::cerr << "Error getPatch: " << theName << std::endl;
            }
        }
        else {
            std::cerr << "Error getPatch with parameter: " << theName << std::endl;
        }

    }

    params.shrink_to_fit();

    return params;
}

const size_t
PluginProcessor::getPluginParameterSize()
{
    if (!myPlugin) {
        std::cerr << "Please load the plugin first!" << std::endl;
        return 0;
    }

    return myPlugin->getNumParameters();
}

int
PluginProcessor::getNumMidiEvents() {
    return myMidiBuffer.getNumEvents();
};

bool
PluginProcessor::loadMidi(const std::string& path, bool allEvents)
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
            if (allEvents || m.isNoteOff() || m.isNoteOn()) {
                myMidiBuffer.addEvent(m, sampleOffset);
            }
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
        throw std::runtime_error("The note length must be greater than zero.");
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
        std::cerr << "Please load the plugin first!" << std::endl;
        return 0.;
    }

    if (parameterIndex >= myPlugin->AudioProcessor::getNumParameters()) {
        std::cerr << "Parameter not found for index: " << parameterIndex << std::endl;
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
        throw std::runtime_error("Please load the plugin first!");
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
        std::cerr << "Please load the plugin first!" << std::endl;
    }

    return myList;
}

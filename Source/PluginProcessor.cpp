#pragma once

#include "PluginProcessor.h"

#include <filesystem>

#include "StandalonePluginWindow.h"

static std::mutex PLUGIN_INSTANCE_MUTEX;
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

void
PluginProcessor::openEditor() {
    if (!myPlugin) {
        throw std::runtime_error(
            "Editor cannot be shown because the plugin isn't loaded.");
    }

    if (!juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()) {
        throw std::runtime_error(
            "Editor cannot be shown because no visual display devices are available.");
    }

    if (!juce::MessageManager::getInstance()->isThisTheMessageThread()) {
        throw std::runtime_error(
            "Plugin UI windows can only be shown from the main thread.");
    }

    StandalonePluginWindow::openWindowAndWait(*this, *myPlugin);
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
        std::lock_guard<std::mutex> lock(PLUGIN_INSTANCE_MUTEX);
        myPlugin.reset();
    }

    // If there is a problem here first check the preprocessor definitions
    // in the projucer are sensible - is it set up to scan for plugin's?
    if (pluginDescriptions.size() <= 0) {
        throw std::runtime_error("Unable to load plugin.");
    }

    String errorMessage;

    std::lock_guard<std::mutex> lock(PLUGIN_INSTANCE_MUTEX);
    myPlugin = pluginFormatManager.createPluginInstance(*pluginDescriptions[0],
        sampleRate,
        samplesPerBlock,
        errorMessage);

    if (myPlugin.get() == nullptr)
    {
        throw std::runtime_error("PluginProcessor::loadPlugin error: " + errorMessage.toStdString());
    }

    myPlugin->enableAllBuses();

    auto inputs = myPlugin->getTotalNumInputChannels();
    auto outputs = myPlugin->getTotalNumOutputChannels();
    this->setPlayConfigDetails(inputs, outputs, sampleRate, samplesPerBlock);
    myPlugin->setPlayConfigDetails(inputs, outputs, sampleRate, samplesPerBlock);
    myPlugin->prepareToPlay(sampleRate, samplesPerBlock);
    myPlugin->setNonRealtime(true);
    mySampleRate = sampleRate;
    
    createParameterLayout();

    {
        // Process a block of silence a few times to "warm up" the processor.
        juce::AudioSampleBuffer audioBuffer = AudioSampleBuffer(std::max(inputs, outputs), samplesPerBlock);
        MidiBuffer emptyMidiBuffer;
        for (int i = 0; i < 5; i++) {
            audioBuffer.clear();
            myPlugin->processBlock(audioBuffer, emptyMidiBuffer);
        }
    }

    return true;
}

PluginProcessor::~PluginProcessor() {
    if (myPlugin.get())
    {
        std::lock_guard<std::mutex> lock(PLUGIN_INSTANCE_MUTEX);
        myPlugin.reset();
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

bool
PluginProcessor::setBusesLayout(const BusesLayout& arr) {
    if (myPlugin.get()) {
        AudioProcessor::setBusesLayout(arr);
        return myPlugin->setBusesLayout(arr);
    }
    return false;
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
            throw std::runtime_error("Error: no plugin was processed for processor named " + this->getUniqueName());
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
                throw std::runtime_error("Error automateParameters: " + myPlugin->getParameterName(i).toStdString());
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
    }

    try {
        if (!std::filesystem::exists(path.c_str())) {
            throw std::runtime_error("File not found: " + path);
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
    }

}

bool
PluginProcessor::loadVST3Preset(const std::string& path)
{
    if (!myPlugin.get()) {
        throw std::runtime_error("You must load a plugin before loading a preset.");
    }

    juce::File fPath(path);

    if (!fPath.getFileExtension().equalsIgnoreCase(".VSTPRESET")) {
        throw std::runtime_error("The file extension is not .vstpreset for file: " + path);
    }

    if (!std::filesystem::exists(path.c_str())) {
        throw std::runtime_error("Preset file not found: " + path);
    }

    PresetVisitor presetVisitor{ path };

    try
    {
        myPlugin->getExtensions(presetVisitor);
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("PluginProcessor::loadVST3Preset: unknown error.");
    }
    
    for (int i = 0; i < myPlugin->getNumParameters(); i++) {
        // set the values on the layout.
        setParameter(i, myPlugin.get()->getParameter(i));
    }

    return true;
}

void
PluginProcessor::loadStateInformation(std::string filepath) {

    if (!std::filesystem::exists(filepath.c_str())) {
        throw std::runtime_error("File not found: " + filepath);
    }

    MemoryBlock state;
    File file = File(filepath);
    file.loadFileAsData(state);

    myPlugin->setStateInformation((const char*)state.getData(), (int)state.getSize());

    for (int i = 0; i < myPlugin->AudioProcessor::getNumParameters(); i++) {
        std::string paramID = std::to_string(i);
        ProcessorBase::setAutomationVal(paramID, myPlugin->getParameter(i));
    }

    // todo: this is a little hacky. We create a window because this forces the loaded state to take effect
    // in certain plugins.
    // This allows us to call load_state and not bother calling open_editor().
    StandalonePluginWindow tmp_window(*this, *myPlugin);
}

void
PluginProcessor::saveStateInformation(std::string filepath) {
    if (!myPlugin) {
        throw std::runtime_error("Please load the plugin first");
    }
    MemoryBlock state;
    myPlugin->getStateInformation(state);

    juce::File file(filepath);
    juce::FileOutputStream stream(file);

    if (stream.openedOk())
    {
        // overwrite existing file.
        stream.setPosition(0);
        stream.truncate();
    }

    stream.write(state.getData(), state.getSize());
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
            throw std::runtime_error(
                "RenderEngine::setPatch error: Incorrect parameter index!"
                "\n- Current index:  " + std::to_string(pair.first) +
                "\n- Max index: " + std::to_string(myPlugin->getNumParameters() - 1)
            );
        }
    }

}

//==============================================================================
std::string
PluginProcessor::getParameterAsText(const int parameter)
{
    if (!myPlugin) {
        throw std::runtime_error("Please load the plugin first!");
        return "";
    }
    return myPlugin->getParameterText(parameter).toStdString();
}

//==============================================================================
void
PluginProcessor::setParameter(const int paramIndex, const float value)
{
    if (!myPlugin) {
        throw std::runtime_error("Please load the plugin first!");
        return;
    }

    myPlugin->setParameter(paramIndex, value);

    std::string paramID = std::to_string(paramIndex);

    ProcessorBase::setAutomationVal(paramID, value);
}

//==============================================================================
const PluginPatch
PluginProcessor::getPatch() {

    PluginPatch params;

    if (!myPlugin) {
        throw std::runtime_error("Please load the plugin first!");
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
                throw std::runtime_error("Error getPatch : " + theName.toStdString());
            }
        }
        else {
            throw std::runtime_error("Error getPatch with parameter: " + theName.toStdString());
        }

    }

    params.shrink_to_fit();

    return params;
}

const size_t
PluginProcessor::getPluginParameterSize()
{
    if (!myPlugin) {
        throw std::runtime_error("Please load the plugin first!");
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
        throw std::runtime_error("Please load the plugin first!");
        return 0.;
    }

    if (parameterIndex >= myPlugin->AudioProcessor::getNumParameters()) {
        throw std::runtime_error("Parameter not found for index: " + std::to_string(parameterIndex));
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
        throw std::runtime_error("Please load the plugin first!");
    }

    return myList;
}

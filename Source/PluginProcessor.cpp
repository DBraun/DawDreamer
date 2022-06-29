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
    delete myMidiIteratorQN;
    delete myMidiIteratorSec;
}

void PluginProcessor::setPlayHead(AudioPlayHead* newPlayHead)
{
    ProcessorBase::setPlayHead(newPlayHead);
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
        ProcessorBase::setBusesLayout(arr);
        return myPlugin->setBusesLayout(arr);
    }
    return false;
}

void
PluginProcessor::numChannelsChanged() {
    ProcessorBase::numChannelsChanged();
    if (myPlugin.get()) {
        myPlugin->setPlayConfigDetails(this->getTotalNumInputChannels(), this->getTotalNumOutputChannels(), this->getSampleRate(), this->getBlockSize());
    }
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
    // todo: PluginProcessor should be able to use the incoming midiBuffer too.
    
    if (!myPlugin.get() || !isLoaded) {
        throw std::runtime_error("Error: no plugin was processed for processor named " + this->getUniqueName());
    }
    
    auto posInfo = getPlayHead()->getPosition();

    myRenderMidiBuffer.clear();

    {
        auto start = *posInfo->getTimeInSamples();
        auto end = start + buffer.getNumSamples();
        myIsMessageBetweenSec = myMidiMessagePositionSec >= start && myMidiMessagePositionSec < end;
        while (myIsMessageBetweenSec && myMidiEventsDoRemainSec) {
            // steps for saving midi to file output
            auto messageCopy = MidiMessage(myMidiMessageSec);
            messageCopy.setTimeStamp(myMidiMessagePositionSec*(2400./mySampleRate));
            if (!(messageCopy.isEndOfTrackMetaEvent() || messageCopy.isTempoMetaEvent())) {
                myRecordedMidiSequence.addEvent(messageCopy);
            }

            // steps for playing MIDI
            myRenderMidiBuffer.addEvent(myMidiMessageSec, int(myMidiMessagePositionSec - start));
            myMidiEventsDoRemainSec = myMidiIteratorSec->getNextEvent(myMidiMessageSec, myMidiMessagePositionSec);
            myIsMessageBetweenSec = myMidiMessagePositionSec >= start && myMidiMessagePositionSec < end;
        }
    }

    {
        auto pulseStart = std::floor(*posInfo->getPpqPosition() * PPQN);
        auto pulseEnd = pulseStart + buffer.getNumSamples() * (*posInfo->getBpm() * PPQN) / (mySampleRate * 60.);

        myIsMessageBetweenQN = myMidiMessagePositionQN >= pulseStart && myMidiMessagePositionQN < pulseEnd;
        while (myIsMessageBetweenQN && myMidiEventsDoRemainQN) {
            // steps for saving midi to file output
            auto messageCopy = MidiMessage(myMidiMessageQN);
            messageCopy.setTimeStamp((*posInfo->getTimeInSeconds() + (myMidiMessagePositionQN-pulseStart)*(60./(*posInfo->getBpm()))/PPQN )*2400.);
            if (!(messageCopy.isEndOfTrackMetaEvent() || messageCopy.isTempoMetaEvent())) {
                myRecordedMidiSequence.addEvent(messageCopy);
            }
            
            // steps for playing MIDI
            myRenderMidiBuffer.addEvent(myMidiMessageQN, int( (myMidiMessagePositionQN - pulseStart)*60.*mySampleRate/(PPQN**posInfo->getBpm())));
            myMidiEventsDoRemainQN = myMidiIteratorQN->getNextEvent(myMidiMessageQN, myMidiMessagePositionQN);
            myIsMessageBetweenQN = myMidiMessagePositionQN >= pulseStart && myMidiMessagePositionQN < pulseEnd;
        }
    }

    myPlugin->processBlock(buffer, myRenderMidiBuffer);
        
    ProcessorBase::processBlock(buffer, midiBuffer);
}


void
PluginProcessor::automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) {

    if (!myPlugin.get()) {
        return;
    }
            
    int i = 0;
    
    for (juce::AudioProcessorParameter *parameter : myPlugin->getParameters()) {
        auto paramID = std::to_string(i);
        auto theParameter = ((AutomateParameterFloat*)myParameters.getParameter(paramID));

        const juce::ScopedLock sl2 (myPlugin->getCallbackLock());
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(theParameter->sample(posInfo));  // todo: this line breaks test_plugins.py
        parameter->endChangeGesture();
    
        i++;
    }
}

void
PluginProcessor::reset()
{
    if (myPlugin.get()) {
        myPlugin->reset();
    }

    delete myMidiIteratorSec;
    myMidiIteratorSec = new MidiBuffer::Iterator(myMidiBufferSec); // todo: deprecated.

    myMidiEventsDoRemainSec = myMidiIteratorSec->getNextEvent(myMidiMessageSec, myMidiMessagePositionSec);

    delete myMidiIteratorQN;
    myMidiIteratorQN = new MidiBuffer::Iterator(myMidiBufferQN); // todo: deprecated.

    myMidiEventsDoRemainQN = myMidiIteratorQN->getNextEvent(myMidiMessageQN, myMidiMessagePositionQN);

    myRenderMidiBuffer.clear();
    
    myRecordedMidiSequence.clear();
    myRecordedMidiSequence.addEvent(juce::MidiMessage::midiStart());
    myRecordedMidiSequence.addEvent(juce::MidiMessage::timeSignatureMetaEvent(4, 4));
    myRecordedMidiSequence.addEvent(juce::MidiMessage::tempoMetaEvent(500*1000));
    myRecordedMidiSequence.addEvent(juce::MidiMessage::midiChannelMetaEvent(1));
    
    ProcessorBase::reset();
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
                
        int i = 0;
        for (auto *parameter : myPlugin->getParameters()) {
            setParameter(i, parameter->getValue());
            i++;
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
    
    
    int i = 0;
    for (auto *parameter : myPlugin->getParameters()) {
        setParameter(i, parameter->getValue());
        i++;
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
    
    
    int i = 0;
    for (auto *parameter : myPlugin->getParameters()) {
        std::string paramID = std::to_string(i);
        ProcessorBase::setAutomationVal(paramID, parameter->getValue());
        i++;
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
    
    int maximumStringLength = 64;
    int i = 0;
    for (auto *parameter : myPlugin->getParameters()) {
        auto parameterName = parameter->getName(maximumStringLength);
        std::string paramID = std::to_string(i);
        myParameters.createAndAddParameter(std::make_unique<AutomateParameterFloat>(paramID, parameterName, NormalisableRange<float>(0.f, 1.f), 0.f));
        // give it a valid single sample of automation.
        ProcessorBase::setAutomationVal(paramID, parameter->getValue());
        i++;
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
PluginProcessor::setParameter(const int parameterIndex, const float value)
{
    if (!myPlugin) {
        throw std::runtime_error("Please load the plugin first!");
        return;
    }

    myPlugin->setParameter(parameterIndex, value); // todo: instead we need to do parameter->setValue(value)

    std::string paramID = std::to_string(parameterIndex);

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

    AudioPlayHead::PositionInfo posInfo;
    posInfo.setTimeInSeconds(0.);
    posInfo.setTimeInSamples(0.);
    for (int i = 0; i < myPlugin->getNumParameters(); i++) {

        auto theName = myPlugin->getParameterName(i);

        if (theName == "Param") {
            continue;
        }

        auto parameter = ((AutomateParameterFloat*)myParameters.getParameter(theName));
        if (parameter) {
            float val = parameter->sample(posInfo);
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
    return myMidiBufferSec.getNumEvents() + myMidiBufferQN.getNumEvents();
};

bool
PluginProcessor::loadMidi(const std::string& path, bool clearPrevious, bool isBeats, bool allEvents)
{
    if (!std::filesystem::exists(path.c_str())) {
        throw std::runtime_error("File not found: " + path);
    }

    File file = File(path);
    FileInputStream fileStream(file);
    MidiFile midiFile;
    midiFile.readFrom(fileStream);

    if (clearPrevious) {
        myMidiBufferSec.clear();
        myMidiBufferQN.clear();
    }

    if (!isBeats) {
        midiFile.convertTimestampTicksToSeconds();

        for (int t = 0; t < midiFile.getNumTracks(); t++) {
            const MidiMessageSequence* track = midiFile.getTrack(t);
            for (int i = 0; i < track->getNumEvents(); i++) {
                MidiMessage& m = track->getEventPointer(i)->message;
                int sampleOffset = (int)(mySampleRate * m.getTimeStamp());
                if (allEvents || m.isNoteOff() || m.isNoteOn()) {
                    myMidiBufferSec.addEvent(m, sampleOffset);
                }
            }
        }
    }
    else {
        auto timeFormat = midiFile.getTimeFormat(); // the ppqn (Ableton makes midi files with 96 ppqn)

        for (int t = 0; t < midiFile.getNumTracks(); t++) {
            const MidiMessageSequence* track = midiFile.getTrack(t);
            for (int i = 0; i < track->getNumEvents(); i++) {
                MidiMessage& m = track->getEventPointer(i)->message;
                if (allEvents || m.isNoteOff() || m.isNoteOn()) {
                    // convert timestamp from its original time format to our high resolution PPQN
                    auto timeStamp = m.getTimeStamp() * PPQN / timeFormat;
                    myMidiBufferQN.addEvent(m, timeStamp);
                }
            }
        }
    }

    return true;
}

void
PluginProcessor::clearMidi() {
    myMidiBufferSec.clear();
    myMidiBufferQN.clear();
}

bool
PluginProcessor::addMidiNote(uint8  midiNote,
    uint8  midiVelocity,
    const double noteStart,
    const double noteLength,
    bool isBeats) {

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

    if (!isBeats) {
        auto startTime = noteStart * mySampleRate;
        onMessage.setTimeStamp(startTime);
        offMessage.setTimeStamp(startTime + noteLength * mySampleRate);
        myMidiBufferSec.addEvent(onMessage, (int)onMessage.getTimeStamp());
        myMidiBufferSec.addEvent(offMessage, (int)offMessage.getTimeStamp());
    }
    else {
        auto startTime = noteStart * PPQN;
        onMessage.setTimeStamp(startTime);
        offMessage.setTimeStamp(startTime + noteLength * PPQN);
        myMidiBufferQN.addEvent(onMessage, (int)onMessage.getTimeStamp());
        myMidiBufferQN.addEvent(offMessage, (int)offMessage.getTimeStamp());
    }

    return true;
}

void PluginProcessor::saveMIDI(std::string& savePath) {
    
    MidiFile file;
    
    // 30*80 = 2400, so that's why the MIDI messages had their
    // timestamp set to seconds*2400
    file.setSmpteTimeFormat(30, 80);
    
    File myFile(savePath);

    file.addTrack( myRecordedMidiSequence );
    
    juce::FileOutputStream stream( myFile );
    if (stream.openedOk())
    {
        // overwrite existing file.
        stream.setPosition(0);
        stream.truncate();
    }
    file.writeTo( stream );
    
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

    if (parameterIndex >= myPlugin->getNumParameters()) {
        throw std::runtime_error("Parameter not found for index: " + std::to_string(parameterIndex));
        return 0.;
    }

    return ProcessorBase::getAutomationAtZero(std::to_string(parameterIndex));
}

std::string
PluginProcessorWrapper::wrapperGetParameterName(int parameter)
{
    return myPlugin->getParameterName(parameter).toStdString();
}

bool
PluginProcessorWrapper::wrapperSetParameter(int parameterIndex, float value)
{
    
    if (!myPlugin) {
        throw std::runtime_error("Please load the plugin first!");
        return false;
    }

    myPlugin->setParameter(parameterIndex, value); // todo: instead we need to do parameter->setValue(value)

    std::string paramID = std::to_string(parameterIndex);

    return ProcessorBase::setAutomationVal(paramID, value);
}

bool
PluginProcessorWrapper::wrapperSetAutomation(int parameterIndex, py::array input, std::uint32_t ppqn) {
    return PluginProcessorWrapper::setAutomation(std::to_string(parameterIndex), input, ppqn);
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
        for (int i = 0; i < myPlugin->getNumParameters(); i++) {

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

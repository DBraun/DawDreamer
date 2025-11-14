#include "PluginProcessor.h"

#include <filesystem>
#include <regex>

#include "StandalonePluginWindow.h"

using juce::ExtensionsVisitor;

#define THROW_ERROR_IF_NO_PLUGIN                                                                   \
    if (!myPlugin)                                                                                 \
    {                                                                                              \
        throw std::runtime_error("Please load the plugin first!");                                 \
    }

struct PresetVisitor : public ExtensionsVisitor
{
    const std::string presetFilePath;

    PresetVisitor(const std::string presetFilePath) : presetFilePath(presetFilePath) {}

    void visitVST3Client(const ExtensionsVisitor::VST3Client& client) override
    {
        juce::File presetFile(presetFilePath);
        juce::MemoryBlock presetData;

        if (!presetFile.loadFileAsData(presetData))
        {
            throw std::runtime_error("Failed to read preset file: " + presetFilePath);
        }

        if (!client.setPreset(presetData))
        {
            throw std::runtime_error("Failed to set preset file: " + presetFilePath);
        }
    }
};

PluginProcessor::PluginProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock,
                                 std::string path)
    : ProcessorBase{newUniqueName}, myPlugin{nullptr}
{
    myPluginPath = path;

    loadPlugin(sampleRate, samplesPerBlock);
}

void PluginProcessor::openEditor()
{
    THROW_ERROR_IF_NO_PLUGIN

    if (!juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        throw std::runtime_error("Editor cannot be shown because no visual display devices are "
                                 "available.");
    }

    if (!juce::MessageManager::getInstance()->isThisTheMessageThread())
    {
        throw std::runtime_error("Plugin UI windows can only be shown from the main thread.");
    }

    StandalonePluginWindow::openWindowAndWait(*this, *myPlugin);
}

bool PluginProcessor::loadPlugin(double sampleRate, int samplesPerBlock)
{
    OwnedArray<PluginDescription> pluginDescriptions;
    KnownPluginList pluginList;
    AudioPluginFormatManager pluginFormatManager;

    pluginFormatManager.addDefaultFormats();

    for (int i = pluginFormatManager.getNumFormats(); --i >= 0;)
    {
        pluginList.scanAndAddFile(String(myPluginPath), true, pluginDescriptions,
                                  *pluginFormatManager.getFormat(i));
    }

    if (myPlugin.get())
    {
        myPlugin.get()->releaseResources();
        myPlugin.reset();
    }

    // If there is a problem here first check the preprocessor definitions
    // in the projucer are sensible - is it set up to scan for plugin's?
    if (pluginDescriptions.size() <= 0)
    {
        throw std::runtime_error("Unable to load plugin.");
    }

    String errorMessage;

    myPlugin = pluginFormatManager.createPluginInstance(*pluginDescriptions[0], sampleRate,
                                                        samplesPerBlock, errorMessage);

    if (myPlugin.get() == nullptr)
    {
        throw std::runtime_error("PluginProcessor::loadPlugin error: " +
                                 errorMessage.toStdString());
    }
    // We loaded the plugin.

    auto outputs = myPlugin->getTotalNumOutputChannels();

    if (outputs == 0)
    {
        myPlugin->enableAllBuses();
        myPlugin->disableNonMainBuses();
        outputs = myPlugin->getTotalNumOutputChannels();
    }
    auto inputs = myPlugin->getTotalNumInputChannels();

    ProcessorBase::setBusesLayout(myPlugin->getBusesLayout());

    this->setPlayConfigDetails(inputs, outputs, sampleRate, samplesPerBlock);
    myPlugin->prepareToPlay(sampleRate, samplesPerBlock);
    myPlugin->setNonRealtime(true);
    mySampleRate = sampleRate;

    createParameterLayout();

    {
        // Process a block of silence a few times to "warm up" the processor.
        juce::AudioSampleBuffer audioBuffer =
            AudioSampleBuffer(std::max(inputs, outputs), samplesPerBlock);
        MidiBuffer emptyMidiBuffer;
        for (int i = 0; i < 5; i++)
        {
            audioBuffer.clear();
            myPlugin->processBlock(audioBuffer, emptyMidiBuffer);
        }
    }

    return true;
}

PluginProcessor::~PluginProcessor()
{
    if (myPlugin.get())
    {
        myPlugin.get()->releaseResources();
        myPlugin.reset();
    }

    myMidiBufferQN.clear();
    myMidiBufferSec.clear();
    myRenderMidiBuffer.clear();
    myRecordedMidiSequence.clear();

    delete myMidiIteratorQN;
    delete myMidiIteratorSec;
}

void PluginProcessor::setPlayHead(AudioPlayHead* newPlayHead)
{
    ProcessorBase::setPlayHead(newPlayHead);
    if (myPlugin.get())
    {
        myPlugin->setPlayHead(newPlayHead);
    }
}

bool PluginProcessor::canApplyBusesLayout(const juce::AudioProcessor::BusesLayout& layout)
{
    THROW_ERROR_IF_NO_PLUGIN

    return myPlugin->checkBusesLayoutSupported(layout);
}

bool PluginProcessor::setBusesLayout(const BusesLayout& arr)
{
    THROW_ERROR_IF_NO_PLUGIN
    ProcessorBase::setBusesLayout(arr);
    return myPlugin->setBusesLayout(arr);
}

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    THROW_ERROR_IF_NO_PLUGIN
    myPlugin->prepareToPlay(sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
    THROW_ERROR_IF_NO_PLUGIN
    myPlugin->releaseResources();
}

void PluginProcessor::processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer)
{
    juce::ScopedNoDenormals noDenormals;

    // todo: PluginProcessor should be able to use the incoming midiBuffer too.

    THROW_ERROR_IF_NO_PLUGIN

    auto posInfo = getPlayHead()->getPosition();

    myRenderMidiBuffer.clear();

    const bool isPlaying = posInfo->getIsPlaying();

    if (!isPlaying)
    {
        // send All Notes Off MIDI message to all channels and then process it.
        // todo: is it possible to send all notes midi off without doing a
        // processBlock?
        for (int i = 1; i < 17; i++)
        {
            myRenderMidiBuffer.addEvent(MidiMessage::allNotesOff(i), 0);
        }
        myPlugin->processBlock(buffer, myRenderMidiBuffer);

        ProcessorBase::processBlock(buffer, midiBuffer);
        return;
    }

    {
        auto start = *posInfo->getTimeInSamples();
        auto end = start + buffer.getNumSamples();
        myIsMessageBetweenSec = myMidiMessagePositionSec >= start && myMidiMessagePositionSec < end;
        while (myIsMessageBetweenSec && myMidiEventsDoRemainSec)
        {
            // steps for saving midi to file output
            auto messageCopy = MidiMessage(myMidiMessageSec);
            messageCopy.setTimeStamp(myMidiMessagePositionSec * (2400. / mySampleRate));
            if (!(messageCopy.isEndOfTrackMetaEvent() || messageCopy.isTempoMetaEvent()))
            {
                myRecordedMidiSequence.addEvent(messageCopy);
            }

            // steps for playing MIDI
            myRenderMidiBuffer.addEvent(myMidiMessageSec, int(myMidiMessagePositionSec - start));
            myMidiEventsDoRemainSec =
                myMidiIteratorSec->getNextEvent(myMidiMessageSec, myMidiMessagePositionSec);
            myIsMessageBetweenSec =
                myMidiMessagePositionSec >= start && myMidiMessagePositionSec < end;
        }
    }

    {
        auto pulseStart = std::floor(*posInfo->getPpqPosition() * PPQN);
        auto pulseEnd = pulseStart +
                        buffer.getNumSamples() * (*posInfo->getBpm() * PPQN) / (mySampleRate * 60.);

        myIsMessageBetweenQN =
            myMidiMessagePositionQN >= pulseStart && myMidiMessagePositionQN < pulseEnd;
        while (myIsMessageBetweenQN && myMidiEventsDoRemainQN)
        {
            // steps for saving midi to file output
            auto messageCopy = MidiMessage(myMidiMessageQN);
            messageCopy.setTimeStamp(
                (*posInfo->getTimeInSeconds() +
                 (myMidiMessagePositionQN - pulseStart) * (60. / (*posInfo->getBpm())) / PPQN) *
                2400.);
            if (!(messageCopy.isEndOfTrackMetaEvent() || messageCopy.isTempoMetaEvent()))
            {
                myRecordedMidiSequence.addEvent(messageCopy);
            }

            // steps for playing MIDI
            myRenderMidiBuffer.addEvent(myMidiMessageQN,
                                        int((myMidiMessagePositionQN - pulseStart) * 60. *
                                            mySampleRate / (PPQN * *posInfo->getBpm())));
            myMidiEventsDoRemainQN =
                myMidiIteratorQN->getNextEvent(myMidiMessageQN, myMidiMessagePositionQN);
            myIsMessageBetweenQN =
                myMidiMessagePositionQN >= pulseStart && myMidiMessagePositionQN < pulseEnd;
        }
    }

    myPlugin->processBlock(buffer, myRenderMidiBuffer);

    ProcessorBase::processBlock(buffer, midiBuffer);
}

void PluginProcessor::automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples)
{
    THROW_ERROR_IF_NO_PLUGIN

    int i = 0;

    auto allParameters = this->getParameters();

    for (juce::AudioProcessorParameter* parameter : myPlugin->getParameters())
    {
        if (!parameter->isAutomatable())
        {
            i++;
            continue;
        }

        auto theParameter = (AutomateParameterFloat*)allParameters.getUnchecked(i);

        // only need to update if it's an automated parameter (there are multiple
        // samples to choose from)
        if (theParameter->isAutomated())
        {
            // parameter->setValue(theParameter->sample(posInfo));
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(theParameter->sample(posInfo));
            parameter->endChangeGesture();
        }

        i++;
    }
}

void PluginProcessor::reset()
{
    if (myPlugin.get())
    {
        myPlugin->reset();
    }

    delete myMidiIteratorSec;
    myMidiIteratorSec = new MidiBuffer::Iterator(myMidiBufferSec); // todo: deprecated.

    myMidiEventsDoRemainSec =
        myMidiIteratorSec->getNextEvent(myMidiMessageSec, myMidiMessagePositionSec);

    delete myMidiIteratorQN;
    myMidiIteratorQN = new MidiBuffer::Iterator(myMidiBufferQN); // todo: deprecated.

    myMidiEventsDoRemainQN =
        myMidiIteratorQN->getNextEvent(myMidiMessageQN, myMidiMessagePositionQN);

    myRenderMidiBuffer.clear();

    myRecordedMidiSequence.clear();
    myRecordedMidiSequence.addEvent(juce::MidiMessage::midiStart());
    myRecordedMidiSequence.addEvent(juce::MidiMessage::timeSignatureMetaEvent(4, 4));
    myRecordedMidiSequence.addEvent(juce::MidiMessage::tempoMetaEvent(500 * 1000));
    myRecordedMidiSequence.addEvent(juce::MidiMessage::midiChannelMetaEvent(1));

    ProcessorBase::reset();
}

bool PluginProcessor::loadPreset(const std::string& path)
{
    THROW_ERROR_IF_NO_PLUGIN

    try
    {
        if (!std::filesystem::exists(path))
        {
            throw std::runtime_error("File not found: " + path);
        }

        MemoryBlock mb;
        File file = File(path);
        file.loadFileAsData(mb);

        // The VST2 way of loading preset.
        bool result = VSTPluginFormat::loadFromFXBFile(myPlugin.get(), mb.getData(), mb.getSize());

        int i = 0;
        for (auto* parameter : myPlugin->getParameters())
        {
            auto value = parameter->getValue();
            ProcessorBase::setAutomationValByIndex(i, value);
            i++;
        }

        return result;
    }
    catch (std::exception& e)
    {
        throw std::runtime_error(std::string("Error: (PluginProcessor::loadPreset) ") + e.what());
    }
}

bool PluginProcessor::loadVST3Preset(const std::string& path)
{
    THROW_ERROR_IF_NO_PLUGIN

    juce::File fPath(path);

    if (!fPath.getFileExtension().equalsIgnoreCase(".VSTPRESET"))
    {
        throw std::runtime_error("The file extension is not .vstpreset for file: " + path);
    }

    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("Preset file not found: " + path);
    }

    PresetVisitor presetVisitor{path};

    try
    {
        myPlugin->getExtensions(presetVisitor);
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("PluginProcessor::loadVST3Preset: unknown error.");
    }

    int i = 0;
    for (auto* parameter : myPlugin->getParameters())
    {
        auto value = parameter->getValue();
        ProcessorBase::setAutomationValByIndex(i, value);
        i++;
    }

    return true;
}

void PluginProcessor::loadStateInformation(std::string filepath)
{
    if (!std::filesystem::exists(filepath))
    {
        throw std::runtime_error("File not found: " + filepath);
    }

    MemoryBlock state;
    File file = File(filepath);
    file.loadFileAsData(state);

    myPlugin->setStateInformation((const char*)state.getData(), (int)state.getSize());

    int i = 0;
    for (auto* parameter : myPlugin->getParameters())
    {
        ProcessorBase::setAutomationValByIndex(i, parameter->getValue());
        i++;
    }

    // todo: this is a little hacky. We create a window because this forces the
    // loaded state to take effect in certain plugins. This allows us to call
    // load_state and not bother calling open_editor().
    StandalonePluginWindow tmp_window(*this, *myPlugin);
}

void PluginProcessor::saveStateInformation(std::string filepath)
{
    THROW_ERROR_IF_NO_PLUGIN

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

void PluginProcessor::createParameterLayout()
{
    juce::AudioProcessorParameterGroup group;

    int i = 0;
    for (auto* parameter : myPlugin->getParameters())
    {
        auto parameterName = parameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH);
        std::string paramID = std::to_string(i);

        group.addChild(std::make_unique<AutomateParameterFloat>(
            paramID, parameterName, NormalisableRange<float>(0.f, 1.f), parameter->getValue()));
        i++;
    }

    this->setParameterTree(std::move(group));

    this->updateHostDisplay();

    i = 0;
    for (auto* parameter : this->getParameters())
    {
        ProcessorBase::setAutomationValByIndex(i, parameter->getValue());
        i++;
    }
}

void PluginProcessor::setPatch(const PluginPatch patch)
{
    int i = 0;
    const Array<AudioProcessorParameter*>& pluginParameters = myPlugin->getParameters();
    for (auto pair : patch)
    {
        if (pair.first < myPlugin->getNumParameters())
        {
            pluginParameters.getUnchecked(pair.first)->setValue(pair.second);
            ProcessorBase::setAutomationValByIndex(pair.first, pair.second);
        }
        else
        {
            throw std::runtime_error("RenderEngine::setPatch error: Incorrect parameter index!"
                                     "\n- Current index:  " +
                                     std::to_string(pair.first) + "\n- Max index: " +
                                     std::to_string(myPlugin->getNumParameters() - 1));
        }
        i++;
    }
}

double PluginProcessor::getTailLengthSeconds() const
{
    THROW_ERROR_IF_NO_PLUGIN
    return myPlugin->getTailLengthSeconds();
}

int PluginProcessor::getLatencySamples()
{
    THROW_ERROR_IF_NO_PLUGIN
    return myPlugin->getLatencySamples();
}

//==============================================================================
std::string PluginProcessor::getParameterAsText(const int parameter)
{
    THROW_ERROR_IF_NO_PLUGIN
    return myPlugin->getParameterText(parameter).toStdString();
}

//==============================================================================
const PluginPatch PluginProcessor::getPatch()
{
    PluginPatch params;

    THROW_ERROR_IF_NO_PLUGIN

    params.clear();
    params.reserve(myPlugin->getNumParameters());

    AudioPlayHead::PositionInfo posInfo;
    posInfo.setTimeInSeconds(0.);
    posInfo.setTimeInSamples(0.);

    int i = 0;
    for (auto& uncastedParameter : this->getParameters())
    {
        auto parameter = (AutomateParameterFloat*)uncastedParameter;
        float val = parameter->sample(posInfo);
        params.emplace_back(i, val);
        i++;
    }

    params.shrink_to_fit();

    return params;
}

const size_t PluginProcessor::getPluginParameterSize()
{
    THROW_ERROR_IF_NO_PLUGIN

    return myPlugin->getNumParameters();
}

int PluginProcessor::getNumMidiEvents()
{
    return myMidiBufferSec.getNumEvents() + myMidiBufferQN.getNumEvents();
};

bool PluginProcessor::loadMidi(const std::string& path, bool clearPrevious, bool isBeats,
                               bool allEvents)
{
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("File not found: " + path);
    }

    File file = File(path);
    FileInputStream fileStream(file);
    MidiFile midiFile;
    midiFile.readFrom(fileStream);

    if (clearPrevious)
    {
        myMidiBufferSec.clear();
        myMidiBufferQN.clear();
    }

    if (!isBeats)
    {
        midiFile.convertTimestampTicksToSeconds();

        for (int t = 0; t < midiFile.getNumTracks(); t++)
        {
            const MidiMessageSequence* track = midiFile.getTrack(t);
            for (int i = 0; i < track->getNumEvents(); i++)
            {
                MidiMessage& m = track->getEventPointer(i)->message;
                int sampleOffset = (int)(mySampleRate * m.getTimeStamp());
                if (allEvents || m.isNoteOff() || m.isNoteOn())
                {
                    myMidiBufferSec.addEvent(m, sampleOffset);
                }
            }
        }
    }
    else
    {
        auto timeFormat = midiFile.getTimeFormat(); // the ppqn (Ableton makes midi
                                                    // files with 96 ppqn)

        for (int t = 0; t < midiFile.getNumTracks(); t++)
        {
            const MidiMessageSequence* track = midiFile.getTrack(t);
            for (int i = 0; i < track->getNumEvents(); i++)
            {
                MidiMessage& m = track->getEventPointer(i)->message;
                if (allEvents || m.isNoteOff() || m.isNoteOn())
                {
                    // convert timestamp from its original time format to our high
                    // resolution PPQN
                    auto timeStamp = m.getTimeStamp() * PPQN / timeFormat;
                    myMidiBufferQN.addEvent(m, timeStamp);
                }
            }
        }
    }

    return true;
}

void PluginProcessor::clearMidi()
{
    myMidiBufferSec.clear();
    myMidiBufferQN.clear();
}

bool PluginProcessor::addMidiNote(uint8 midiNote, uint8 midiVelocity, const double noteStart,
                                  const double noteLength, bool isBeats)
{
    if (midiNote > 255)
        midiNote = 255;
    if (midiNote < 0)
        midiNote = 0;
    if (midiVelocity > 255)
        midiVelocity = 255;
    if (midiVelocity < 0)
        midiVelocity = 0;
    if (noteLength <= 0)
    {
        throw std::runtime_error("The note length must be greater than zero.");
    }

    // Get the note on midiBuffer.
    MidiMessage onMessage = MidiMessage::noteOn(1, midiNote, midiVelocity);

    MidiMessage offMessage = MidiMessage::noteOff(1, midiNote, midiVelocity);

    if (!isBeats)
    {
        auto startTime = noteStart * mySampleRate;
        onMessage.setTimeStamp(startTime);
        offMessage.setTimeStamp(startTime + noteLength * mySampleRate);
        myMidiBufferSec.addEvent(onMessage, (int)onMessage.getTimeStamp());
        myMidiBufferSec.addEvent(offMessage, (int)offMessage.getTimeStamp());
    }
    else
    {
        auto startTime = noteStart * PPQN;
        onMessage.setTimeStamp(startTime);
        offMessage.setTimeStamp(startTime + noteLength * PPQN);
        myMidiBufferQN.addEvent(onMessage, (int)onMessage.getTimeStamp());
        myMidiBufferQN.addEvent(offMessage, (int)offMessage.getTimeStamp());
    }

    return true;
}

void PluginProcessor::saveMIDI(std::string& savePath)
{
    MidiFile file;

    // 30*80 = 2400, so that's why the MIDI messages had their
    // timestamp set to seconds*2400
    file.setSmpteTimeFormat(30, 80);

    File myFile(savePath);

    file.addTrack(myRecordedMidiSequence);

    juce::FileOutputStream stream(myFile);
    if (stream.openedOk())
    {
        // overwrite existing file.
        stream.setPosition(0);
        stream.truncate();
    }
    file.writeTo(stream);
}

//==============================================================================

PluginProcessorWrapper::PluginProcessorWrapper(std::string newUniqueName, double sampleRate,
                                               int samplesPerBlock, std::string path)
    : PluginProcessor(newUniqueName, sampleRate, samplesPerBlock, path)
{
}

void PluginProcessorWrapper::wrapperSetPatch(nb::list listOfTuples)
{
    PluginPatch patch = customBoost::listOfTuplesToPluginPatch(listOfTuples);
    PluginProcessor::setPatch(patch);
}

nb::list PluginProcessorWrapper::wrapperGetPatch()
{
    return customBoost::pluginPatchToListOfTuples(PluginProcessor::getPatch());
}

std::string PluginProcessorWrapper::wrapperGetParameterName(const int& parameter)
{
    return myPlugin->getParameterName(parameter).toStdString();
}

bool PluginProcessorWrapper::wrapperSetParameter(const int& parameterIndex, const float& value)
{
    THROW_ERROR_IF_NO_PLUGIN

    if (parameterIndex < 0 || parameterIndex >= getParameters().size())
    {
        throw std::runtime_error("Parameter not found for index: " +
                                 std::to_string(parameterIndex));
    }

    auto pluginParameter = myPlugin->getParameters().getUnchecked(parameterIndex);
    pluginParameter->setValue(value);
    // pluginParameter->beginChangeGesture();
    // pluginParameter->setValueNotifyingHost(value);
    // pluginParameter->endChangeGesture();

    return ProcessorBase::setAutomationValByIndex(parameterIndex, value);
}

int PluginProcessorWrapper::wrapperGetPluginParameterSize()
{
    return int(PluginProcessor::getPluginParameterSize());
}

// Function to convert various strings to floats
/*
"42.3" -> 42.3f
"75%" -> 0.75f
"75" -> 75f
"-4" -> -4f
"78% (-20.1 dB)" -> -20.1f
"-20.1 dB" -> -20.1f
"35% (30.1 kHz)" -> 30100f
"30.1 kHz" -> 30100f
"35% (30.1 khz)" -> 30100f
"1% (500 Hz)" -> 500f
"500 Hz" -> 500f
"0% (-oo dB)" -> -std::numeric_limits<float>::infinity()
"100% (oo)" -> std::numeric_limits<float>::infinity()
"35% (30.1 khz)" -> 30100f
"10% (-18 db)" -> -18f
"-4 Oct" -> -4f
"500 ms" -> 0.5f
"500 MS" -> 0.5f
"500 millisec" -> 0.5f
"500 milliseconds" -> 0.5f
"5 sec" -> 5
*/
float stringToFloat(const std::string& input)
{
    // Regex to match numbers in parentheses or the whole string
    std::regex parenthesesRegex(R"(\(([^)]+)\))");
    std::smatch match;

    // Check for a number inside parentheses first
    std::string extractedInput = input;
    if (std::regex_search(input, match, parenthesesRegex))
    {
        extractedInput = match[1]; // Use the value inside parentheses
    }

    // Handle percentage (before checking for other numbers)
    std::regex percentRegex(R"((\d+)%$)");
    if (std::regex_search(input, match, percentRegex))
    {
        return std::stof(match[1]) / 100.0f;
    }

    // Regex for different number formats, including infinity (oo)
    std::regex numberRegex(R"(-?\d+(\.\d+)?(e[+-]?\d+)?|([+-]?\b(?:oo)\b))");
    if (std::regex_search(extractedInput, match, numberRegex))
    {
        std::string numberStr = match[0];

        // Handle special cases of infinity
        if (numberStr == "oo")
        {
            return std::numeric_limits<float>::infinity();
        }
        else if (numberStr == "-oo")
        {
            return -std::numeric_limits<float>::infinity();
        }

        // Convert the extracted number to float
        try
        {
            float result = std::stof(numberStr);

            // Check for frequency unit kHz or Hz, case insensitive
            std::regex khzRegex(R"((\d+(\.\d+)?)\s*[kK][hH][zZ])"); // Matches kHz or khz
            std::regex hzRegex(R"((\d+(\.\d+)?)\s*[hH][zZ])");      // Matches Hz or hz

            if (std::regex_search(extractedInput, match, khzRegex))
            {
                return result * 1000.0f; // Convert kHz to Hz
            }
            else if (std::regex_search(extractedInput, match, hzRegex))
            {
                return result; // Return Hz as is
            }

            std::regex msRegex(R"((\d+(\.\d+)?)\s*(?:[mM][sS]|millisec.*))"); // Matches ms or MS

            // Check for unit ms or MS
            if (std::regex_search(extractedInput, match, msRegex))
            {
                return result / 1000.0f; // Convert ms to seconds
            }
            else
            {
                return result; // Return the extracted number directly
            }
        }
        catch (...)
        {
            // If something goes wrong in conversion, return 0 as a fallback
            return 0.0f;
        }
    }

    // As a fallback, treat the input as a plain number
    try
    {
        return std::stof(extractedInput);
    }
    catch (...)
    {
        return 0.0f; // If no valid conversion was found, return 0
    }
}

std::string getTextForRawValue(AudioProcessorParameter* parameter, float rawValue)
{
    /*
    Some plugins don't respond properly to parameter->getText but do respond when
    a parameter's raw value is changed. This helper method works around this
    issue.
    */
    float originalValue = parameter->getValue();
    parameter->setValue(rawValue);
    std::string text = parameter->getCurrentValueAsText().toStdString();
    parameter->setValue(originalValue);

    return text;
}

// Function to attempt float extraction, fall back to string if exception occurs
ValueType extract_value(const std::string& s)
{
    try
    {
        // Attempt to convert the string to a float
        return std::stof(s);
    }
    catch (const std::invalid_argument&)
    {
        // If conversion fails, return the original string
        return s;
    }
}

std::map<std::pair<float, float>, ValueType> getParameterRange(AudioProcessorParameter* parameter,
                                                               int searchSteps, bool convert)
{
    // Adapted from pedalboard (GPL-3.0)
    // https://github.com/spotify/pedalboard/blob/ee16bb8805859fcd7e2fb7b00c8946666194774b/pedalboard/_pedalboard.py#L290-L318
    std::map<std::pair<float, float>, ValueType> ranges;
    std::string text;
    bool resultsLookIncorrect = false;

    float startOfRange = 0;
    text.clear();
    ranges.clear();

    for (int x = 0; x <= searchSteps; ++x)
    {
        float rawValue = static_cast<float>(x) / searchSteps;
        std::string tmpTextValue = getTextForRawValue(parameter, rawValue);

        if (text.empty())
        {
            text = tmpTextValue;
        }
        else if (tmpTextValue != text)
        {
            // End current range and start a new one
            ranges[{startOfRange, rawValue}] = text;
            text = tmpTextValue;
            startOfRange = rawValue;
        }
    }

    if (text.empty())
    {
        const std::string parameterName =
            parameter->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString();
        throw std::runtime_error("Plugin parameter '" + parameterName +
                                 "' failed to return a valid string for its value.");
    }

    ranges[{ranges.rbegin()->first.second, 1.0f}] = text; // Final range

    if (!convert)
    {
        return ranges;
    }

    std::map<std::pair<float, float>, ValueType> rangeFloat;
    for (auto& kv : ranges)
    {
        try
        {
            rangeFloat[kv.first] = stringToFloat(std::get<std::string>(kv.second));
        }
        catch (const std::invalid_argument& e)
        {
            return ranges;
        }
    }

    return rangeFloat;
}

std::map<std::pair<float, float>, ValueType>
PluginProcessor::getParameterValueRange(const int parameterIndex, int search_steps, bool convert)
{
    if (parameterIndex < 0 || parameterIndex >= getParameters().size())
    {
        throw std::runtime_error("Parameter not found for index: " +
                                 std::to_string(parameterIndex));
    }

    auto pluginParameter = myPlugin->getParameters().getUnchecked(parameterIndex);
    return getParameterRange(pluginParameter, search_steps, convert);
}

nb::list PluginProcessorWrapper::getPluginParametersDescription()
{
    THROW_ERROR_IF_NO_PLUGIN

    nb::list myList;

    // get the parameters as an AudioProcessorParameter array
    const Array<AudioProcessorParameter*>& processorParams = myPlugin->getParameters();
    for (int i = 0; i < processorParams.size(); i++)
    {
        std::string theName =
            (processorParams[i])->getName(DAW_PARAMETER_MAX_NAME_LENGTH).toStdString();
        std::string currentText =
            processorParams[i]
                ->getText(processorParams[i]->getValue(), DAW_PARAMETER_MAX_NAME_LENGTH)
                .toStdString();
        std::string label = processorParams[i]->getLabel().toStdString();

        std::string category;

        switch (processorParams[i]->getCategory())
        {
        case AudioProcessorParameter::Category::genericParameter:
            category = "genericParameter";
            break;
        case AudioProcessorParameter::Category::inputGain:
            category = "inputGain";
            break;
        case AudioProcessorParameter::Category::outputGain:
            category = "outputGain";
            break;
        case AudioProcessorParameter::Category::inputMeter:
            category = "inputMeter";
            break;
        case AudioProcessorParameter::Category::outputMeter:
            category = "outputMeter";
            break;
        case AudioProcessorParameter::Category::compressorLimiterGainReductionMeter:
            category = "compressorLimiterGainReductionMeter";
            break;
        case AudioProcessorParameter::Category::expanderGateGainReductionMeter:
            category = "expanderGateGainReductionMeter";
            break;
        case AudioProcessorParameter::Category::analysisMeter:
            category = "analysisMeter";
            break;
        case AudioProcessorParameter::Category::otherMeter:
            category = "otherMeter";
            break;
        default:
            category = "unknown";
            break;
        }

        nb::dict myDictionary;
        myDictionary["index"] = i;
        myDictionary["name"] = theName;
        myDictionary["numSteps"] = processorParams[i]->getNumSteps();
        myDictionary["isBoolean"] = processorParams[i]->isBoolean();
        myDictionary["isDiscrete"] = processorParams[i]->isDiscrete();
        myDictionary["label"] = label;
        myDictionary["category"] = category;

        myDictionary["text"] = currentText;
        myDictionary["currentValText"] = processorParams[i]->getCurrentValueAsText().toStdString();
        myDictionary["isMetaParameter"] = processorParams[i]->isMetaParameter();
        myDictionary["isAutomatable"] = processorParams[i]->isAutomatable();
        myDictionary["defaultValue"] = processorParams[i]->getDefaultValue();
        myDictionary["defaultValueText"] =
            processorParams[i]
                ->getText(processorParams[i]->getDefaultValue(), DAW_PARAMETER_MAX_NAME_LENGTH)
                .toStdString();
        myDictionary["min"] =
            processorParams[i]->getText(0.f, DAW_PARAMETER_MAX_NAME_LENGTH).toStdString();
        myDictionary["max"] =
            processorParams[i]->getText(1.f, DAW_PARAMETER_MAX_NAME_LENGTH).toStdString();

        std::vector<std::string> valueStrings;
        for (auto& valueString : processorParams[i]->getAllValueStrings())
        {
            valueStrings.push_back(valueString.toStdString());
        }

        myDictionary["valueStrings"] = valueStrings;
        myList.append(myDictionary);
    }

    return myList;
}

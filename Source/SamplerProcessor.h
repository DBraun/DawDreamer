#pragma once

#include "ProcessorBase.h"
#include "../Source/Sampler/Source/SamplerAudioProcessor.h"

class SamplerProcessor : public ProcessorBase
{
public:
    SamplerProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData, double sr, int blocksize) : ProcessorBase{ newUniqueName }, mySampleRate{sr}
    {
        createParameterLayout();
        sampler.setNonRealtime(true);
        sampler.setSample(inputData, mySampleRate);
    }

    SamplerProcessor(std::string newUniqueName, py::array_t<float, py::array::c_style | py::array::forcecast> input, double sr, int blocksize) : ProcessorBase{ newUniqueName }, mySampleRate{ sr }
    {
        createParameterLayout();
        sampler.setNonRealtime(true);
        setData(input);
    }

    ~SamplerProcessor() {}

    bool acceptsMidi() const { return true; }
    bool producesMidi() const { return true; }

    float
    wrapperGetParameter(int parameterIndex)
    {
        if (parameterIndex >= sampler.getNumParameters()) {
            std::cout << "Parameter not found for index: " << parameterIndex << std::endl;
            return 0.;
        }

        auto parameterName = sampler.getParameterName(parameterIndex);

        return ProcessorBase::getAutomationVal(parameterName.toStdString(), 0);
    }

    void
    wrapperSetParameter(const int paramIndex, const float value)
    {
        auto parameterName = sampler.getParameterName(paramIndex);

        if (parameterName == "Param") {
            return;
        }

        ProcessorBase::setAutomationVal(parameterName.toStdString(), value);
    }

    void
    setPlayHead(AudioPlayHead* newPlayHead)
    {
        AudioProcessor::setPlayHead(newPlayHead);
        sampler.setPlayHead(newPlayHead);
    }
   
    void
    prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        sampler.prepareToPlay(sampleRate, samplesPerBlock);
    }

    void reset() {
        sampler.reset();

        if (myMidiIterator) {
            delete myMidiIterator;
        }

        myMidiIterator = new MidiBuffer::Iterator(myMidiBuffer); // todo: deprecated.
        myMidiEventsDoRemain = myMidiIterator->getNextEvent(myMidiMessage, myMidiMessagePosition);
        myRenderMidiBuffer.clear();
    }

    void
    processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer)
    {
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

        sampler.processBlock(buffer, myRenderMidiBuffer);

        myRenderMidiBuffer.clear();

        ProcessorBase::processBlock(buffer, midiBuffer);
    }

    const juce::String getName() const { return "SamplerProcessor"; }

    void
    setData(py::array_t<float, py::array::c_style | py::array::forcecast> input) {
        float* input_ptr = (float*)input.data();

        std::vector<std::vector<float>> data = std::vector<std::vector<float>>(input.shape(0), std::vector<float>(input.shape(1)));

        for (int y = 0; y < input.shape(1); y++) {
            for (int x = 0; x < input.shape(0); x++) {
                data[x][y] = input_ptr[x * input.shape(1) + y];
            }
        }

        sampler.setSample(data, mySampleRate);
    }

    int
    getNumMidiEvents()
    {
        return myMidiBuffer.getNumEvents();
    };

    bool
    loadMidi(const std::string& path)
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
    clearMidi() {
        myMidiBuffer.clear();
    }

    bool
    addMidiNote(uint8  midiNote,
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

    std::string
    wrapperGetParameterName(int parameter)
    {
        return sampler.getParameterName(parameter).toStdString();
    }

    std::string
    wrapperGetParameterAsText(const int parameter)
    {
        return sampler.getParameterText(parameter).toStdString();
    }

    int
    wrapperGetPluginParameterSize()
    {
        return sampler.getNumParameters();
    }

    py::list
    getParametersDescription()
    {
        py::list myList;

        //get the parameters as an AudioProcessorParameter array
        const Array<AudioProcessorParameter*>& processorParams = sampler.getParameters();
        for (int i = 0; i < sampler.getNumParameters(); i++) {

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
 
        return myList;
    }

    void
    createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout blankLayout;

        // clear existing parameters in the layout?
        ValueTree blankState;
        myParameters.replaceState(blankState);

        int usedParameterAmount = 0;
        for (int i = 0; i < sampler.getNumParameters(); ++i)
        {
            auto parameterName = sampler.getParameterName(i);
            // Ensure the parameter is not unused.
            if (parameterName != "Param")
            {
                ++usedParameterAmount;

                myParameters.createAndAddParameter(std::make_unique<AutomateParameterFloat>(parameterName, parameterName, NormalisableRange<float>(0.f, 1.f), 0.f));
                // give it a valid single sample of automation.
                ProcessorBase::setAutomationVal(parameterName.toStdString(), sampler.getParameter(i));
            }
        }
    }

    void
    automateParameters() {

        AudioPlayHead::CurrentPositionInfo posInfo;
        getPlayHead()->getCurrentPosition(posInfo);

        for (int i = 0; i < sampler.getNumParameters(); i++) {

            auto theName = sampler.getParameterName(i);

            auto theParameter = ((AutomateParameterFloat*)myParameters.getParameter(theName));
            if (theParameter) {
                sampler.setParameterRawNotifyingHost(i, theParameter->sample(posInfo.timeInSamples));
            }
            else {
                std::cout << "Error automateParameters: " << theName << std::endl;
            }
        }
        
    }


private:

    double mySampleRate;

    SamplerAudioProcessor sampler;

    MidiBuffer myMidiBuffer;
    MidiBuffer myRenderMidiBuffer;
    MidiMessage myMidiMessage;
    int myMidiMessagePosition = -1;
    MidiBuffer::Iterator* myMidiIterator = nullptr;
    bool myIsMessageBetween = false;
    bool myMidiEventsDoRemain = false;
};
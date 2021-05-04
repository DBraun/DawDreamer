#pragma once

#include "ProcessorBase.h"
#include "../Source/Sampler/Source/SamplerAudioProcessor.h"

class SamplerProcessor : public ProcessorBase
{
public:
    SamplerProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData, double sr, int blocksize) : ProcessorBase{ newUniqueName }, mySampleRate{sr}
    {
        sampler.setNonRealtime(true);
        sampler.setSample(inputData, mySampleRate);
        
    }

    SamplerProcessor(std::string newUniqueName, py::array_t<float, py::array::c_style | py::array::forcecast> input, double sr, int blocksize) : ProcessorBase{ newUniqueName }, mySampleRate{ sr }
    {
        sampler.setNonRealtime(true);
        setData(input);
    }

    ~SamplerProcessor() {}

    bool acceptsMidi() const { return true; }
    bool producesMidi() const { return true; }

    void
    setParameter(const int paramIndex, const float value)
    {
        sampler.setParameterNotifyingHost(paramIndex, value);
    }

    void setPlayHead(AudioPlayHead* newPlayHead)
    {
        AudioProcessor::setPlayHead(newPlayHead);
        sampler.setPlayHead(newPlayHead);
    }
   
    void
        prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        sampler.prepareToPlay(sampleRate, samplesPerBlock);
        
        if (myMidiIterator) {
            delete myMidiIterator;
        }

        myMidiIterator = new MidiBuffer::Iterator(myMidiBuffer); // todo: deprecated.
        myMidiEventsDoRemain = myMidiIterator->getNextEvent(myMidiMessage, myMidiMessagePosition);
        myWriteIndex = 0;
        myRenderMidiBuffer.clear();
    }

    void
    processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
    {
        automateParameters(myPlayheadIndex);

        long long int start = myWriteIndex;
        long long int end = myWriteIndex + buffer.getNumSamples();
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

        myWriteIndex = end;

        myPlayheadIndex += buffer.getNumSamples();
    }

    void
    reset()
    {
        sampler.reset();

        myWriteIndex = 0;
        myPlayheadIndex = 0;
    }

    const juce::String getName() const { return "SamplerProcessor"; }

    void setData(py::array_t<float, py::array::c_style | py::array::forcecast> input) {
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

    std::string wrapperGetParameterName(int parameter)
    {
        return sampler.getParameterName(parameter).toStdString();
    }

    std::string wrapperGetParameterAsText(const int parameter)
    {
        return sampler.getParameterText(parameter).toStdString();
    }

    int wrapperGetPluginParameterSize()
    {
        return sampler.getNumParameters();
    }

    py::list getParametersDescription()
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


private:

    uint32_t myPlaybackIndex = 0;
    double mySampleRate;

    SamplerAudioProcessor sampler;

    MidiBuffer myMidiBuffer;
    MidiBuffer myRenderMidiBuffer;
    MidiMessage myMidiMessage;
    long long int myWriteIndex = 0;
    int myMidiMessagePosition = -1;
    MidiBuffer::Iterator* myMidiIterator = nullptr;
    bool myIsMessageBetween = false;
    bool myMidiEventsDoRemain = false;
};
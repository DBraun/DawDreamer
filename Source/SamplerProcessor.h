#pragma once

#include <filesystem>

#include "../Source/Sampler/Source/SamplerAudioProcessor.h"
#include "ProcessorBase.h"

class SamplerProcessor : public ProcessorBase {
 public:
  SamplerProcessor(std::string newUniqueName,
                   std::vector<std::vector<float>> inputData, double sr,
                   int blocksize)
      : ProcessorBase{newUniqueName}, mySampleRate{sr} {
    createParameterLayout();
    sampler.setNonRealtime(true);
    sampler.setSample(inputData, mySampleRate);
    setMainBusInputsAndOutputs(0, inputData.size());
  }

  SamplerProcessor(
      std::string newUniqueName,
      py::array_t<float, py::array::c_style | py::array::forcecast> input,
      double sr, int blocksize)
      : ProcessorBase{newUniqueName}, mySampleRate{sr} {
    createParameterLayout();
    sampler.setNonRealtime(true);
    setData(input);
    setMainBusInputsAndOutputs(0, (int)input.shape(0));
  }

  ~SamplerProcessor() {
    myMidiBufferQN.clear();
    myMidiBufferSec.clear();
    myRenderMidiBuffer.clear();
    myRecordedMidiSequence.clear();
    delete myMidiIteratorSec;
    delete myMidiIteratorQN;
  }

  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return true; }

  void setPlayHead(AudioPlayHead* newPlayHead) override {
    AudioProcessor::setPlayHead(newPlayHead);
    sampler.setPlayHead(newPlayHead);
  }

  void prepareToPlay(double sampleRate, int samplesPerBlock) override {
    sampler.prepareToPlay(sampleRate, samplesPerBlock);
  }

  void reset() override {
    sampler.reset();

    delete myMidiIteratorSec;
    myMidiIteratorSec =
        new MidiBuffer::Iterator(myMidiBufferSec);  // todo: deprecated.

    myMidiEventsDoRemainSec = myMidiIteratorSec->getNextEvent(
        myMidiMessageSec, myMidiMessagePositionSec);

    delete myMidiIteratorQN;
    myMidiIteratorQN =
        new MidiBuffer::Iterator(myMidiBufferQN);  // todo: deprecated.

    myMidiEventsDoRemainQN = myMidiIteratorQN->getNextEvent(
        myMidiMessageQN, myMidiMessagePositionQN);

    myRenderMidiBuffer.clear();

    myRecordedMidiSequence.clear();
    myRecordedMidiSequence.addEvent(juce::MidiMessage::midiStart());
    myRecordedMidiSequence.addEvent(
        juce::MidiMessage::timeSignatureMetaEvent(4, 4));
    myRecordedMidiSequence.addEvent(
        juce::MidiMessage::tempoMetaEvent(500 * 1000));
    myRecordedMidiSequence.addEvent(juce::MidiMessage::midiChannelMetaEvent(1));
    ProcessorBase::reset();
  }

  void processBlock(juce::AudioSampleBuffer& buffer,
                    juce::MidiBuffer& midiBuffer) override {
    auto posInfo = getPlayHead()->getPosition();

    buffer.clear();  // todo: why did this become necessary?
    midiBuffer.clear();
    myRenderMidiBuffer.clear();

    {
      auto start = *posInfo->getTimeInSamples();
      auto end = start + buffer.getNumSamples();

      myIsMessageBetweenSec =
          myMidiMessagePositionSec >= start && myMidiMessagePositionSec < end;
      while (myIsMessageBetweenSec && myMidiEventsDoRemainSec) {
        // steps for saving midi to file output
        auto messageCopy = MidiMessage(myMidiMessageSec);
        messageCopy.setTimeStamp(myMidiMessagePositionSec *
                                 (2400. / mySampleRate));
        if (!(messageCopy.isEndOfTrackMetaEvent() ||
              messageCopy.isTempoMetaEvent())) {
          myRecordedMidiSequence.addEvent(messageCopy);
        }

        // steps for playing MIDI
        myRenderMidiBuffer.addEvent(myMidiMessageSec,
                                    int(myMidiMessagePositionSec - start));
        myMidiEventsDoRemainSec = myMidiIteratorSec->getNextEvent(
            myMidiMessageSec, myMidiMessagePositionSec);
        myIsMessageBetweenSec =
            myMidiMessagePositionSec >= start && myMidiMessagePositionSec < end;
      }
    }

    {
      auto pulseStart = std::floor(*posInfo->getPpqPosition() * PPQN);
      auto pulseEnd = pulseStart + buffer.getNumSamples() *
                                       (*posInfo->getBpm() * PPQN) /
                                       (mySampleRate * 60.);

      myIsMessageBetweenQN = myMidiMessagePositionQN >= pulseStart &&
                             myMidiMessagePositionQN < pulseEnd;
      while (myIsMessageBetweenQN && myMidiEventsDoRemainQN) {
        // steps for saving midi to file output
        auto messageCopy = MidiMessage(myMidiMessageQN);
        messageCopy.setTimeStamp((*posInfo->getTimeInSeconds() +
                                  (myMidiMessagePositionQN - pulseStart) *
                                      (60. / (*posInfo->getBpm()) / PPQN)) *
                                 2400.);
        if (!(messageCopy.isEndOfTrackMetaEvent() ||
              messageCopy.isTempoMetaEvent())) {
          myRecordedMidiSequence.addEvent(messageCopy);
        }

        // steps for playing MIDI
        myRenderMidiBuffer.addEvent(
            myMidiMessageQN, int((myMidiMessagePositionQN - pulseStart) * 60. *
                                 mySampleRate / (PPQN * *posInfo->getBpm())));
        myMidiEventsDoRemainQN = myMidiIteratorQN->getNextEvent(
            myMidiMessageQN, myMidiMessagePositionQN);
        myIsMessageBetweenQN = myMidiMessagePositionQN >= pulseStart &&
                               myMidiMessagePositionQN < pulseEnd;
      }
    }

    sampler.processBlock(buffer, myRenderMidiBuffer);

    ProcessorBase::processBlock(buffer, midiBuffer);
  }

  const juce::String getName() const override { return "SamplerProcessor"; }

  void setData(
      py::array_t<float, py::array::c_style | py::array::forcecast> input) {
    float* input_ptr = (float*)input.data();

    std::vector<std::vector<float>> data = std::vector<std::vector<float>>(
        input.shape(0), std::vector<float>(input.shape(1)));

    for (int y = 0; y < input.shape(1); y++) {
      for (int x = 0; x < input.shape(0); x++) {
        data[x][y] = input_ptr[x * input.shape(1) + y];
      }
    }

    sampler.setSample(data, mySampleRate);
  }

  int getNumMidiEvents() {
    return myMidiBufferSec.getNumEvents() + myMidiBufferQN.getNumEvents();
  };

  bool loadMidi(const std::string& path, bool clearPrevious, bool isBeats,
                bool allEvents) {
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
    } else {
      auto timeFormat = midiFile.getTimeFormat();  // the ppqn (Ableton makes
                                                   // midi files with 96 ppqn)
      for (int t = 0; t < midiFile.getNumTracks(); t++) {
        const MidiMessageSequence* track = midiFile.getTrack(t);
        for (int i = 0; i < track->getNumEvents(); i++) {
          MidiMessage& m = track->getEventPointer(i)->message;

          if (allEvents || m.isNoteOff() || m.isNoteOn()) {
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

  void clearMidi() {
    myMidiBufferSec.clear();
    myMidiBufferQN.clear();
  }

  bool addMidiNote(uint8 midiNote, uint8 midiVelocity, const double noteStart,
                   const double noteLength, bool isBeats) {
    if (midiNote > 255) midiNote = 255;
    if (midiNote < 0) midiNote = 0;
    if (midiVelocity > 255) midiVelocity = 255;
    if (midiVelocity < 0) midiVelocity = 0;
    if (noteLength <= 0) {
      throw std::runtime_error("The note length must be greater than zero.");
    }

    // Get the note on midiBuffer.
    MidiMessage onMessage = MidiMessage::noteOn(1, midiNote, midiVelocity);

    MidiMessage offMessage = MidiMessage::noteOff(1, midiNote, midiVelocity);

    if (!isBeats) {
      auto startTime = noteStart * mySampleRate;
      onMessage.setTimeStamp(startTime);
      offMessage.setTimeStamp(startTime + noteLength * mySampleRate);
      myMidiBufferSec.addEvent(onMessage, (int)onMessage.getTimeStamp());
      myMidiBufferSec.addEvent(offMessage, (int)offMessage.getTimeStamp());
    } else {
      auto startTime = noteStart * PPQN;
      onMessage.setTimeStamp(startTime);
      offMessage.setTimeStamp(startTime + noteLength * PPQN);
      myMidiBufferQN.addEvent(onMessage, (int)onMessage.getTimeStamp());
      myMidiBufferQN.addEvent(offMessage, (int)offMessage.getTimeStamp());
    }

    return true;
  }

  void saveMIDI(std::string& savePath) {
    MidiFile file;

    // 30*80 = 2400, so that's why the MIDI messages had their
    // timestamp set to seconds*2400
    file.setSmpteTimeFormat(30, 80);

    File myFile(savePath);

    file.addTrack(myRecordedMidiSequence);

    juce::FileOutputStream stream(myFile);
    if (stream.openedOk()) {
      // overwrite existing file.
      stream.setPosition(0);
      stream.truncate();
    }
    file.writeTo(stream);
  }

  std::string wrapperGetParameterName(int parameter) {
    return sampler.getParameterName(parameter).toStdString();
  }

  std::string wrapperGetParameterAsText(const int parameter) {
    return sampler.getParameterText(parameter).toStdString();
  }

  int wrapperGetPluginParameterSize() { return sampler.getNumParameters(); }

  py::list getParametersDescription() {
    py::list myList;

    // get the parameters as an AudioProcessorParameter array
    const Array<AudioProcessorParameter*>& processorParams =
        sampler.getParameters();
    for (int i = 0; i < sampler.getNumParameters(); i++) {
      int maximumStringLength = 64;

      std::string theName =
          (processorParams[i])->getName(maximumStringLength).toStdString();
      std::string currentText =
          processorParams[i]
              ->getText(processorParams[i]->getValue(), maximumStringLength)
              .toStdString();
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

  void createParameterLayout() {
    juce::AudioProcessorParameterGroup group;

    for (int i = 0; i < sampler.getNumParameters(); ++i) {
      auto parameterName = sampler.getParameterName(i);
      group.addChild(std::make_unique<AutomateParameterFloat>(
          parameterName, parameterName, NormalisableRange<float>(0.f, 1.f),
          0.f));
    }

    this->setParameterTree(std::move(group));

    for (int i = 0; i < sampler.getNumParameters(); ++i) {
      // give it a valid single sample of automation.
      ProcessorBase::setAutomationValByIndex(i, sampler.getParameter(i));
    }
  }

  void automateParameters(AudioPlayHead::PositionInfo& posInfo,
                          int numSamples) override {
    auto allParameters = this->getParameters();

    for (int i = 0; i < sampler.getNumParameters(); i++) {
      auto theParameter =
          (AutomateParameterFloat*)allParameters.getUnchecked(i);
      sampler.setParameterRawNotifyingHost(i, theParameter->sample(posInfo));
    }
  }

 private:
  double mySampleRate;

  SamplerAudioProcessor sampler;

  MidiBuffer myMidiBufferQN;
  MidiBuffer myMidiBufferSec;

  MidiBuffer myRenderMidiBuffer;

  MidiMessage myMidiMessageQN;
  MidiMessage myMidiMessageSec;

  int myMidiMessagePositionQN = -1;
  int myMidiMessagePositionSec = -1;

  MidiBuffer::Iterator* myMidiIteratorQN = nullptr;
  MidiBuffer::Iterator* myMidiIteratorSec = nullptr;

  bool myIsMessageBetweenQN = false;
  bool myIsMessageBetweenSec = false;

  bool myMidiEventsDoRemainQN = false;
  bool myMidiEventsDoRemainSec = false;

  MidiMessageSequence myRecordedMidiSequence;
};

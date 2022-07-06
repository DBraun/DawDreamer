#pragma once

#include "ProcessorBase.h"

#ifdef BUILD_DAWDREAMER_FAUST

#include "faust/dsp/poly-llvm-dsp.h"
#include "faust/dsp/poly-interpreter-dsp.h"
#include "generator/libfaust.h"
#include "faust/gui/APIUI.h"
#include "faust/gui/MidiUI.h"
#include "faust/gui/SoundUI.h"
#include "faust/midi/rt-midi.h"
#include "TMutex.h"

#include "faust/dsp/libfaust-box.h"
#include "faust/dsp/libfaust-signal.h"
#include "faust/dsp/llvm-dsp.h"
//#include "faust/dsp/interpreter-dsp.h"
//#include "faust/dsp/poly-dsp.h"
#include "faust/misc.h"
#include "faust/export.h"

#include <iostream>
#include <map>


class MySoundUI : public SoundUI {

public:

    virtual void addSoundfile(const char* label, const char* filename, Soundfile** sf_zone) {
        // Parse the possible list
        std::string saved_url_real = std::string(label);
        if (fSoundfileMap.find(saved_url_real) == fSoundfileMap.end()) {
            // If failure, use 'defaultsound'
            std::cerr << "addSoundfile : soundfile for " << label << " cannot be created !" << std::endl;
            *sf_zone = defaultsound;
            return;
        }

        // Get the soundfile
        *sf_zone = fSoundfileMap[saved_url_real];
    }

    virtual void addSoundfileFromBuffers(const char* label, std::vector<AudioSampleBuffer> buffers, int sample_rate)
    {
        // Parse the possible list
        std::string saved_url_real = std::string(label);
        if (fSoundfileMap.find(saved_url_real) == fSoundfileMap.end()) {

            int total_length = 0;
            int numChannels = 1;  // start with at least 1 channel. This may increase due to code below.

            for (auto& buffer : buffers) {
                total_length += buffer.getNumSamples();
                numChannels = std::max(numChannels, buffer.getNumChannels());
            }

            total_length += (MAX_SOUNDFILE_PARTS - buffers.size()) * BUFFER_SIZE;

            Soundfile* soundfile = new Soundfile(numChannels, total_length, MAX_CHAN, (int) buffers.size(), false);

            // Manually fill in the soundfile:
            // The following code is a modification of SoundfileReader::createSoundfile and SoundfileReader::readFile

            int offset = 0;

            int i = 0;
            for (auto& buffer : buffers) {

                int numSamples = buffer.getNumSamples();

                soundfile->fLength[i] = numSamples;
                soundfile->fSR[i] = sample_rate;
                soundfile->fOffset[i] = offset;

                void* tmpBuffers = alloca(soundfile->fChannels * sizeof(float*));
                soundfile->getBuffersOffsetReal<float>(tmpBuffers, offset);

                for (int chan = 0; chan < buffer.getNumChannels(); chan++) {
                    for (int sample = 0; sample < numSamples; sample++) {
                        // todo: don't assume float
                        // todo: use memcpy or similar to be faster
                        static_cast<float**>(soundfile->fBuffers)[chan][offset + sample] = buffer.getSample(chan, sample);
                    }
                }

                offset += soundfile->fLength[i];
                i++;
            }

            // Complete with empty parts
            for (auto i = (int) buffers.size(); i < MAX_SOUNDFILE_PARTS; i++) {
                soundfile->emptyFile(i, offset);
            }

            // Share the same buffers for all other channels so that we have max_chan channels available
            soundfile->shareBuffers(numChannels, MAX_CHAN);

            fSoundfileMap[saved_url_real] = soundfile;
        }
    }
};


struct SigWrapper {
    CTree *ptr;
    SigWrapper(CTree *ptr) : ptr{ptr} {}
    SigWrapper(float val) : ptr{sigReal(val)} {}  // todo: this ignores createLibContext();
    SigWrapper(int val) : ptr{sigInt(val)} {}  // todo: this ignores createLibContext();
    operator CTree *() { return ptr; }
};


struct BoxWrapper {
    CTree *ptr;
    BoxWrapper(CTree *ptr) : ptr{ptr} {}
    BoxWrapper(float val) : ptr{boxReal(val)} {}  // todo: this ignores createLibContext();
    BoxWrapper(int val) : ptr{boxInt(val)} {}  // todo: this ignores createLibContext();
    operator CTree *() { return ptr; }
};


class FaustProcessor : public ProcessorBase
{
public:
        
    FaustProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock);
    ~FaustProcessor();

    bool canApplyBusesLayout(const juce::AudioProcessor::BusesLayout& layout) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    
    int
    getTotalNumOutputChannels() override {
        if (!m_compileState) {
            this->compile();
        }
        return ProcessorBase::getTotalNumOutputChannels();
    }
    
    int
    getTotalNumInputChannels() override {
        if (!m_compileState) {
            this->compile();
        }
        return ProcessorBase::getTotalNumInputChannels();
    }

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override;

    bool acceptsMidi() const override { return false; } // todo: faust should be able to play MIDI.
    bool producesMidi() const override { return false; }

    void reset() override;

    void createParameterLayout();  // NB: this is different from other processors because it's called after a Faust DSP file is compiled.

    const juce::String getName() const override { return "FaustProcessor"; }

    void automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) override;
    bool setAutomation(std::string parameterName, py::array input, std::uint32_t ppqn) override;

    // faust stuff
    void clear();
    bool compile();
    bool setDSPString(const std::string& code);
    bool setDSPFile(const std::string& path);
    bool setParamWithIndex(const int index, float p);
    float getParamWithIndex(const int index);
    float getParamWithPath(const std::string& n);
    std::string code();
    bool isCompiled() { return bool(m_compileState); };

    py::list getPluginParametersDescription();

    void setNumVoices(int numVoices);
    int getNumVoices();

    void setGroupVoices(bool groupVoices);
    int getGroupVoices();

    void setAutoImport(const std::string& s) { m_autoImport = s; }
    std::string getAutoImport() { return m_autoImport; }

    bool loadMidi(const std::string& path, bool clearPrevious, bool isBeats, bool allEvents);

    void clearMidi();

    int getNumMidiEvents();

    bool addMidiNote(const uint8  midiNote,
        const uint8  midiVelocity,
        const double noteStart,
        const double noteLength,
        bool isBeats);

    void setSoundfiles(py::dict);

    double getReleaseLength();

    void setReleaseLength(double sec);

    void setFaustLibrariesPath(std::string faustLibrariesPath) {
        m_faustLibrariesPath = faustLibrariesPath;
    }

    std::string getFaustLibrariesPath() {
        return m_faustLibrariesPath;
    }

    std::map<std::string, std::vector<juce::AudioSampleBuffer>> m_SoundfileMap;
    
    void saveMIDI(std::string& savePath);

private:

    double mySampleRate;

    std::string getPathToFaustLibraries();
    
    enum CompileState { kNotCompiled, kMono, kPoly, kSignalMono, kSignalPoly };
    
    CompileState m_compileState;

protected:

    llvm_dsp_factory* m_factory = nullptr;
    llvm_dsp_poly_factory* m_poly_factory = nullptr;

    dsp* m_dsp = nullptr;
    dsp_poly* m_dsp_poly = nullptr;

    APIUI* m_ui = nullptr;
    MySoundUI* m_soundUI = nullptr;

    rt_midi m_midi_handler;

    int m_numInputChannels = 0;
    int m_numOutputChannels = 0;

    double m_releaseLengthSec = 0.5;

    std::string m_autoImport;
    std::string m_code;
    std::string m_faustLibrariesPath = "";

    int m_nvoices = 0;
    bool m_groupVoices = true;

    MidiBuffer myMidiBufferQN;
    MidiBuffer myMidiBufferSec;
    
    MidiMessageSequence myRecordedMidiSequence; // for fetching by user later.

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

    juce::AudioSampleBuffer oneSampleInBuffer;
    juce::AudioSampleBuffer oneSampleOutBuffer;

    std::map<int, int> m_map_juceIndex_to_faustIndex;
    std::map<int, std::string> m_map_juceIndex_to_parAddress;

    TMutex guiUpdateMutex;
    
    std::string getTarget();
    
    // public libfaust signal API
public:
    SigWrapper getSigInt(int val) { return SigWrapper(sigInt(val)); }
    
    SigWrapper getSigReal(double val) { return SigWrapper(sigReal(val)); }

    SigWrapper getSigInput(int index) { return SigWrapper(sigInput(index)); }
    
    SigWrapper getSigDelay(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigDelay(sig1, sig2)); }
    
    SigWrapper getSigIntCast(SigWrapper &sig1) { return SigWrapper(sigIntCast(sig1)); }
    SigWrapper getSigFloatCast(SigWrapper &sig1) { return SigWrapper(sigFloatCast(sig1)); }
    
    SigWrapper getSigReadOnlyTable(SigWrapper &n, SigWrapper &init, SigWrapper& ridx) {
        return SigWrapper(sigReadOnlyTable(n, init, sigIntCast(ridx)));
    }
    
    SigWrapper getSigWriteReadTable(SigWrapper &n, SigWrapper &init, SigWrapper& widx, SigWrapper& wsig, SigWrapper& ridx) {
        return SigWrapper(sigWriteReadTable(n, init, sigIntCast(widx), sigIntCast(wsig), sigIntCast(ridx)));
    }
    
    std::vector<SigWrapper> getSigWaveform(std::vector<float> vals) {
        tvec waveform;
        for (auto& val : vals) {
            waveform.push_back(sigReal(val));
        }
        auto mySigWaveform = sigWaveform(waveform);

        auto result = std::vector<SigWrapper>{
            SigWrapper(sigInt((int)waveform.size())),
            SigWrapper(mySigWaveform)
        };
        
        return result;
    }
    
    std::vector<SigWrapper> getSigSoundfile(std::string &name, SigWrapper& rdx, SigWrapper& chan, SigWrapper &part) {
        // Soundfile definition
        Signal sf = sigSoundfile(name);
        Signal partInt = sigIntCast(part);
        // Wrapped index to avoid reading outside the buffer
        Signal wridx = sigIntCast(sigMax(sigInt(0),
                                         sigMin(sigIntCast(rdx),
                                                sigSub(sigSoundfileLength(sf, partInt),
                                                       sigInt(1)))));

        auto result = std::vector<SigWrapper>{
            SigWrapper(sigSoundfileLength(sf, partInt)),
            SigWrapper(sigSoundfileRate(sf, partInt)),
            SigWrapper(sigSoundfileBuffer(sf, sigIntCast(chan), partInt, wridx))
        };
        
        return result;
    }
    
    SigWrapper getSigSelect2(SigWrapper& selector, SigWrapper &sig1, SigWrapper &sig2) {
        return SigWrapper(sigSelect2(selector, sig1, sig2));
    }
    
    SigWrapper getSigSelect3(SigWrapper& selector, SigWrapper &sig1, SigWrapper &sig2, SigWrapper &sig3) {
        return SigWrapper(sigSelect3(selector, sig1, sig2, sig3));
    }
    
    SigWrapper getSigFConst(SType type, const std::string& name, const std::string& file) {
        return SigWrapper(sigFConst(type, name, file));
    }

    SigWrapper getSigFVar(SType type, const std::string& name, const std::string& file) {
        return SigWrapper(sigFVar(type, name, file));
    }

    SigWrapper getSigBinOp(SOperator op, SigWrapper &sig1, SigWrapper &sig2) {
        return SigWrapper(sigBinOp(op, sig1, sig2));
    }

    SigWrapper getSigAdd(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigAdd(sig1, sig2)); }
    SigWrapper getSigSub(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigSub(sig1, sig2)); }
    SigWrapper getSigMul(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigMul(sig1, sig2)); }
    SigWrapper getSigDiv(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigDiv(sig1, sig2)); }
    SigWrapper getSigRem(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigRem(sig1, sig2)); }
    
    SigWrapper getSigLeftShift(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigLeftShift(sig1, sig2)); }
    SigWrapper getSigLRightShift(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigLRightShift(sig1, sig2)); }
    SigWrapper getSigARightShift(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigARightShift(sig1, sig2)); }
        
    SigWrapper getSigGT(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigGT(sig1, sig2)); }
    SigWrapper getSigLT(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigLT(sig1, sig2)); }
    SigWrapper getSigGE(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigGE(sig1, sig2)); }
    SigWrapper getSigLE(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigLE(sig1, sig2)); }
    SigWrapper getSigEQ(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigEQ(sig1, sig2)); }
    SigWrapper getSigNE(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigNE(sig1, sig2)); }

    SigWrapper getSigAND(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigAND(sig1, sig2)); }
    SigWrapper getSigOR(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigOR(sig1, sig2)); }
    SigWrapper getSigXOR(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigXOR(sig1, sig2)); }
        
    SigWrapper getSigAbs(SigWrapper &sig1) { return SigWrapper(sigAbs(sig1)); }
    SigWrapper getSigAcos(SigWrapper &sig1) { return SigWrapper(sigAcos(sig1)); }
    SigWrapper getSigTan(SigWrapper &sig1) { return SigWrapper(sigTan(sig1)); }
    SigWrapper getSigSqrt(SigWrapper &sig1) { return SigWrapper(sigSqrt(sig1)); }
    SigWrapper getSigSin(SigWrapper &sig1) { return SigWrapper(sigSin(sig1)); }
    SigWrapper getSigRint(SigWrapper &sig1) { return SigWrapper(sigRint(sig1)); }
    SigWrapper getSigLog(SigWrapper &sig1) { return SigWrapper(sigLog(sig1)); }
    SigWrapper getSigLog10(SigWrapper &sig1) { return SigWrapper(sigLog10(sig1)); }
    SigWrapper getSigFloor(SigWrapper &sig1) { return SigWrapper(sigFloor(sig1)); }
    SigWrapper getSigExp(SigWrapper &sig1) { return SigWrapper(sigExp(sig1)); }
    SigWrapper getSigExp10(SigWrapper &sig1) { return SigWrapper(sigExp10(sig1)); }
    SigWrapper getSigCos(SigWrapper &sig1) { return SigWrapper(sigCos(sig1)); }
    SigWrapper getSigCeil(SigWrapper &sig1) { return SigWrapper(sigCeil(sig1)); }
    SigWrapper getSigAtan(SigWrapper &sig1) { return SigWrapper(sigAtan(sig1)); }
    SigWrapper getSigAsin(SigWrapper &sig1) { return SigWrapper(sigAsin(sig1)); }
    
    SigWrapper getSigRemainder(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigRemainder(sig1, sig2)); }
    SigWrapper getSigPow(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigPow(sig1, sig2)); }
    SigWrapper getSigMin(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigMin(sig1, sig2)); }
    SigWrapper getSigMax(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigMax(sig1, sig2)); }
    SigWrapper getSigFmod(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigFmod(sig1, sig2)); }
    SigWrapper getSigAtan2(SigWrapper &sig1, SigWrapper &sig2) { return SigWrapper(sigAtan2(sig1, sig2)); }

    SigWrapper getSigSelf() { return SigWrapper(sigSelf()); }
    
    SigWrapper getSigRecursion(SigWrapper &sig1) { return SigWrapper(sigRecursion(sig1)); }

    SigWrapper getSigButton(std::string &label) { return SigWrapper(sigButton(label)); }
    
    SigWrapper getSigCheckbox(std::string &label) { return SigWrapper(sigCheckbox(label)); }

    SigWrapper getSigVSlider(std::string &label, SigWrapper &sigInit, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sigStep)
        { return SigWrapper(sigVSlider(label, sigInit, sigMin, sigMax, sigStep)); }
    
    SigWrapper getSigHSlider(std::string &label, SigWrapper &sigInit, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sigStep)
        { return SigWrapper(sigHSlider(label, sigInit, sigMin, sigMax, sigStep)); }
    
    SigWrapper getSigNumEntry(std::string &label, SigWrapper &sigInit, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sigStep)
        { return SigWrapper(sigNumEntry(label, sigInit, sigMin, sigMax, sigStep)); }
    
    SigWrapper getSigVBargraph(std::string &label, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sig)
        { return SigWrapper(sigVBargraph(label, sigMin, sigMax, sig)); }
    
    SigWrapper getSigHBargraph(std::string &label, SigWrapper &sigMin, SigWrapper &sigMax, SigWrapper &sig)
        { return SigWrapper(sigHBargraph(label, sigMin, sigMax, sig)); }
    
    SigWrapper getSigAttach(SigWrapper &s1, SigWrapper &s2)
        { return SigWrapper(sigAttach(s1, s2)); }
    
    SigWrapper getSigSampleRate() { return SigWrapper(sigMin(sigReal(192000.0),
                                                             sigMax(sigReal(1.0),
                                                                    sigFConst(SType::kSInt, "fSamplingFreq", "<math.h>")))); }
    SigWrapper getSigBufferSize() { return SigWrapper(sigFVar(SType::kSInt, "count", "<math.h>")); }
    
    
    void compileSignals(const std::string& name,
                        std::vector<SigWrapper> &wrappers,
                        std::optional<std::vector<std::string>> in_argv);

    // box API

    BoxWrapper getBoxInt(int val) { return BoxWrapper(boxInt(val)); }
    
    BoxWrapper getBoxReal(double val) { return BoxWrapper(boxReal(val)); }

    BoxWrapper getBoxWire() { return BoxWrapper(boxWire()); }
    
    BoxWrapper getBoxCut() { return BoxWrapper(boxCut()); }
    
    BoxWrapper getBoxSeq(BoxWrapper &box1, BoxWrapper &box2) { return BoxWrapper(boxSeq(box1, box2)); }
    
    BoxWrapper getBoxPar(BoxWrapper &box1, BoxWrapper &box2) { return BoxWrapper(boxPar(box1, box2)); }
    BoxWrapper getBoxPar3(BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3) { return BoxWrapper(boxPar3(box1, box2, box3)); }
    BoxWrapper getBoxPar4(BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3, BoxWrapper &box4) { return BoxWrapper(boxPar4(box1, box2, box3, box4)); }
    BoxWrapper getBoxPar5(BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3, BoxWrapper &box4, BoxWrapper &box5) { return BoxWrapper(boxPar5(box1, box2, box3, box4, box5)); }

    BoxWrapper getBoxSplit(BoxWrapper &box1, BoxWrapper &box2) { return BoxWrapper(boxSplit(box1, box2)); }
    BoxWrapper getBoxMerge(BoxWrapper &box1, BoxWrapper &box2) { return BoxWrapper(boxMerge(box1, box2)); }
    
    BoxWrapper getBoxRoute(BoxWrapper &box1, BoxWrapper &box2, BoxWrapper &box3) { return BoxWrapper(boxRoute(box1, box2, box3)); }
    
    BoxWrapper getBoxDelay(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
        if (box1.has_value() && box2.has_value()) {
            return BoxWrapper(boxDelay(*box1, *box2));
        } else {
            return BoxWrapper(boxDelay());
        }
    }
    
    BoxWrapper getBoxIntCast(std::optional<BoxWrapper> box1) {
        if (box1.has_value()) {
            return BoxWrapper(boxIntCast(*box1));
        } else {
            return BoxWrapper(boxIntCast());
        }
    }

    BoxWrapper getBoxFloatCast(std::optional<BoxWrapper> box1) {
        if (box1.has_value()) {
            return BoxWrapper(boxFloatCast(*box1));
        } else {
            return BoxWrapper(boxFloatCast());
        }
    }
    BoxWrapper getBoxReadOnlyTable(std::optional<BoxWrapper> n, std::optional<BoxWrapper> init, std::optional<BoxWrapper> ridx) {
        if (n.has_value() && init.has_value() && ridx.has_value()) {
            return BoxWrapper(boxReadOnlyTable(boxIntCast(*n), *init, boxIntCast(*ridx)));
        } else {
            return BoxWrapper(boxReadOnlyTable());
        }
    }

    BoxWrapper getBoxWriteReadTable(std::optional<BoxWrapper> n, std::optional<BoxWrapper> init, std::optional<BoxWrapper> widx, std::optional<BoxWrapper> wsig, std::optional<BoxWrapper> ridx) {
        if (n.has_value() && init.has_value() && widx.has_value() && wsig.has_value() && ridx.has_value()) {
            return BoxWrapper(boxWriteReadTable(boxIntCast(*n), *init, boxIntCast(*widx), boxIntCast(*wsig), boxIntCast(*ridx)));
        } else {
            return BoxWrapper(boxWriteReadTable());
        }
    }
    
    BoxWrapper getBoxWaveform(std::vector<float> vals) {
        tvec waveform;
        for (auto& val : vals) {
            waveform.push_back(boxReal(val));
        }
        return BoxWrapper(boxWaveform(waveform));
    }

    BoxWrapper getBoxSoundfile(std::string &label, BoxWrapper& chan, std::optional<BoxWrapper> part, std::optional<BoxWrapper> rdx) {
        if (part.has_value() && rdx.has_value()) {
            return BoxWrapper(boxSoundfile(label, boxIntCast(chan), boxIntCast(*part), boxIntCast(*rdx)));
        } else {
            return BoxWrapper(boxSoundfile(label, chan));
        }
    }
    
    BoxWrapper getBoxSelect2(std::optional<BoxWrapper> selector, std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
        if (selector.has_value() && box1.has_value() && box2.has_value()) {
            return BoxWrapper(boxSelect2(*selector, *box1, *box2));
        } else {
            return BoxWrapper(boxSelect2());
        }
    }
        
    BoxWrapper getBoxSelect3(std::optional<BoxWrapper> selector, std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2, std::optional<BoxWrapper> box3) {
        if (selector.has_value() && box1.has_value() && box2.has_value()) {
            return BoxWrapper(boxSelect3(*selector, *box1, *box2, *box3));
        } else {
            return BoxWrapper(boxSelect3());
        }
    }
    
    BoxWrapper getBoxFConst(SType type, const std::string& name, const std::string& file) {
        return BoxWrapper(boxFConst(type, name, file));
    }

    BoxWrapper getBoxFVar(SType type, const std::string& name, const std::string& file) {
        return BoxWrapper(boxFVar(type, name, file));
    }

    BoxWrapper getBoxBinOp(SOperator op, std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) {
        if (box1.has_value() && box2.has_value()) {
            return BoxWrapper(boxBinOp(op, *box1, *box2));
        } else {
            return BoxWrapper(boxBinOp(op));
        }
    }

    BoxWrapper getBoxAdd(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxAdd(*box1, *box2)) : BoxWrapper(boxAdd()); }

    BoxWrapper getBoxSub(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxSub(*box1, *box2)) : BoxWrapper(boxSub()); }

    BoxWrapper getBoxMul(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxMul(*box1, *box2)) : BoxWrapper(boxMul()); }

    BoxWrapper getBoxDiv(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxDiv(*box1, *box2)) : BoxWrapper(boxDiv()); }

    BoxWrapper getBoxRem(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxRem(*box1, *box2)) : BoxWrapper(boxRem()); }

    BoxWrapper getBoxLeftShift(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxLeftShift(*box1, *box2)) : BoxWrapper(boxLeftShift()); }

    BoxWrapper getBoxLRightShift(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxLRightShift(*box1, *box2)) : BoxWrapper(boxLRightShift()); }

    BoxWrapper getBoxARightShift(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxARightShift(*box1, *box2)) : BoxWrapper(boxARightShift()); }
        
    BoxWrapper getBoxGT(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxGT(*box1, *box2)) : BoxWrapper(boxGT()); }
    BoxWrapper getBoxLT(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxLT(*box1, *box2)) : BoxWrapper(boxLT()); }
    BoxWrapper getBoxGE(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxGE(*box1, *box2)) : BoxWrapper(boxGE()); }
    BoxWrapper getBoxLE(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxLE(*box1, *box2)) : BoxWrapper(boxLE()); }
    BoxWrapper getBoxEQ(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxEQ(*box1, *box2)) : BoxWrapper(boxEQ()); }
    BoxWrapper getBoxNE(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxNE(*box1, *box2)) : BoxWrapper(boxNE()); }
    
    BoxWrapper getBoxAND(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxAND(*box1, *box2)) : BoxWrapper(boxAND()); }
    BoxWrapper getBoxOR(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxOR(*box1, *box2)) : BoxWrapper(boxOR()); }
    BoxWrapper getBoxXOR(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxXOR(*box1, *box2)) : BoxWrapper(boxXOR()); }
            
    BoxWrapper getBoxAbs(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxAbs(*box1) : boxAbs()); }
    BoxWrapper getBoxAcos(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxAcos(*box1) : boxAcos()); }
    BoxWrapper getBoxTan(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxTan(*box1) : boxTan()); }
    BoxWrapper getBoxSqrt(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxSqrt(*box1) : boxSqrt()); }
    BoxWrapper getBoxSin(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxSin(*box1) : boxSin()); }
    BoxWrapper getBoxRint(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxRint(*box1) : boxRint()); }
    BoxWrapper getBoxLog(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxLog(*box1) : boxLog()); }
    BoxWrapper getBoxLog10(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxLog10(*box1) : boxLog10()); }
    BoxWrapper getBoxFloor(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxFloor(*box1) : boxFloor()); }
    BoxWrapper getBoxExp(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxExp(*box1) : boxExp()); }
    BoxWrapper getBoxExp10(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxExp10(*box1) : boxExp10()); }
    BoxWrapper getBoxCos(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxCos(*box1) : boxCos()); }
    BoxWrapper getBoxCeil(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxCeil(*box1) : boxCeil()); }
    BoxWrapper getBoxAtan(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxAtan(*box1) : boxAtan()); }
    BoxWrapper getBoxAsin(std::optional<BoxWrapper> box1) { return BoxWrapper(box1.has_value() ? boxAsin(*box1) : boxAsin()); }
    
    BoxWrapper getBoxRemainder(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxRemainder(*box1, *box2)) : BoxWrapper(boxRemainder()); }
    BoxWrapper getBoxPow(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxPow(*box1, *box2)) : BoxWrapper(boxPow()); }
    BoxWrapper getBoxMin(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxMin(*box1, *box2)) : BoxWrapper(boxMin()); }
    BoxWrapper getBoxMax(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxMax(*box1, *box2)) : BoxWrapper(boxMax()); }
    BoxWrapper getBoxFmod(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxFmod(*box1, *box2)) : BoxWrapper(boxFmod()); }
    BoxWrapper getBoxAtan2(std::optional<BoxWrapper> box1, std::optional<BoxWrapper> box2) { return box1.has_value() && box2.has_value() ? BoxWrapper(boxAtan2(*box1, *box2)) : BoxWrapper(boxAtan2()); }
    
    BoxWrapper getBoxRec(BoxWrapper &box1, BoxWrapper &box2) { return BoxWrapper(boxRec(box1, box2)); }

    BoxWrapper getBoxButton(std::string &label) { return BoxWrapper(boxButton(label)); }
    
    BoxWrapper getBoxCheckbox(std::string &label) { return BoxWrapper(boxCheckbox(label)); }

    BoxWrapper getBoxVSlider(std::string &label, BoxWrapper &boxInit, BoxWrapper &boxMin, BoxWrapper &boxMax, BoxWrapper &boxStep)
        { return BoxWrapper(boxVSlider(label, boxInit, boxMin, boxMax, boxStep)); }
    
    BoxWrapper getBoxHSlider(std::string &label, BoxWrapper &boxInit, BoxWrapper &boxMin, BoxWrapper &boxMax, BoxWrapper &boxStep)
        { return BoxWrapper(boxHSlider(label, boxInit, boxMin, boxMax, boxStep)); }
    
    BoxWrapper getBoxNumEntry(std::string &label, BoxWrapper &boxInit, BoxWrapper &boxMin, BoxWrapper &boxMax, BoxWrapper &boxStep)
        { return BoxWrapper(boxNumEntry(label, boxInit, boxMin, boxMax, boxStep)); }
    
    BoxWrapper getBoxVBargraph(std::string &label, BoxWrapper &boxMin, BoxWrapper &boxMax, BoxWrapper &box)
        { return BoxWrapper(boxVBargraph(label, boxMin, boxMax, box)); }
    
    BoxWrapper getBoxHBargraph(std::string &label, BoxWrapper &boxMin, BoxWrapper &boxMax, BoxWrapper &box)
        { return BoxWrapper(boxHBargraph(label, boxMin, boxMax, box)); }
    
    BoxWrapper getBoxAttach(std::optional<BoxWrapper> s1, std::optional<BoxWrapper> s2)
        { return BoxWrapper((s1.has_value() && s2.has_value()) ? boxAttach(*s1, *s2) : boxAttach()); }
  
    // todo:
//    tvec getBoxToSignals(BoxWrapper& box1)
//        {
//        std::string error_msg;
//        auto signals = boxesToSignals(box1, error_msg);
//        return signals;
//    }
    
    BoxWrapper getBoxSampleRate() { return BoxWrapper(boxMin(boxReal(192000.0),
                                                             boxMax(boxReal(1.0),
                                                                    boxFConst(SType::kSInt, "fSamplingFreq", "<math.h>")))); }
    BoxWrapper getBoxBufferSize() { return BoxWrapper(boxFVar(SType::kSInt, "count", "<math.h>")); }

    void compileBox(const std::string& name,
                    BoxWrapper &box,
                    std::optional<std::vector<std::string>> in_argv);
    
    std::tuple<BoxWrapper, int, int> dspToBox(const std::string& dsp_content) {
        int inputs = 0;
        int outputs = 0;
        std::string error_msg = "";
        Box box = DSPToBoxes(dsp_content, inputs, outputs, error_msg);
        if (error_msg != "") {
            throw std::runtime_error(error_msg);
        }
        
        return std::tuple<BoxWrapper, int, int>(BoxWrapper(box), inputs, outputs);
        
    }
};


inline void create_bindings_for_faust_signal(py::module &m) {
    
    using arg = py::arg;
    using kw_only = py::kw_only;
    
    py::return_value_policy returnPolicy = py::return_value_policy::reference;
    
    // todo: for consistency, lookup these descriptions from source.cpp
    auto add_midi_description = "Add a single MIDI note whose note and velocity are integers between 0 and 127. By default, when `beats` is False, the start time and duration are measured in seconds, otherwise beats.";
    auto load_midi_description = "Load MIDI from a file. If `all_events` is True, then all events (not just Note On/Off) will be loaded. By default, when `beats` is False, notes will be converted to absolute times and will not be affected by the Render Engine's BPM. By default, `clear_previous` is True.";
    auto save_midi_description = "After rendering, you can save the MIDI to a file using absolute times (SMPTE format).";
    
    py::class_<SigWrapper>(m, "Signal")
        .def(py::init<float>(), arg("val"), "Init with a float")
        .def(py::init<int>(), arg("val"), "Init with an int")
    // todo: this ignores createLibContext()
        .def("__add__", [](const SigWrapper &s1, SigWrapper &s2) { return SigWrapper(sigAdd((SigWrapper&)s1, s2)); })
        .def("__add__", [](const SigWrapper &s1, float other) { return SigWrapper(sigAdd((SigWrapper&)s1, sigReal(other))); })
        .def("__radd__", [](const SigWrapper &s1, float other) { return SigWrapper(sigAdd((SigWrapper&)s1, sigReal(other))); })
        .def("__add__", [](const SigWrapper &s1, int other) { return SigWrapper(sigAdd((SigWrapper&)s1, sigInt(other))); })
        .def("__radd__", [](const SigWrapper &s1, int other) { return SigWrapper(sigAdd((SigWrapper&)s1, sigInt(other))); })
    
        .def("__sub__", [](const SigWrapper &s1, SigWrapper &s2) { return SigWrapper(sigSub((SigWrapper&)s1, s2)); })
        .def("__sub__", [](const SigWrapper &s1, float other) { return SigWrapper(sigSub((SigWrapper&)s1, sigReal(other))); })
        .def("__rsub__", [](const SigWrapper &s1, float other) { return SigWrapper(sigSub((SigWrapper&)s1, sigReal(other))); })
        .def("__sub__", [](const SigWrapper &s1, int other) { return SigWrapper(sigSub((SigWrapper&)s1, sigInt(other))); })
        .def("__rsub__", [](const SigWrapper &s1, int other) { return SigWrapper(sigSub((SigWrapper&)s1, sigInt(other))); })
    
        .def("__mul__", [](const SigWrapper &s1, SigWrapper &s2) { return SigWrapper(sigMul((SigWrapper&)s1, s2)); })
        .def("__mul__", [](const SigWrapper &s1, float other) { return SigWrapper(sigMul((SigWrapper&)s1, sigReal(other))); })
        .def("__rmul__", [](const SigWrapper &s1, float other) { return SigWrapper(sigMul((SigWrapper&)s1, sigReal(other))); })
        .def("__mul__", [](const SigWrapper &s1, int other) { return SigWrapper(sigMul((SigWrapper&)s1, sigInt(other))); })
        .def("__rmul__", [](const SigWrapper &s1, int other) { return SigWrapper(sigMul((SigWrapper&)s1, sigInt(other))); })
    
        .def("__truediv__", [](const SigWrapper &s1, SigWrapper &s2) { return SigWrapper(sigDiv((SigWrapper&)s1, s2)); })
        .def("__truediv__", [](const SigWrapper &s1, float other) { return SigWrapper(sigDiv((SigWrapper&)s1, sigReal(other))); })
        .def("__rtruediv__", [](const SigWrapper &s1, float other) { return SigWrapper(sigDiv((SigWrapper&)s1, sigReal(other))); })
        .def("__truediv__", [](const SigWrapper &s1, int other) { return SigWrapper(sigDiv((SigWrapper&)s1, sigInt(other))); })
        .def("__rtruediv__", [](const SigWrapper &s1, int other) { return SigWrapper(sigDiv((SigWrapper&)s1, sigInt(other))); })
    
        .def("__mod__", [](const SigWrapper &s1, SigWrapper &s2) { return SigWrapper(sigFmod((SigWrapper&)s1, s2)); })
        .def("__mod__", [](const SigWrapper &s1, float other) { return SigWrapper(sigFmod((SigWrapper&)s1, sigReal(other))); })
        .def("__mod__", [](const SigWrapper &s1, int other) { return SigWrapper(sigFmod((SigWrapper&)s1, sigInt(other))); })
    ;
    
    py::class_<BoxWrapper>(m, "Box")
        .def(py::init<float>(), arg("val"), "Init with a float")
        .def(py::init<int>(), arg("val"), "Init with an int")
    // todo: this ignores createLibContext()
        .def("__add__", [](const BoxWrapper &box1, BoxWrapper &box2) { return BoxWrapper(boxAdd((BoxWrapper&)box1, box2)); })
        .def("__add__", [](const BoxWrapper &box1, float other) { return BoxWrapper(boxAdd((BoxWrapper&)box1, boxReal(other))); })
        .def("__radd__", [](const BoxWrapper &box1, float other) { return BoxWrapper(boxAdd((BoxWrapper&)box1, boxReal(other))); })
        .def("__add__", [](const BoxWrapper &box1, int other) { return BoxWrapper(boxAdd((BoxWrapper&)box1, boxInt(other))); })
        .def("__radd__", [](const BoxWrapper &box1, int other) { return BoxWrapper(boxAdd((BoxWrapper&)box1, boxInt(other))); })
    
        .def("__sub__", [](const BoxWrapper &box1, BoxWrapper &box2) { return BoxWrapper(boxSub((BoxWrapper&)box1, box2)); })
        .def("__sub__", [](const BoxWrapper &box1, float other) { return BoxWrapper(boxSub((BoxWrapper&)box1, boxReal(other))); })
        .def("__rsub__", [](const BoxWrapper &box1, float other) { return BoxWrapper(boxSub((BoxWrapper&)box1, boxReal(other))); })
        .def("__sub__", [](const BoxWrapper &box1, int other) { return BoxWrapper(boxSub((BoxWrapper&)box1, boxInt(other))); })
        .def("__rsub__", [](const BoxWrapper &box1, int other) { return BoxWrapper(boxSub((BoxWrapper&)box1, boxInt(other))); })
    
        .def("__mul__", [](const BoxWrapper &box1, BoxWrapper &box2) { return BoxWrapper(boxMul((BoxWrapper&)box1, box2)); })
        .def("__mul__", [](const BoxWrapper &box1, float other) { return BoxWrapper(boxMul((BoxWrapper&)box1, boxReal(other))); })
        .def("__rmul__", [](const BoxWrapper &box1, float other) { return BoxWrapper(boxMul((BoxWrapper&)box1, boxReal(other))); })
        .def("__mul__", [](const BoxWrapper &box1, int other) { return BoxWrapper(boxMul((BoxWrapper&)box1, boxInt(other))); })
        .def("__rmul__", [](const BoxWrapper &box1, int other) { return BoxWrapper(boxMul((BoxWrapper&)box1, boxInt(other))); })
    
        .def("__truediv__", [](const BoxWrapper &box1, BoxWrapper &box2) { return BoxWrapper(boxDiv((BoxWrapper&)box1, box2)); })
        .def("__truediv__", [](const BoxWrapper &box1, float other) { return BoxWrapper(boxDiv((BoxWrapper&)box1, boxReal(other))); })
        .def("__rtruediv__", [](const BoxWrapper &box1, float other) { return BoxWrapper(boxDiv((BoxWrapper&)box1, boxReal(other))); })
        .def("__truediv__", [](const BoxWrapper &box1, int other) { return BoxWrapper(boxDiv((BoxWrapper&)box1, boxInt(other))); })
        .def("__rtruediv__", [](const BoxWrapper &box1, int other) { return BoxWrapper(boxDiv((BoxWrapper&)box1, boxInt(other))); })
    
        .def("__mod__", [](const BoxWrapper &box1, BoxWrapper &box2) { return BoxWrapper(boxFmod((BoxWrapper&)box1, box2)); })
        .def("__mod__", [](const BoxWrapper &box1, float other) { return BoxWrapper(boxFmod((BoxWrapper&)box1, boxReal(other))); })
        .def("__mod__", [](const BoxWrapper &box1, int other) { return BoxWrapper(boxFmod((BoxWrapper&)box1, boxInt(other))); })
    ;
    
    py::class_<FaustProcessor, ProcessorBase> faustProcessor(m, "FaustProcessor");
    
    faustProcessor
        .def("set_dsp", &FaustProcessor::setDSPFile, arg("filepath"), "Set the FAUST signal process with a file.")
        .def("set_dsp_string", &FaustProcessor::setDSPString, arg("faust_code"),
            "Set the FAUST signal process with a string containing FAUST code.")
        .def("compile", &FaustProcessor::compile, "Compile the FAUST object. You must have already set a dsp file path or dsp string.")
        .def_property("auto_import", &FaustProcessor::getAutoImport, &FaustProcessor::setAutoImport, "The auto import string. Default is `import(\"stdfaust.lib\");`")
        .def("get_parameters_description", &FaustProcessor::getPluginParametersDescription,
            "Get a list of dictionaries describing the parameters of the most recently compiled FAUST code.")
        .def("get_parameter", &FaustProcessor::getParamWithIndex, arg("param_index"))
        .def("get_parameter", &FaustProcessor::getParamWithPath, arg("parameter_path"))
        .def("set_parameter", &FaustProcessor::setParamWithIndex, arg("parameter_index"), arg("value"))
        .def("set_parameter", &FaustProcessor::setAutomationVal, arg("parameter_path"), arg("value"))
        .def_property_readonly("compiled", &FaustProcessor::isCompiled, "Did the most recent DSP code compile?")
        .def_property_readonly("code", &FaustProcessor::code, "Get the most recently compiled Faust DSP code.")
        .def_property("num_voices", &FaustProcessor::getNumVoices, &FaustProcessor::setNumVoices, "The number of voices for polyphony. Set to zero to disable polyphony. One or more enables polyphony.")
        .def_property("group_voices", &FaustProcessor::getGroupVoices, &FaustProcessor::setGroupVoices, "If grouped, all polyphonic voices will share the same parameters. This parameter only matters if polyphony is enabled.")
        .def_property("release_length", &FaustProcessor::getReleaseLength, &FaustProcessor::setReleaseLength, "If using polyphony, specifying the release length accurately can help avoid warnings about voices being stolen.")
        .def_property("faust_libraries_path", &FaustProcessor::getFaustLibrariesPath, &FaustProcessor::setFaustLibrariesPath, "Absolute path to directory containing your custom \".lib\" files containing Faust code.")
        .def_property_readonly("n_midi_events", &FaustProcessor::getNumMidiEvents, "The number of MIDI events stored in the buffer. \
Note that note-ons and note-offs are counted separately.")
        .def("load_midi", &FaustProcessor::loadMidi, arg("filepath"), kw_only(), arg("clear_previous")=true, arg("beats")=false, arg("all_events")=true, load_midi_description)
        .def("clear_midi", &FaustProcessor::clearMidi, "Remove all MIDI notes.")
        .def("add_midi_note", &FaustProcessor::addMidiNote,
            arg("note"), arg("velocity"), arg("start_time"), arg("duration"), kw_only(), arg("beats")=false, add_midi_description)
        .def("save_midi", &FaustProcessor::saveMIDI,
            arg("filepath"), save_midi_description)
        .def("set_soundfiles", &FaustProcessor::setSoundfiles, arg("soundfile_dict"), "Set the audio data that the FaustProcessor can use with the `soundfile` primitive.")
    
        .def("sigInt", &FaustProcessor::getSigInt, arg("val"), "Blah", returnPolicy)
        .def("sigReal", &FaustProcessor::getSigReal, arg("val"), "Blah", returnPolicy)
        .def("sigInput", &FaustProcessor::getSigInput, arg("index"), "Blah", returnPolicy)
        .def("sigDelay", &FaustProcessor::getSigDelay, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigIntCast", &FaustProcessor::getSigIntCast, arg("sig1"), "Blah", returnPolicy)
        .def("sigFloatCast", &FaustProcessor::getSigFloatCast, arg("sig1"), "Blah", returnPolicy)
    
        .def("sigReadOnlyTable", &FaustProcessor::getSigReadOnlyTable, arg("n"), arg("init"), arg("ridx"), "Blah", returnPolicy)
        .def("sigWriteReadTable", &FaustProcessor::getSigWriteReadTable, arg("n"), arg("init"), arg("widx"), arg("wsig"), arg("ridx"), "Blah", returnPolicy)

        .def("sigWaveform", &FaustProcessor::getSigWaveform, arg("vals"), "Blah", returnPolicy)
        .def("sigSoundfile", &FaustProcessor::getSigSoundfile, arg("filepath"), arg("sig_read_index"), arg("sig_chan"), arg("sig_part"), "Blah", returnPolicy)
    
        .def("sigSelect2", &FaustProcessor::getSigSelect2, arg("selector"), arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigSelect3", &FaustProcessor::getSigSelect3, arg("selector"), arg("sig1"), arg("sig2"), arg("sig3"), "Blah", returnPolicy)

        .def("sigFConst", &FaustProcessor::getSigFConst, arg("type"), arg("name"), arg("file"), "Blah", returnPolicy)
        .def("sigFVar", &FaustProcessor::getSigFVar, arg("type"), arg("name"), arg("file"), "Blah", returnPolicy)

        .def("sigBinOp", &FaustProcessor::getSigBinOp, arg("op"), arg("x"), arg("y"), "Blah", returnPolicy)

        .def("sigAdd", &FaustProcessor::getSigAdd, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigSub", &FaustProcessor::getSigSub, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigMul", &FaustProcessor::getSigMul, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigDiv", &FaustProcessor::getSigDiv, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigRem", &FaustProcessor::getSigRem, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
    
        .def("sigLeftShift", &FaustProcessor::getSigLeftShift, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigLRightShift", &FaustProcessor::getSigLRightShift, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigARightShift", &FaustProcessor::getSigARightShift, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigGT", &FaustProcessor::getSigGT, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigLT", &FaustProcessor::getSigLT, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigGE", &FaustProcessor::getSigGE, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigLE", &FaustProcessor::getSigLE, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigEQ", &FaustProcessor::getSigEQ, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigNE", &FaustProcessor::getSigNE, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigAND", &FaustProcessor::getSigAND, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigOR", &FaustProcessor::getSigOR, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigXOR", &FaustProcessor::getSigXOR, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigAbs", &FaustProcessor::getSigAbs, arg("sig1"), "Blah", returnPolicy)
        .def("sigAcos", &FaustProcessor::getSigAcos, arg("sig1"), "Blah", returnPolicy)
        .def("sigTan", &FaustProcessor::getSigTan, arg("sig1"), "Blah", returnPolicy)
        .def("sigSqrt", &FaustProcessor::getSigSqrt, arg("sig1"), "Blah", returnPolicy)
        .def("sigSin", &FaustProcessor::getSigSin, arg("sig1"), "Blah", returnPolicy)
        .def("sigRint", &FaustProcessor::getSigRint, arg("sig1"), "Blah", returnPolicy)
        .def("sigLog", &FaustProcessor::getSigLog, arg("sig1"), "Blah", returnPolicy)
        .def("sigLog10", &FaustProcessor::getSigLog10, arg("sig1"), "Blah", returnPolicy)
        .def("sigFloor", &FaustProcessor::getSigFloor, arg("sig1"), "Blah", returnPolicy)
        .def("sigExp", &FaustProcessor::getSigExp, arg("sig1"), "Blah", returnPolicy)
        .def("sigExp10", &FaustProcessor::getSigExp10, arg("sig1"), "Blah", returnPolicy)
        .def("sigCos", &FaustProcessor::getSigCos, arg("sig1"), "Blah", returnPolicy)
        .def("sigCeil", &FaustProcessor::getSigCeil, arg("sig1"), "Blah", returnPolicy)
        .def("sigAtan", &FaustProcessor::getSigAtan, arg("sig1"), "Blah", returnPolicy)
        .def("sigAsin", &FaustProcessor::getSigAsin, arg("sig1"), "Blah", returnPolicy)
    
        .def("sigRemainder", &FaustProcessor::getSigRemainder, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigPow", &FaustProcessor::getSigPow, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigMin", &FaustProcessor::getSigMin, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigMax", &FaustProcessor::getSigMax, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigFmod", &FaustProcessor::getSigFmod, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
        .def("sigAtan2", &FaustProcessor::getSigAtan2, arg("sig1"), arg("sig2"), "Blah", returnPolicy)

        .def("sigSelf", &FaustProcessor::getSigSelf, "Blah", returnPolicy)
        .def("sigRecursion", &FaustProcessor::getSigRecursion, arg("sig"), "Blah", returnPolicy)
    
        .def("sigButton", &FaustProcessor::getSigButton, arg("label"), "Blah", returnPolicy)
        .def("sigCheckbox", &FaustProcessor::getSigCheckbox, arg("label"), "Blah", returnPolicy)

        .def("sigVSlider", &FaustProcessor::getSigVSlider, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("sigHSlider", &FaustProcessor::getSigHSlider, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("sigNumEntry", &FaustProcessor::getSigNumEntry, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("sigVBargraph", &FaustProcessor::getSigVBargraph, arg("label"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("sigHBargraph", &FaustProcessor::getSigHBargraph, arg("label"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("sigAttach", &FaustProcessor::getSigAttach, arg("sig1"), arg("sig2"), "Blah", returnPolicy)
    
        .def("sigSampleRate", &FaustProcessor::getSigSampleRate, "Blah", returnPolicy)
        .def("sigBufferSize", &FaustProcessor::getSigBufferSize, "Blah", returnPolicy)

        .def("compile_signals", &FaustProcessor::compileSignals, arg("name"), arg("signal"), arg("argv")=py::none(), "Blah")
    
// box api
    
        .def("set_dsp", &FaustProcessor::setDSPFile, arg("filepath"), "Set the FAUST box process with a file.")
        .def("set_dsp_string", &FaustProcessor::setDSPString, arg("faust_code"),
           "Set the FAUST box process with a string containing FAUST code.")
        .def("compile", &FaustProcessor::compile, "Compile the FAUST object. You must have already set a dsp file path or dsp string.")
        .def_property("auto_import", &FaustProcessor::getAutoImport, &FaustProcessor::setAutoImport, "The auto import string. Default is `import(\"stdfaust.lib\");`")
        .def("get_parameters_description", &FaustProcessor::getPluginParametersDescription,
           "Get a list of dictionaries describing the parameters of the most recently compiled FAUST code.")
        .def("get_parameter", &FaustProcessor::getParamWithIndex, arg("param_index"))
        .def("get_parameter", &FaustProcessor::getParamWithPath, arg("parameter_path"))
        .def("set_parameter", &FaustProcessor::setParamWithIndex, arg("parameter_index"), arg("value"))
        .def("set_parameter", &FaustProcessor::setAutomationVal, arg("parameter_path"), arg("value"))
        .def_property_readonly("compiled", &FaustProcessor::isCompiled, "Did the most recent DSP code compile?")
        .def_property_readonly("code", &FaustProcessor::code, "Get the most recently compiled Faust DSP code.")
        .def_property("num_voices", &FaustProcessor::getNumVoices, &FaustProcessor::setNumVoices, "The number of voices for polyphony. Set to zero to disable polyphony. One or more enables polyphony.")
        .def_property("group_voices", &FaustProcessor::getGroupVoices, &FaustProcessor::setGroupVoices, "If grouped, all polyphonic voices will share the same parameters. This parameter only matters if polyphony is enabled.")
        .def_property("release_length", &FaustProcessor::getReleaseLength, &FaustProcessor::setReleaseLength, "If using polyphony, specifying the release length accurately can help avoid warnings about voices being stolen.")
        .def_property("faust_libraries_path", &FaustProcessor::getFaustLibrariesPath, &FaustProcessor::setFaustLibrariesPath, "Absolute path to directory containing your custom \".lib\" files containing Faust code.")
        .def_property_readonly("n_midi_events", &FaustProcessor::getNumMidiEvents, "The number of MIDI events stored in the buffer. \
        Note that note-ons and note-offs are counted separately.")
        .def("load_midi", &FaustProcessor::loadMidi, arg("filepath"), kw_only(), arg("clear_previous")=true, arg("beats")=false, arg("all_events")=true, load_midi_description)
        .def("clear_midi", &FaustProcessor::clearMidi, "Remove all MIDI notes.")
        .def("add_midi_note", &FaustProcessor::addMidiNote,
           arg("note"), arg("velocity"), arg("start_time"), arg("duration"), kw_only(), arg("beats")=false, add_midi_description)
        .def("save_midi", &FaustProcessor::saveMIDI,
           arg("filepath"), save_midi_description)
        .def("set_soundfiles", &FaustProcessor::setSoundfiles, arg("soundfile_dict"), "Set the audio data that the FaustProcessor can use with the `soundfile` primitive.")

        .def("boxInt", &FaustProcessor::getBoxInt, arg("val"), "Blah", returnPolicy)
        .def("boxReal", &FaustProcessor::getBoxReal, arg("val"), "Blah", returnPolicy)
        .def("boxWire", &FaustProcessor::getBoxWire, "Blah", returnPolicy)
        .def("boxCut", &FaustProcessor::getBoxCut, "Blah", returnPolicy)
    
        .def("boxSeq", &FaustProcessor::getBoxSeq, arg("box1"), arg("box2"), "The sequential composition of two blocks (e.g., A:B) expects: outputs(A)=inputs(B)", returnPolicy)

        .def("boxPar", &FaustProcessor::getBoxPar, arg("box1"), arg("box2"), "The parallel composition of two blocks (e.g., A,B). It places the two block-diagrams one on top of the other, without connections.", returnPolicy)
        .def("boxPar3", &FaustProcessor::getBoxPar3, arg("box1"), arg("box2"), arg("box3"), "The parallel composition of three blocks (e.g., A,B,C).", returnPolicy)
        .def("boxPar4", &FaustProcessor::getBoxPar4, arg("box1"), arg("box2"), arg("box3"), arg("box4"), "The parallel composition of four blocks (e.g., A,B,C,D).", returnPolicy)
        .def("boxPar5", &FaustProcessor::getBoxPar5, arg("box1"), arg("box2"), arg("box3"), arg("box4"), arg("box5"), "The parallel composition of five blocks (e.g., A,B,C,D,E).", returnPolicy)

        .def("boxSplit", &FaustProcessor::getBoxSplit, arg("box1"), arg("box2"), "The split composition (e.g., A<:B) operator is used to distribute the outputs of A to the inputs of B. The number of inputs of B must be a multiple of the number of outputs of A: outputs(A).k=inputs(B)", returnPolicy)
        .def("boxMerge", &FaustProcessor::getBoxMerge, arg("box1"), arg("box2"), "The merge composition (e.g., A:>B) is the dual of the split composition. The number of outputs of A must be a multiple of the number of inputs of B: outputs(A)=k.inputs(B)", returnPolicy)
             
        .def("boxRoute", &FaustProcessor::getBoxRoute, arg("box_n"), arg("box_m"), arg("box_r"), "Blah", returnPolicy)

        .def("boxDelay", &FaustProcessor::getBoxDelay, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)

        .def("boxIntCast", &FaustProcessor::getBoxIntCast, arg("box1")=py::none(), "Blah", returnPolicy)

        .def("boxFloatCast", &FaustProcessor::getBoxFloatCast, arg("box1")=py::none(), "Blah", returnPolicy)
    
        .def("boxReadOnlyTable", &FaustProcessor::getBoxReadOnlyTable, arg("n")=py::none(), arg("init")=py::none(), arg("ridx")=py::none(), "Blah", returnPolicy)
    
        .def("boxWriteReadTable", &FaustProcessor::getBoxWriteReadTable, arg("n")=py::none(), arg("init")=py::none(), arg("widx")=py::none(), arg("wsig")=py::none(), arg("ridx")=py::none(), "Blah", returnPolicy)

        .def("boxWaveform", &FaustProcessor::getBoxWaveform, arg("vals"), "Blah", returnPolicy)
        .def("boxSoundfile", &FaustProcessor::getBoxSoundfile, arg("filepath"), arg("chan"), arg("part")=py::none(), arg("ridx")=py::none(), "Blah", returnPolicy)

        .def("boxSelect2", &FaustProcessor::getBoxSelect2, arg("selector")=py::none(), arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)

        .def("boxSelect3", &FaustProcessor::getBoxSelect3, arg("selector")=py::none(), arg("box1")=py::none(), arg("box2")=py::none(), arg("box3")=py::none(), "Blah", returnPolicy)
    
        .def("boxFConst", &FaustProcessor::getBoxFConst, arg("type"), arg("name"), arg("file"), "Blah", returnPolicy)
        .def("boxFVar", &FaustProcessor::getBoxFVar, arg("type"), arg("name"), arg("file"), "Blah", returnPolicy)

        .def("boxBinOp", &FaustProcessor::getBoxBinOp, arg("op"), arg("x")=py::none(), arg("y")=py::none(), "Blah", returnPolicy)
    
        .def("boxAdd", &FaustProcessor::getBoxAdd, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxSub", &FaustProcessor::getBoxSub, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxMul", &FaustProcessor::getBoxMul, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxDiv", &FaustProcessor::getBoxDiv, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxRem", &FaustProcessor::getBoxRem, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)

        .def("boxLeftShift", &FaustProcessor::getBoxLeftShift, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxLRightShift", &FaustProcessor::getBoxLRightShift, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxARightShift", &FaustProcessor::getBoxARightShift, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)

        .def("boxGT", &FaustProcessor::getBoxGT, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxLT", &FaustProcessor::getBoxLT, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxGE", &FaustProcessor::getBoxGE, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxLE", &FaustProcessor::getBoxLE, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxEQ", &FaustProcessor::getBoxEQ, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxNE", &FaustProcessor::getBoxNE, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)

        .def("boxAND", &FaustProcessor::getBoxAND, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxOR", &FaustProcessor::getBoxOR, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxXOR", &FaustProcessor::getBoxXOR, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)

        .def("boxAbs", &FaustProcessor::getBoxAbs, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxAcos", &FaustProcessor::getBoxAcos, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxTan", &FaustProcessor::getBoxTan, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxSqrt", &FaustProcessor::getBoxSqrt, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxSin", &FaustProcessor::getBoxSin, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxRint", &FaustProcessor::getBoxRint, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxLog", &FaustProcessor::getBoxLog, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxLog10", &FaustProcessor::getBoxLog10, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxFloor", &FaustProcessor::getBoxFloor, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxExp", &FaustProcessor::getBoxExp, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxExp10", &FaustProcessor::getBoxExp10, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxCos", &FaustProcessor::getBoxCos, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxCeil", &FaustProcessor::getBoxCeil, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxAtan", &FaustProcessor::getBoxAtan, arg("box1")=py::none(), "Blah", returnPolicy)
        .def("boxAsin", &FaustProcessor::getBoxAsin, arg("box1")=py::none(), "Blah", returnPolicy)

        .def("boxRemainder", &FaustProcessor::getBoxRemainder, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxPow", &FaustProcessor::getBoxPow, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxMin", &FaustProcessor::getBoxMin, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxMax", &FaustProcessor::getBoxMax, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxFmod", &FaustProcessor::getBoxFmod, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)
        .def("boxAtan2", &FaustProcessor::getBoxAtan2, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)

        .def("boxRec", &FaustProcessor::getBoxRec, arg("box1"), arg("box2"), "The recursive composition (e.g., A~B) is used to create cycles in the block-diagram in order to express recursive computations. It is the most complex operation in terms of connections: outputs(A)≥inputs(B) and inputs(A)≥outputs(B)", returnPolicy)

        .def("boxButton", &FaustProcessor::getBoxButton, arg("label"), "Blah", returnPolicy)
        .def("boxCheckbox", &FaustProcessor::getBoxCheckbox, arg("label"), "Blah", returnPolicy)

        .def("boxVSlider", &FaustProcessor::getBoxVSlider, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("boxHSlider", &FaustProcessor::getBoxHSlider, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("boxNumEntry", &FaustProcessor::getBoxNumEntry, arg("label"), arg("init"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("boxVBargraph", &FaustProcessor::getBoxVBargraph, arg("label"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)
        .def("boxHBargraph", &FaustProcessor::getBoxHBargraph, arg("label"), arg("min"), arg("max"), arg("step"), "Blah", returnPolicy)

        .def("boxAttach", &FaustProcessor::getBoxAttach, arg("box1")=py::none(), arg("box2")=py::none(), "Blah", returnPolicy)

        .def("boxSampleRate", &FaustProcessor::getBoxSampleRate, "Blah", returnPolicy)
        .def("boxBufferSize", &FaustProcessor::getBoxBufferSize, "Blah", returnPolicy)

        .def("compile_box", &FaustProcessor::compileBox, arg("name"), arg("box"), arg("argv")=py::none(), "Blah")
    
        .def("dsp_to_box", &FaustProcessor::dspToBox, arg("dsp_code"), "Convert Faust DSP code to a Box.")
        
        .doc() = "A Faust Processor can compile and execute FAUST code. See https://faust.grame.fr for more information.";

}


#endif

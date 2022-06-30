#pragma once

#ifdef BUILD_DAWDREAMER_RUBBERBAND

#include "ProcessorBase.h"
#include "custom_pybind_wrappers.h"

#include "rubberband/RubberBandStretcher.h"
#include "AbletonClipInfo.h"

class PlaybackWarpProcessor : public ProcessorBase
{
public:
    PlaybackWarpProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData, double sr, double data_sr);

    PlaybackWarpProcessor(std::string newUniqueName, py::array_t<float, py::array::c_style | py::array::forcecast> input, double sr, double data_sr);

    void prepareToPlay(double, int) override;

    void automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) override;

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override;

    void reset() override;

    const juce::String getName() const override { return "PlaybackWarpProcessor"; }

    void setData(py::array_t<float, py::array::c_style | py::array::forcecast> input, double data_sr);

    void setTimeRatio(double ratio) { m_time_ratio_if_warp_off = ratio; }
    double getTimeRatio() { return m_time_ratio_if_warp_off; }

    void setTranspose(float newVal) { setAutomationVal("transpose", newVal); }
    float getTranspose() { return getAutomationAtZero("transpose"); }

    bool getWarpOn() { return m_clipInfo.warp_on; }
    void setWarpOn(bool warpOn) { m_clipInfo.warp_on = warpOn; }

    bool getLoopOn() { return m_clipInfo.loop_on; }
    void setLoopOn(bool loopOn) { m_clipInfo.loop_on = loopOn; }
    double getLoopStart() { return m_clipInfo.loop_start; }
    void setLoopStart(double loopStart) { m_clipInfo.loop_start = loopStart; }
    double getLoopEnd() { return m_clipInfo.loop_end; }
    void setLoopEnd(double loopEnd) { m_clipInfo.loop_end = loopEnd; }
    double getStartMarker() { return m_clipInfo.start_marker; }
    void setStartMarker(double startMarker) { m_clipInfo.start_marker = startMarker; }
    double getEndMarker() { return m_clipInfo.end_marker; }
    void setEndMarker(double endMarker) { m_clipInfo.end_marker = endMarker;}
    void setRubberBandOptions(int options);
    void defaultRubberBandOptions();

    py::array_t<float> getWarpMarkers();

    void resetWarpMarkers(double bpm);

    void setWarpMarkers(py::array_t<float, py::array::c_style | py::array::forcecast> input);

    bool setClipPositions(std::vector<std::tuple<double, double, double>> positions);

    bool loadAbletonClipInfo(const char* filepath);

private:

    void init();

    void setClipPositionsDefault();

    class Clip {
        public:
            Clip(double startPos, double endPos, double startMarkerOffset) : start_pos{ startPos }, end_pos{ endPos }, start_marker_offset{ startMarkerOffset } {};
            Clip() : start_pos { 0. }, end_pos{ std::numeric_limits<double>::max() }, start_marker_offset{ 0. } {};
            double start_pos = 0.;
            double end_pos = std::numeric_limits<double>::max();
            double start_marker_offset = 0.;
        };

    juce::AudioSampleBuffer myPlaybackData;
    double myPlaybackDataSR = 0;

    std::unique_ptr<RubberBand::RubberBandStretcher> m_rbstretcher;

    int m_numChannels = 2;

    juce::AudioSampleBuffer m_nonInterleavedBuffer;
    int sampleReadIndex = 0;

    AbletonClipInfo m_clipInfo;
    double m_sample_rate;

    double m_time_ratio_if_warp_off = 1.;
    std::atomic<float>* myTranspose;

    std::vector<Clip> m_clips;
    int m_clipIndex = 0;
    Clip m_currentClip;

    void setupRubberband();

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    int m_rubberbandConfig = 0;
};

#endif

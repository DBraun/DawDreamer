#pragma once

#ifdef BUILD_DAWDREAMER_RUBBERBAND

#include "AbletonClipInfo.h"
#include "custom_nanobind_wrappers.h"
#include "PickleVersion.h"
#include "ProcessorBase.h"
#include <rubberband/rubberband/RubberBandStretcher.h>

class PlaybackWarpProcessor : public ProcessorBase
{
  public:
    PlaybackWarpProcessor(std::string newUniqueName, std::vector<std::vector<float>> inputData,
                          double sr, double data_sr);

    PlaybackWarpProcessor(std::string newUniqueName, nb::ndarray<float> input, double sr,
                          double data_sr);

    void prepareToPlay(double, int) override;

    void automateParameters(AudioPlayHead::PositionInfo& posInfo, int numSamples) override;

    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer) override;

    void reset() override;

    const juce::String getName() const override { return "PlaybackWarpProcessor"; }

    void setData(nb::ndarray<float> input, double data_sr);

    void setTimeRatio(double ratio) { m_time_ratio_if_warp_off = ratio; }
    double getTimeRatio() const { return m_time_ratio_if_warp_off; }

    void setTranspose(float newVal) { setAutomationVal("transpose", newVal); }
    float getTranspose() const { return getAutomationAtZero("transpose"); }

    bool getWarpOn() const { return m_clipInfo.warp_on; }
    void setWarpOn(bool warpOn) { m_clipInfo.warp_on = warpOn; }

    bool getLoopOn() const { return m_clipInfo.loop_on; }
    void setLoopOn(bool loopOn) { m_clipInfo.loop_on = loopOn; }
    double getLoopStart() const { return m_clipInfo.loop_start; }
    void setLoopStart(double loopStart) { m_clipInfo.loop_start = loopStart; }
    double getLoopEnd() const { return m_clipInfo.loop_end; }
    void setLoopEnd(double loopEnd) { m_clipInfo.loop_end = loopEnd; }
    double getStartMarker() const { return m_clipInfo.start_marker; }
    void setStartMarker(double startMarker) { m_clipInfo.start_marker = startMarker; }
    double getEndMarker() const { return m_clipInfo.end_marker; }
    void setEndMarker(double endMarker) { m_clipInfo.end_marker = endMarker; }
    void setRubberBandOptions(int options);
    void defaultRubberBandOptions();

    nb::ndarray<nb::numpy, float> getWarpMarkers();

    void resetWarpMarkers(double bpm);

    void setWarpMarkers(nb::ndarray<float> input);

    bool setClipPositions(std::vector<std::tuple<double, double, double>> positions);

    bool loadAbletonClipInfo(const char* filepath);

    void createParameterLayout();

    nb::dict getPickleState()
    {
        nb::dict state;
        state["pickle_version"] = DawDreamerPickle::getVersion();
        state["unique_name"] = getUniqueName();
        state["sample_rate"] = m_sample_rate;
        state["data_sample_rate"] = myPlaybackDataSR;
        state["audio_data"] = bufferToPyArray(myPlaybackData);

        // Serialize clip info
        state["warp_on"] = m_clipInfo.warp_on;
        state["loop_on"] = m_clipInfo.loop_on;
        state["loop_start"] = m_clipInfo.loop_start;
        state["loop_end"] = m_clipInfo.loop_end;
        state["start_marker"] = m_clipInfo.start_marker;
        state["hidden_loop_start"] = m_clipInfo.hidden_loop_start;
        state["hidden_loop_end"] = m_clipInfo.hidden_loop_end;
        state["end_marker"] = m_clipInfo.end_marker;

        // Serialize warp markers
        state["warp_markers"] = getWarpMarkers();

        // Serialize other settings
        state["time_ratio_if_warp_off"] = m_time_ratio_if_warp_off;
        state["rubberband_config"] = m_rubberbandConfig;
        state["transpose"] = getAutomationAtZero("transpose");

        return state;
    }

    void setPickleState(nb::dict state)
    {
        // Check pickle version
        if (state.contains("pickle_version"))
        {
            int version = nb::cast<int>(state["pickle_version"]);
            if (!DawDreamerPickle::isCompatibleVersion(version))
            {
                throw std::runtime_error(DawDreamerPickle::getVersionErrorMessage(version));
            }
        }

        // Extract state
        std::string name = nb::cast<std::string>(state["unique_name"]);
        double sample_rate = nb::cast<double>(state["sample_rate"]);
        double data_sample_rate = nb::cast<double>(state["data_sample_rate"]);
        nb::ndarray<float> audio_data = nb::cast<nb::ndarray<float>>(state["audio_data"]);

        // Use placement new to construct the object in-place
        new (this) PlaybackWarpProcessor(name, audio_data, sample_rate, data_sample_rate);

        // Restore clip info
        if (state.contains("warp_on"))
            m_clipInfo.warp_on = nb::cast<bool>(state["warp_on"]);
        if (state.contains("loop_on"))
            m_clipInfo.loop_on = nb::cast<bool>(state["loop_on"]);
        if (state.contains("loop_start"))
            m_clipInfo.loop_start = nb::cast<double>(state["loop_start"]);
        if (state.contains("loop_end"))
            m_clipInfo.loop_end = nb::cast<double>(state["loop_end"]);
        if (state.contains("start_marker"))
            m_clipInfo.start_marker = nb::cast<double>(state["start_marker"]);
        if (state.contains("hidden_loop_start"))
            m_clipInfo.hidden_loop_start = nb::cast<double>(state["hidden_loop_start"]);
        if (state.contains("hidden_loop_end"))
            m_clipInfo.hidden_loop_end = nb::cast<double>(state["hidden_loop_end"]);
        if (state.contains("end_marker"))
            m_clipInfo.end_marker = nb::cast<double>(state["end_marker"]);

        // Restore warp markers
        if (state.contains("warp_markers"))
        {
            nb::ndarray<float> warp_markers = nb::cast<nb::ndarray<float>>(state["warp_markers"]);
            setWarpMarkers(warp_markers);
        }

        // Restore other settings
        if (state.contains("time_ratio_if_warp_off"))
            m_time_ratio_if_warp_off = nb::cast<double>(state["time_ratio_if_warp_off"]);
        if (state.contains("rubberband_config"))
        {
            m_rubberbandConfig = nb::cast<int>(state["rubberband_config"]);
            setupRubberband();
        }
        if (state.contains("transpose"))
            setAutomationVal("transpose", nb::cast<float>(state["transpose"]));
    }

  private:
    void init();

    void setClipPositionsDefault();

    class Clip
    {
      public:
        Clip(double startPos, double endPos, double startMarkerOffset)
            : start_pos{startPos}, end_pos{endPos}, start_marker_offset{startMarkerOffset} {};
        Clip()
            : start_pos{0.}, end_pos{std::numeric_limits<double>::max()},
              start_marker_offset{0.} {};
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

    std::vector<Clip> m_clips;
    int m_clipIndex = 0;
    Clip m_currentClip;

    void setupRubberband();

    int m_rubberbandConfig = 0;
};

#endif

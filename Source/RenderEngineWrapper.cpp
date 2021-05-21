#include "RenderEngineWrapper.h"

RenderEngineWrapper::RenderEngineWrapper(double sr, int bs) :
    RenderEngine(sr, bs)
{
}

py::array_t<float>
RenderEngineWrapper::wrapperGetAudioFrames()
{
    std::vector<std::vector<float>> channelBuffers = RenderEngine::getAudioFrames();

    size_t num_channels = channelBuffers.size();
    size_t num_samples = channelBuffers[0].size();

    py::array_t<float, py::array::c_style> arr({ (int)num_channels, (int)num_samples });

    auto ra = arr.mutable_unchecked();

    for (size_t i = 0; i < num_channels; i++)
    {
        for (size_t j = 0; j < num_samples; j++)
        {
            ra(i, j) = channelBuffers[i][j];
        }
    }

    return arr;
}

/// @brief
std::shared_ptr<OscillatorProcessor>
RenderEngineWrapper::makeOscillatorProcessor(const std::string& name, float freq)
{
    return std::shared_ptr<OscillatorProcessor>{new OscillatorProcessor{ name, freq }};
}

/// @brief
std::shared_ptr<PluginProcessorWrapper>
RenderEngineWrapper::makePluginProcessor(const std::string& name, const std::string& path)
{
    return std::shared_ptr<PluginProcessorWrapper>{new PluginProcessorWrapper{ name, mySampleRate, myBufferSize, path }};
}

/// @brief
std::shared_ptr<PlaybackProcessor>
RenderEngineWrapper::makePlaybackProcessor(const std::string& name, py::array data)
{
    return std::shared_ptr<PlaybackProcessor>{new PlaybackProcessor{ name, data }};
}

/// @brief
std::shared_ptr<PlaybackWarpProcessor>
RenderEngineWrapper::makePlaybackWarpProcessor(const std::string& name, py::array data)
{
    return std::shared_ptr<PlaybackWarpProcessor>{new PlaybackWarpProcessor{ name, data }};
}

std::shared_ptr<FilterProcessor>
RenderEngineWrapper::makeFilterProcessor(const std::string& name, const std::string& mode, float freq, float q, float gain) {

    float validFreq = std::fmax(.0001f, freq);
    float validQ = std::fmax(.0001f, q);
    float validGain = std::fmax(.0001f, gain);

    return std::shared_ptr<FilterProcessor>{new FilterProcessor{ name, mode, validFreq, validQ, validGain }};
}

std::shared_ptr<CompressorProcessor>
RenderEngineWrapper::makeCompressorProcessor(const std::string& name, float threshold = 0.f, float ratio = 2.f, float attack = 2.f, float release = 50.f) {

    // ratio must be >= 1.0
    // attack and release are in milliseconds
    float validRatio = std::fmax(1.0f, ratio);
    float validAttack = std::fmax(0.f, attack);
    float validRelease = std::fmax(0.f, release);
    return std::shared_ptr<CompressorProcessor>{new CompressorProcessor{ name, threshold, validRatio, validAttack, validRelease }};
}


std::shared_ptr<AddProcessor>
RenderEngineWrapper::makeAddProcessor(const std::string& name, std::vector<float> gainLevels) {
    return std::shared_ptr<AddProcessor>{new AddProcessor{ name, gainLevels }};
}


std::shared_ptr<ReverbProcessor>
RenderEngineWrapper::makeReverbProcessor(const std::string& name, float roomSize = 0.5f, float damping = 0.5f, float wetLevel = 0.33f,
    float dryLevel = 0.4f, float width = 1.0f) {
    return std::shared_ptr<ReverbProcessor>{new ReverbProcessor{ name, roomSize, damping, wetLevel, dryLevel, width }};
}

std::shared_ptr<PannerProcessor>
RenderEngineWrapper::makePannerProcessor(const std::string& name, std::string& rule, float pan) {

    float safeVal = std::fmax(-1.f, pan);
    safeVal = std::fmin(1.f, pan);

    return std::shared_ptr<PannerProcessor>{new PannerProcessor{ name, rule, safeVal }};
}

std::shared_ptr<DelayProcessor>
RenderEngineWrapper::makeDelayProcessor(const std::string& name, std::string& rule, float delay, float wet) {
    float safeDelay = std::fmax(0.f, delay);

    float safeWet = std::fmin(1.f, std::fmax(0.f, wet));

    return std::shared_ptr<DelayProcessor>{new DelayProcessor{ name, rule, safeDelay, safeWet }};
}

/// @brief
std::shared_ptr<SamplerProcessor>
RenderEngineWrapper::makeSamplerProcessor(const std::string& name, py::array data)
{
    return std::shared_ptr<SamplerProcessor>{new SamplerProcessor{ name, data, mySampleRate, myBufferSize }};
}

#ifdef BUILD_DAWDREAMER_FAUST
std::shared_ptr<FaustProcessor>
RenderEngineWrapper::makeFaustProcessor(const std::string& name, const std::string& path)
{
    return std::shared_ptr<FaustProcessor>{new FaustProcessor{ name, mySampleRate, myBufferSize, path }};
}
#endif

bool
RenderEngineWrapper::loadGraphWrapper(py::object dagObj, int numInputAudioChans = 2, int numOutputAudioChans = 2) {

    if (!py::isinstance<py::list>(dagObj)) {
        return false;
    }

    DAG* buildingDag = new DAG();

    for (py::handle theTuple : dagObj) {  // iterators!

        if (!py::isinstance<py::tuple>(theTuple) && !py::isinstance<py::list>(theTuple)) {
            std::cout << "Error: load_graph. Received graph that is not a list." << std::endl;
            return false;
        }
        py::list castedTuple = theTuple.cast<py::list>();

        if (castedTuple.size() != 2) {
            std::cout << "Error: load_graph. Each tuple in the graph must be size 2." << std::endl;
            return false;
        }

        //if (!py::isinstance<ProcessorBase*>(castedTuple[0])) {
        // std::cout << "Error: load_graph. First argument in tuple wasn't a Processor object." << std::endl;
        //    return false;
        //}
        if (!py::isinstance<py::list>(castedTuple[1])) {
            std::cout << "Error: load_graph. Something was wrong with the list of inputs." << std::endl;
            return false;
        }

        py::list listOfStrings = castedTuple[1].cast<py::list>();

        std::vector<std::string> inputs;

        for (py::handle potentialString : listOfStrings) {
            if (!py::isinstance<py::str>(potentialString)) {
                std::cout << "Error: load_graph. Something was wrong with the list of inputs." << std::endl;
                return false;
            }

            inputs.push_back(potentialString.cast<std::string>());
        }

        DAGNode dagNode;
        try {
            dagNode.processorBase = castedTuple[0].cast<ProcessorBase*>();
        }
        catch (std::exception&) {
            std::cout << "Error: load_graph. First argument in tuple wasn't a Processor object." << std::endl;
            return false;
        }

        dagNode.inputs = inputs;

        buildingDag->nodes.push_back(dagNode);
    }

    return RenderEngine::loadGraph(*buildingDag, numInputAudioChans, numOutputAudioChans);
}

#include "RenderEngineWrapper.h"

RenderEngineWrapper::RenderEngineWrapper(double sr, int bs) :
    RenderEngine(sr, bs)
{
}

py::list
RenderEngineWrapper::wrapperGetAudioFrames()
{
    std::vector<std::vector<float>> channelBuffers = RenderEngine::getAudioFrames();
    py::list theList;
    for (auto oneBuffer = channelBuffers.begin(); oneBuffer != channelBuffers.end(); ++oneBuffer)
    {
        theList.append(customBoost::vectorToList<float>(*oneBuffer));
    }
    return theList;
}

//boost::python::numpy::ndarray
//RenderEngineWrapper::wrapperGetAudioFrames()
//{
//    namespace p = boost::python;
//    namespace np = boost::python::numpy;
//
//    std::vector<std::vector<float>> channelBuffers = RenderEngine::getAudioFrames();
//    boost::python::tuple shape = p::make_tuple(channelBuffers.size(), channelBuffers[0].size());
//    boost::python::tuple strides = p::make_tuple(channelBuffers.size(), channelBuffers[0].size());
//    np::ndarray theArray = np::from_data(&channelBuffers, np::dtype::get_builtin<float>(), shape, strides, p::object());
//
//    return theArray;
//}

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

std::shared_ptr<FilterProcessor>
RenderEngineWrapper::makeFilterProcessor(const std::string& name, const std::string& mode, float freq, float q, float gain) {

    float validFreq = std::max(.0001f, freq);
    float validQ = std::max(.0001f, q);
    float validGain = std::max(.0001f, gain);

    return std::shared_ptr<FilterProcessor>{new FilterProcessor{ name, mode, validFreq, validQ, validGain }};
}

std::shared_ptr<CompressorProcessor>
RenderEngineWrapper::makeCompressorProcessor(const std::string& name, float threshold=0.f, float ratio=2.f, float attack=2.f, float release=50.f) {

    // ratio must be >= 1.0
    // attack and release are in milliseconds
    float validRatio = std::max(1.0f, ratio);
    float validAttack = std::max(0.f, attack);
    float validRelease = std::max(0.f, release);
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

bool
RenderEngineWrapper::loadGraphWrapper(py::object dagObj, int numInputAudioChans=2, int numOutputAudioChans=2) {

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
        catch (std::exception& e) {
            std::cout << "Error: load_graph. First argument in tuple wasn't a Processor object." << std::endl;
            return false;
        }
        
        dagNode.inputs = inputs;

        buildingDag->nodes.push_back(dagNode);
    }

    return RenderEngine::loadGraph(*buildingDag, numInputAudioChans, numOutputAudioChans);
}

#include "RenderEngineWrapper.h"


RenderEngineWrapper::RenderEngineWrapper(double sr, int bs) :
    RenderEngine(sr, bs)
{
}


void
RenderEngineWrapper::prepareProcessor(ProcessorBase* processor, const std::string& name)
{
    if (m_UniqueNameToNodeID.find(name) != m_UniqueNameToNodeID.end()) {
        myMainProcessorGraph->removeNode(m_UniqueNameToNodeID[name]);
        m_UniqueNameToNodeID.erase(name);
    }

    auto node = myMainProcessorGraph->addNode((std::unique_ptr<ProcessorBase>)(processor));
    m_UniqueNameToNodeID[name] = node->nodeID;
}


OscillatorProcessor*
RenderEngineWrapper::makeOscillatorProcessor(const std::string& name, float freq)
{
    auto processor = new OscillatorProcessor{ name, freq };
    this->prepareProcessor(processor, name);
    return processor;
}


PluginProcessorWrapper*
RenderEngineWrapper::makePluginProcessor(const std::string& name, const std::string& path)
{
    auto processor = new PluginProcessorWrapper{ name, mySampleRate, myBufferSize, path };
    this->prepareProcessor(processor, name);
    return processor;
}


PlaybackProcessor*
RenderEngineWrapper::makePlaybackProcessor(const std::string& name, py::array data)
{
    auto processor = new PlaybackProcessor{ name, data };
    this->prepareProcessor(processor, name);
    return processor;
}


#ifdef BUILD_DAWDREAMER_RUBBERBAND
PlaybackWarpProcessor*
RenderEngineWrapper::makePlaybackWarpProcessor(const std::string& name, py::array data)
{
    auto processor =new PlaybackWarpProcessor{ name, data, mySampleRate };
    this->prepareProcessor(processor, name);
    return processor;

}
#endif


FilterProcessor*
RenderEngineWrapper::makeFilterProcessor(const std::string& name, const std::string& mode, float freq, float q, float gain) {

    float validFreq = std::fmax(.0001f, freq);
    float validQ = std::fmax(.0001f, q);
    float validGain = std::fmax(.0001f, gain);

    auto processor = new FilterProcessor{ name, mode, validFreq, validQ, validGain };
    this->prepareProcessor(processor, name);
    return processor;
}


CompressorProcessor*
RenderEngineWrapper::makeCompressorProcessor(const std::string& name, float threshold = 0.f, float ratio = 2.f, float attack = 2.f, float release = 50.f) {

    // ratio must be >= 1.0
    // attack and release are in milliseconds
    float validRatio = std::fmax(1.0f, ratio);
    float validAttack = std::fmax(0.f, attack);
    float validRelease = std::fmax(0.f, release);

    auto processor = new CompressorProcessor{ name, threshold, validRatio, validAttack, validRelease };
    this->prepareProcessor(processor, name);
    return processor;
}


AddProcessor*
RenderEngineWrapper::makeAddProcessor(const std::string& name, std::vector<float> gainLevels) {

    auto processor = new AddProcessor{ name, gainLevels };
    this->prepareProcessor(processor, name);
    return processor;
}


ReverbProcessor*
RenderEngineWrapper::makeReverbProcessor(const std::string& name, float roomSize = 0.5f, float damping = 0.5f, float wetLevel = 0.33f,
    float dryLevel = 0.4f, float width = 1.0f) {

    auto processor = new ReverbProcessor{ name, roomSize, damping, wetLevel, dryLevel, width };
    this->prepareProcessor(processor, name);
    return processor;
}


PannerProcessor*
RenderEngineWrapper::makePannerProcessor(const std::string& name, std::string& rule, float pan) {

    float safeVal = std::fmax(-1.f, pan);
    safeVal = std::fmin(1.f, pan);

    auto processor = new PannerProcessor{ name, rule, safeVal };
    this->prepareProcessor(processor, name);
    return processor;
}


DelayProcessor*
RenderEngineWrapper::makeDelayProcessor(const std::string& name, std::string& rule, float delay, float wet) {
    float safeDelay = std::fmax(0.f, delay);

    float safeWet = std::fmin(1.f, std::fmax(0.f, wet));
    auto processor = new DelayProcessor{ name, rule, safeDelay, safeWet };

    this->prepareProcessor(processor, name);

    return processor;
}


SamplerProcessor*
RenderEngineWrapper::makeSamplerProcessor(const std::string& name, py::array data)
{
    auto processor = new SamplerProcessor{ name, data, mySampleRate, myBufferSize };
    this->prepareProcessor(processor, name);
    return processor;
}


#ifdef BUILD_DAWDREAMER_FAUST
FaustProcessor*
RenderEngineWrapper::makeFaustProcessor(const std::string& name)
{
    auto processor = new FaustProcessor{ name, mySampleRate, myBufferSize };
    this->prepareProcessor(processor, name);
    return processor;
}
#endif

bool
RenderEngineWrapper::loadGraphWrapper(py::object dagObj) {

    if (!py::isinstance<py::list>(dagObj)) {
        throw std::runtime_error("Error: load_graph. No processors were passed.");
    }

    DAG* buildingDag = new DAG();

    for (py::handle theTuple : dagObj) {  // iterators!

        if (!py::isinstance<py::tuple>(theTuple) && !py::isinstance<py::list>(theTuple)) {
            throw std::runtime_error("Error: load_graph. Received graph that is not a list.");
        }
        py::list castedTuple = theTuple.cast<py::list>();

        if (castedTuple.size() != 2) {
            throw std::runtime_error("Error: load_graph. Each tuple in the graph must be size 2.");
        }

        // todo: enable this:
        //if (!py::isinstance<ProcessorBase*>(castedTuple[0])) {
        // std::cout << "Error: load_graph. First argument in tuple wasn't a Processor object." << std::endl;
        //    return false;
        //}
        if (!py::isinstance<py::list>(castedTuple[1])) {
            throw std::runtime_error("Error: load_graph. Something was wrong with the list of inputs.");
        }

        py::list listOfStrings = castedTuple[1].cast<py::list>();

        std::vector<std::string> inputs;

        for (py::handle potentialString : listOfStrings) {
            if (!py::isinstance<py::str>(potentialString)) {
                throw std::runtime_error("Error: load_graph. Something was wrong with the list of inputs.");
            }

            inputs.push_back(potentialString.cast<std::string>());
        }

        DAGNode dagNode;
        try {
            dagNode.processorBase = castedTuple[0].cast<ProcessorBase*>();
        }
        catch (std::exception&) {
            throw std::runtime_error("Error: Load_graph. First argument in tuple wasn't a Processor object.");
        }

        dagNode.inputs = inputs;

        buildingDag->nodes.push_back(dagNode);
    }

    auto result = RenderEngine::loadGraph(*buildingDag);

    delete buildingDag;

    return result;
}

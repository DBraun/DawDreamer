#include "FaustProcessor.h"

#ifdef BUILD_DAWDREAMER_FAUST

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)              do { if(x){ delete x; x = NULL; } } while(0)
#define SAFE_DELETE_ARRAY(x)        do { if(x){ delete [] x; x = NULL; } } while(0)
#define SAFE_RELEASE(x)             do { if(x){ x->release(); x = NULL; } } while(0)
#define SAFE_ADD_REF(x)             do { if(x){ x->add_ref(); } } while(0)
#define SAFE_REF_ASSIGN(lhs,rhs)    do { SAFE_RELEASE(lhs); (lhs) = (rhs); SAFE_ADD_REF(lhs); } while(0)
#endif

FaustProcessor::FaustProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock, std::string path) : ProcessorBase{ newUniqueName }
{
	mySampleRate = sampleRate;

	m_factory = NULL;
	//m_poly_factory = NULL;
	m_dsp = NULL;
	//m_dsp_poly = NULL;
	m_ui = NULL;
	//m_midi_ui = NULL;
	// zero
	//m_input = NULL;
	//m_output = NULL;
	// default
	m_numInputChannels = 0;
	m_numOutputChannels = 0;
	// auto import
	m_autoImport = "// FaustProcessor (DawDreamer) auto import:\n \
        import(\"stdfaust.lib\");\n";

	this->compileFromFile(path);
}

FaustProcessor::~FaustProcessor() {
	clear();
}

void
FaustProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
}

void
FaustProcessor::processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&)
{
	automateParameters();

	AudioPlayHead::CurrentPositionInfo posInfo;
	getPlayHead()->getCurrentPosition(posInfo);

	if (buffer.getNumChannels() != m_numInputChannels) {
		if (posInfo.timeInSamples == 0) {
			std::cerr << "FaustProcessor compiled DSP code for " << m_numInputChannels << " input channels but received " << buffer.getNumChannels() << "." << std::endl;;
		}
		return;
	}
	if (m_numOutputChannels != 2) {
		if (posInfo.timeInSamples == 0) {
			std::cerr << "FaustProcessor must have DSP code with 2 output channels but was compiled for " << m_numOutputChannels << "." << std::endl;
		}
		return;
	}

	dsp* theDsp = m_polyphony_enable ? m_dsp_poly : m_dsp;

	if (theDsp != NULL) {
		theDsp->compute(buffer.getNumSamples(), (float**)buffer.getArrayOfReadPointers(), buffer.getArrayOfWritePointers());
	}
}

void
FaustProcessor::automateParameters() {

    AudioPlayHead::CurrentPositionInfo posInfo;
    getPlayHead()->getCurrentPosition(posInfo);

}

void
FaustProcessor::reset()
{
	if (m_dsp) {
		m_dsp->instanceClear();
	}
}

void
FaustProcessor::clear()
{
	m_numInputChannels = 0;
	m_numOutputChannels = 0;

	// todo: do something with m_midi_handler
	if (m_dsp_poly) {
		//m_midi_handler.removeMidiIn(m_dsp_poly);
		//m_midi_handler.stopMidi();
	}
	//if (m_midi_ui) {
	//	m_midi_ui->removeMidiIn(m_dsp_poly);
	//	m_midi_ui->stop();
	//}

	SAFE_DELETE(m_dsp);
	SAFE_DELETE(m_ui);
	//SAFE_DELETE(m_dsp_poly);
	//SAFE_DELETE(m_midi_ui);

	deleteAllDSPFactories();
	m_factory = NULL;
	//m_poly_factory = NULL;
}

bool
FaustProcessor::eval(const std::string& code)
{
	// clean up
	clear();

	// arguments
	const int argc = 0;
	const char** argv = NULL;
	// optimization level
	const int optimize = -1;

	// save
	m_code = code;

	// auto import
	std::string theCode = m_autoImport + "\n" + code;

	std::string m_errorString;

	// create new factory
	if (m_polyphony_enable) {
		m_poly_factory = createPolyDSPFactoryFromString("DawDreamer", theCode,
			argc, argv, "", m_errorString, optimize);
	}
	else {
		m_factory = createDSPFactoryFromString("DawDreamer", theCode,
			argc, argv, "", m_errorString, optimize);
	}

	// check for error
	if (m_errorString != "") {
		// output error
		std::cerr << "[Faust]: " << m_errorString << std::endl;
		// clear
		clear();
		// done
		return false;
	}

	//// print where faustlib is looking for stdfaust.lib and the other lib files.
	//auto pathnames = m_factory->getIncludePathnames();
	//std::cout << "pathnames:\n" << std::endl;
	//for (auto name : pathnames) {
	//	std::cout << name << "\n" << std::endl;
	//}
	//std::cout << "library list:\n" << std::endl;
	//auto librarylist = m_factory->getLibraryList();
	//for (auto name : librarylist) {
	//	std::cout << name << "\n" << std::endl;
	//}

	//if (m_midi_enable) {
	//	m_midi_handler = rt_midi("my_midi");
	//}

	if (m_polyphony_enable) {
		// (false, true) works
		m_dsp_poly = m_poly_factory->createPolyDSPInstance(m_nvoices, false, false);
		if (!m_dsp_poly) {
			std::cerr << "Cannot create instance " << std::endl;
			clear();
			return false;
		}
	}
	else {
		// create DSP instance
		m_dsp = m_factory->createDSPInstance();
		if (!m_dsp) {
			std::cerr << "Cannot create instance " << std::endl;
			clear();
			return false;
		}
	}

	dsp* theDsp = m_polyphony_enable ? m_dsp_poly : m_dsp;

	// get channels
	int inputs = theDsp->getNumInputs();
	int outputs = theDsp->getNumOutputs();

	if (outputs != 2) {
		std::cerr << "FaustProcessor must have DSP code with 2 output channels but was compiled for " << m_numOutputChannels << "." << std::endl;
		clear();
		return false;
	}
	if ((inputs % 2) != 0) {
		std::cerr << "FaustProcessor must have DSP code with an even number of input channels but was compiled for " << m_numOutputChannels << "." << std::endl;
		clear();
		return false;
	}

	m_numInputChannels = inputs;
	m_numOutputChannels = outputs;

	// make new UI
	UI* theUI = nullptr;
	if (m_polyphony_enable && m_midi_enable)
	{
		m_midi_ui = new MidiUI(nullptr);
		theUI = m_midi_ui;
	}
	else {
		m_ui = new FaustCHOPUI();
		theUI = m_ui;
	}

	// build ui
	theDsp->buildUserInterface(theUI);

	// init
	theDsp->init((int)(mySampleRate + .5));

	if (m_polyphony_enable && m_midi_enable) {
		m_midi_ui->run();
	}

	createParameterLayout();

	return true;
}

bool
FaustProcessor::compileFromFile(const std::string& path)
{
	// open file
	std::ifstream fin(path.c_str());
	// check
	if (!fin.good())
	{
		// error
		std::cerr << "[Faust]: ERROR opening file: '" << path << "'" << std::endl;
		return false;
	}

	// clear code string
	m_code = "";
	// get it
	for (std::string line; std::getline(fin, line); ) {
		m_code += line + '\n';
	}
	// eval it
	return eval(m_code);
}

bool
FaustProcessor::compile()
{
	return eval(m_code);
}

float
FaustProcessor::setParamWithPath(const std::string& n, float p)
{
	// sanity check
	if (!m_ui) return 0;

	// set the value
	m_ui->setParamValue(n.c_str(), p);

	return p;
}

float
FaustProcessor::getParamWithPath(const std::string& n)
{
	if (!m_ui) return 0; // todo: better handling
	return m_ui->getParamValue(n.c_str());
}

float
FaustProcessor::setParamWithIndex(const int index, float p)
{
	// sanity check
	if (!m_ui) return 0;

	// set the value
	m_ui->setParamValue(index, p);

	return p;
}

float
FaustProcessor::getParamWithIndex(const int index)
{
	if (!m_ui) return 0; // todo: better handling
	return m_ui->getParamValue(index);
}

std::string
FaustProcessor::code()
{
	return m_code;
}

void
FaustProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout blankLayout;

	// clear existing parameters in the layout?
	ValueTree blankState;
	myParameters.replaceState(blankState);

	for (int i = 0; i < m_ui->getParamsCount(); ++i)
	{
		auto parameterName = m_ui->getParamAddress(i);
		auto parameterLabel = m_ui->getParamLabel(i);
		myParameters.createAndAddParameter(std::make_unique<AutomateParameterFloat>(parameterName, parameterLabel,
			NormalisableRange<float>(m_ui->getParamMin(i), m_ui->getParamMax(i)), m_ui->getParamInit(i)));
		// give it a valid single sample of automation.
		ProcessorBase::setAutomationVal(parameterName, m_ui->getParamValue(i));
	}
}

py::list
FaustProcessor::getPluginParametersDescription()
{
	py::list myList;

	if (m_dsp != nullptr) {

		for (int i = 0; i < m_ui->getParamsCount(); i++) {

			py::dict myDictionary;
			myDictionary["index"] = i;
			myDictionary["name"] = m_ui->getParamAddress(i);;
			myDictionary["min"] = m_ui->getParamMin(i);
			myDictionary["max"] = m_ui->getParamMax(i);
			myDictionary["label"] = m_ui->getParamLabel(i);
			myDictionary["step"] = m_ui->getParamStep(i);
			myDictionary["value"] = m_ui->getParamValue(i);

			myList.append(myDictionary);
		}
	}
	else
	{
		std::cout << "[FaustProcessor] Please compile DSP code first!" << std::endl;
	}

	return myList;
}

#endif
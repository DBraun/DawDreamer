#include "FaustProcessor.h"

#ifdef BUILD_DAWDREAMER_FAUST

#include <filesystem>
#include "faust/midi/RtMidi.cpp"

#ifdef WIN32
__declspec(selectany) std::list<GUI*> GUI::fGuiList;
__declspec(selectany) ztimedmap GUI::gTimedZoneMap;
#else
std::list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)              do { if(x){ delete x; x = NULL; } } while(0)
#define SAFE_DELETE_ARRAY(x)        do { if(x){ delete [] x; x = NULL; } } while(0)
#define SAFE_RELEASE(x)             do { if(x){ x->release(); x = NULL; } } while(0)
#define SAFE_ADD_REF(x)             do { if(x){ x->add_ref(); } } while(0)
#define SAFE_REF_ASSIGN(lhs,rhs)    do { SAFE_RELEASE(lhs); (lhs) = (rhs); SAFE_ADD_REF(lhs); } while(0)
#endif



FaustProcessor::FaustProcessor(std::string newUniqueName, double sampleRate, int samplesPerBlock) : ProcessorBase{ newUniqueName }
{
	mySampleRate = sampleRate;

	m_factory = NULL;
	m_poly_factory = NULL;
	m_dsp = NULL;
	m_dsp_poly = NULL;
	m_ui = NULL;
	m_soundUI = NULL;
	m_numInputChannels = 0;
	m_numOutputChannels = 0;
	// auto import
	m_autoImport = "// FaustProcessor (DawDreamer) auto import:\nimport(\"stdfaust.lib\");\n";
}

FaustProcessor::~FaustProcessor() {
	clear();
	deleteAllDSPFactories();
}

bool
FaustProcessor::canApplyBusesLayout(const juce::AudioProcessor::BusesLayout& layout) {
	return (layout.getMainInputChannels() == m_numInputChannels) && (layout.getMainOutputChannels() == m_numOutputChannels);
}

void
FaustProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
}

void
FaustProcessor::processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiBuffer)
{
	automateParameters();

	AudioPlayHead::CurrentPositionInfo posInfo;
	getPlayHead()->getCurrentPosition(posInfo);

	if (!m_isCompiled) {
		ProcessorBase::processBlock(buffer, midiBuffer);
		return;
	}

	if (m_nvoices < 1) {
		if (m_dsp != NULL) {
			m_dsp->compute(buffer.getNumSamples(), (float**)buffer.getArrayOfReadPointers(), buffer.getArrayOfWritePointers());
		}
	}
	else if (m_dsp_poly != NULL) {
		long long int start = posInfo.timeInSamples;

		// render one sample at a time

		for (size_t i = 0; i < buffer.getNumSamples(); i++)
		{
			myIsMessageBetween = myMidiMessagePosition >= start && myMidiMessagePosition < start + 1;
			do {
				if (myIsMessageBetween) {

					int midiChannel = 0;
					if (myMidiMessage.isNoteOn()) {
						m_dsp_poly->keyOn(midiChannel, myMidiMessage.getNoteNumber(), myMidiMessage.getVelocity());
					}
					else {
						m_dsp_poly->keyOff(midiChannel, myMidiMessage.getNoteNumber(), myMidiMessage.getVelocity());
					}
					
					myMidiEventsDoRemain = myMidiIterator->getNextEvent(myMidiMessage, myMidiMessagePosition);
					myIsMessageBetween = myMidiMessagePosition >= start && myMidiMessagePosition < start + 1;
				}
			} while (myIsMessageBetween && myMidiEventsDoRemain);

			for (size_t chan = 0; chan < m_numInputChannels; chan++)
			{
				oneSampleInBuffer.setSample(chan, 0, buffer.getSample(chan, i));
			}

			m_dsp_poly->compute(1, (float**) oneSampleInBuffer.getArrayOfReadPointers(), (float**) oneSampleOutBuffer.getArrayOfWritePointers());

			for (size_t chan = 0; chan < m_numOutputChannels; chan++)
			{
				buffer.setSample(chan, i, oneSampleOutBuffer.getSample(chan, 0));
			}

			start += 1;
		}
	}

	ProcessorBase::processBlock(buffer, midiBuffer);
}

#include <iostream>

bool hasEnding(std::string const& fullString, std::string const& ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

bool hasStart(std::string const& fullString, std::string const& start) {
	return fullString.rfind(start, 0) == 0;
}

void
FaustProcessor::automateParameters() {

    AudioPlayHead::CurrentPositionInfo posInfo;
    getPlayHead()->getCurrentPosition(posInfo);

	if (!m_ui) return;

	const Array<AudioProcessorParameter*>& processorParams = this->getParameters();

	for (int i = 0; i < this->getNumParameters(); i++) {

		auto theParameter = (AutomateParameterFloat*)processorParams[i];

		int faustIndex = m_map_juceIndex_to_faustIndex[i];

		if (theParameter) {
			m_ui->setParamValue(faustIndex, theParameter->sample(posInfo.timeInSamples));
		}
		else {
			auto theName = this->getParameterName(i);
			std::cerr << "Error FaustProcessor::automateParameters: " << theName << std::endl;
		}
	}

	// If polyphony is enabled and we're grouping voices,
	// several voices might share the same parameters in a group.
	// Therefore we have to call updateAllGuis to update all dependent parameters.
	if (m_nvoices > 0 && m_groupVoices) {
		// When you want to access shared memory:
        if (guiUpdateMutex.Lock()) {
            // Have Faust update all GUIs.
            GUI::updateAllGuis();
                
            guiUpdateMutex.Unlock();
        }
	}
}

void
FaustProcessor::reset()
{
	if (m_dsp) {
		m_dsp->instanceClear();
	}

	if (m_dsp_poly) {
		m_dsp_poly->instanceClear();
	}

	if (myMidiIterator) {
		delete myMidiIterator;
	}

	myMidiIterator = new MidiBuffer::Iterator(myMidiBuffer); // todo: deprecated.
	myMidiEventsDoRemain = myMidiIterator->getNextEvent(myMidiMessage, myMidiMessagePosition);

	if (!m_isCompiled) {
		this->compile();
	}
}

void
FaustProcessor::clear()
{
	m_isCompiled = false;
	m_numInputChannels = 0;
	m_numOutputChannels = 0;

	// todo: do something with m_midi_handler
	if (m_dsp_poly) {
		m_midi_handler.removeMidiIn(m_dsp_poly);
		m_midi_handler.stopMidi();
	}

	SAFE_DELETE(m_soundUI);
	SAFE_DELETE(m_dsp);
	SAFE_DELETE(m_ui);
	SAFE_DELETE(m_dsp_poly);

	m_factory = NULL;
	m_poly_factory = NULL;
}

void
FaustProcessor::setNumVoices(int numVoices) {
	m_isCompiled = false;
	m_nvoices = std::max(0, numVoices);
}

int
FaustProcessor::getNumVoices() {
	return m_nvoices;
}

void
FaustProcessor::setGroupVoices(bool groupVoices) 
{
	m_isCompiled = false;
	m_groupVoices = groupVoices;
};

int
FaustProcessor::getGroupVoices() {
	return m_groupVoices;
};

bool
FaustProcessor::setDSPString(const std::string& code)
{
	m_isCompiled = false;

	if (std::strcmp(code.c_str(), "") == 0) {
		return false;
	}

	// save
	m_code = code;

	return true;
}

#define FAUSTPROCESSOR_FAIL_COMPILE clear(); return false;

bool
FaustProcessor::compile()
{
    std::cerr << "getDSPMachineTarget(): (" << getDSPMachineTarget() << ")" << std::endl;
    
	m_isCompiled = false;

	// clean up
	clear();

	// optimization level
	const int optimize = -1;
	// arguments

	auto pathToFaustLibraries = getPathToFaustLibraries();

	int argc = 0;
	const char** argv = NULL;
	if (pathToFaustLibraries.compare(std::string("")) != 0) {
		argc = 2;
		argv = new const char* [argc];
		argv[0] = "-I";
		argv[1] = pathToFaustLibraries.c_str();
	}

	auto theCode = m_autoImport + "\n" + m_code;

	std::string m_errorString;

	// create new factory
	bool is_polyphonic = m_nvoices > 0;
	if (is_polyphonic) {
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
		std::cerr << "FaustProcessor::compile(): " << m_errorString << std::endl;
		std::cerr << "Check the faustlibraries path: " << pathToFaustLibraries.c_str() << std::endl;
		FAUSTPROCESSOR_FAIL_COMPILE
	}

    //// print where faustlib is looking for stdfaust.lib and the other lib files.
    //auto pathnames = m_factory->getIncludePathnames();
    //std::cout << "pathnames:\n" << std::endl;
    //for (auto name : pathnames) {
    //    std::cout << name << "\n" << std::endl;
    //}
    //std::cout << "library list:\n" << std::endl;
    //auto librarylist = m_factory->getLibraryList();
    //for (auto name : librarylist) {
    //    std::cout << name << "\n" << std::endl;
    //}

	if (argv) {
		for (int i = 0; i < argc; i++) {
			argv[i] = NULL;
		}
		argv = NULL;
	}

	if (is_polyphonic) {
		// (false, true) works
		m_dsp_poly = m_poly_factory->createPolyDSPInstance(m_nvoices, true, m_groupVoices);
		if (!m_dsp_poly) {
			std::cerr << "FaustProcessor::compile(): Cannot create instance." << std::endl;
			FAUSTPROCESSOR_FAIL_COMPILE
		}
	}
	else {
		// create DSP instance
		m_dsp = m_factory->createDSPInstance();
		if (!m_dsp) {
			std::cerr << "FaustProcessor::compile(): Cannot create instance." << std::endl;
			FAUSTPROCESSOR_FAIL_COMPILE
		}
	}

	dsp* theDsp = is_polyphonic ? m_dsp_poly : m_dsp;

	// get channels
	int inputs = theDsp->getNumInputs();
	int outputs = theDsp->getNumOutputs();

	m_numInputChannels = inputs;
	m_numOutputChannels = outputs;
        
    setMainBusInputsAndOutputs(inputs, outputs);
    
	// make new UI
	if (is_polyphonic)
	{
		m_midi_handler = rt_midi("my_midi");
		m_midi_handler.addMidiIn(m_dsp_poly);

		oneSampleInBuffer.setSize(m_numInputChannels, 1);
		oneSampleOutBuffer.setSize(m_numOutputChannels, 1);
	}

	m_ui = new APIUI();
	theDsp->buildUserInterface(m_ui);

	// soundfile UI.
	m_soundUI = new MySoundUI();
	for (const auto& [label, buffers] : m_SoundfileMap) {
		m_soundUI->addSoundfileFromBuffers(label.c_str(), buffers, (int)(mySampleRate + .5));
	}
	theDsp->buildUserInterface(m_soundUI);

	// init
	theDsp->init((int)(mySampleRate + .5));

	createParameterLayout();

    m_isCompiled = true;
	return true;
}

bool
FaustProcessor::setDSPFile(const std::string& path)
{
	m_isCompiled = false;
	if (std::strcmp(path.c_str(), "") == 0) {
        return false;
    }
    
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

	return true;
}

bool
FaustProcessor::setParamWithIndex(const int index, float p)
{
	if (!m_isCompiled) {
		this->compile();
	}
	if (!m_ui) return false;

	auto it = m_map_juceIndex_to_parAddress.find(index);
	if (it == m_map_juceIndex_to_parAddress.end())
	{
		return false;
	}

	auto& parAddress = it->second;

	return this->setAutomationVal(parAddress, p);
}

float
FaustProcessor::getParamWithIndex(const int index)
{
	if (!m_isCompiled) {
		this->compile();
	}
	if (!m_ui) return 0; // todo: better handling

	auto it = m_map_juceIndex_to_parAddress.find(index);
	if (it == m_map_juceIndex_to_parAddress.end())
	{
		return 0.; // todo: better handling
	}

	auto& parAddress = it->second;

	return this->getAutomationVal(parAddress, 0);
}

float
FaustProcessor::getParamWithPath(const std::string& n)
{

	if (!m_isCompiled) {
		this->compile();
	}
	if (!m_ui) return 0; // todo: better handling

	return this->getAutomationVal(n, 0);
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

	m_map_juceIndex_to_faustIndex.clear();
	m_map_juceIndex_to_parAddress.clear();

	int numParamsAdded = 0;

	for (int i = 0; i < m_ui->getParamsCount(); ++i)
	{
		auto parameterName = m_ui->getParamAddress(i);

		auto parnameString = std::string(parameterName);

		// Ignore the Panic button.
		if (hasEnding(parnameString, std::string("/Panic"))) {
			continue;
		}

		// ignore the names of parameters which are reserved for controlling polyphony:
		// https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters
		// Note that an advanced user might want to not do this in order to
		// have much more control over the frequencies of individual voices,
		// like how MPE works.
		if (m_nvoices > 0) {
			if (hasEnding(parnameString, std::string("/freq")) ||
				hasEnding(parnameString, std::string("/note")) ||
				hasEnding(parnameString, std::string("/gain")) ||
				hasEnding(parnameString, std::string("/gate")) ||
				hasEnding(parnameString, std::string("/vel")) ||
				hasEnding(parnameString, std::string("/velocity"))
				) {
				continue;
			}

			if (!m_groupVoices && hasStart(parnameString, std::string("/Sequencer/DSP1/Polyphonic/Voices/"))) {
				// If we aren't grouping voices, FAUST for some reason is still adding the "grouped" parameters
				// to the UI, so we have to ignore them. The per-voice "ungrouped" parameters won't be skipped.
				continue;
			}
		}

		m_map_juceIndex_to_faustIndex[numParamsAdded] = i;
		m_map_juceIndex_to_parAddress[numParamsAdded] = parnameString;

		auto parameterLabel = m_ui->getParamLabel(i);
		myParameters.createAndAddParameter(std::make_unique<AutomateParameterFloat>(parameterName, parameterName,
			NormalisableRange<float>(m_ui->getParamMin(i), m_ui->getParamMax(i)), m_ui->getParamInit(i), parameterLabel));
		// give it a valid single sample of automation.
		ProcessorBase::setAutomationVal(parameterName, m_ui->getParamValue(i));

		numParamsAdded += 1;
	}
}

py::list
FaustProcessor::getPluginParametersDescription()
{
	py::list myList;

	if (!m_isCompiled) {
		this->compile();
	}

	if (m_isCompiled) {

		//get the parameters as an AudioProcessorParameter array
		const Array<AudioProcessorParameter*>& processorParams = this->getParameters();
		for (int i = 0; i < this->AudioProcessor::getNumParameters(); i++) {

			int maximumStringLength = 64;

			std::string theName = (processorParams[i])->getName(maximumStringLength).toStdString();
			std::string label = processorParams[i]->getLabel().toStdString();

			auto it = m_map_juceIndex_to_faustIndex.find(i);
			if (it == m_map_juceIndex_to_faustIndex.end())
			{
				continue;
			}

			int faustIndex = it->second;

			auto paramItemType = m_ui->getParamItemType(faustIndex);

			bool isDiscrete = (paramItemType == APIUI::kButton) || (paramItemType == APIUI::kCheckButton) || (paramItemType == APIUI::kNumEntry);
			int numSteps = (m_ui->getParamMax(faustIndex) - m_ui->getParamMin(faustIndex)) / m_ui->getParamStep(faustIndex) + 1;

			// todo: It would be better for DawDreamer to store the discrete parameters correctly,
			// but we're still saving them all as AutomateParameterFloat.
			//bool isDiscrete = processorParams[i]->isDiscrete();
			//int numSteps = processorParams[i]->getNumSteps();

			py::dict myDictionary;
			myDictionary["index"] = i;
			myDictionary["name"] = theName;
			myDictionary["numSteps"] = numSteps;
			myDictionary["isDiscrete"] = isDiscrete;
			myDictionary["label"] = label;

			myDictionary["min"] = m_ui->getParamMin(faustIndex);
			myDictionary["max"] = m_ui->getParamMax(faustIndex);
			myDictionary["step"] = m_ui->getParamStep(faustIndex);
			myDictionary["value"] = this->getAutomationVal(theName, 0);

			myList.append(myDictionary);
		}
	}
	else
	{
		std::cerr << "ERROR: The Faust process isn't compiled." << std::endl;
	}

	return myList;
}

int
FaustProcessor::getNumMidiEvents()
{
	return myMidiBuffer.getNumEvents();
};

bool
FaustProcessor::loadMidi(const std::string& path)
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
FaustProcessor::clearMidi() {
	myMidiBuffer.clear();
}

bool
FaustProcessor::addMidiNote(uint8  midiNote,
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

#ifdef WIN32

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Find path to .dll */
// https://stackoverflow.com/a/57738892/12327461
HMODULE hMod;
std::wstring MyDLLPathFull;
std::wstring MyDLLDir;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: case DLL_THREAD_ATTACH: case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	hMod = hModule;
	const int BUFSIZE = 4096;
	wchar_t buffer[BUFSIZE];
	if (::GetModuleFileNameW(hMod, buffer, BUFSIZE - 1) <= 0)
	{
		return TRUE;
	}

	MyDLLPathFull = buffer;

	size_t found = MyDLLPathFull.find_last_of(L"/\\");
	MyDLLDir = MyDLLPathFull.substr(0, found);

	return TRUE;
}

#else

// this applies to both __APPLE__ and linux?

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

// https://stackoverflow.com/a/51993539/911207
const char* getMyDLLPath(void) {
	Dl_info dl_info;
	dladdr((void*)getMyDLLPath, &dl_info);
	return(dl_info.dli_fname);
}
#endif

std::string
FaustProcessor::getPathToFaustLibraries() {

	// Get the path to the directory containing basics.lib, stdfaust.lib etc.

	try {

#ifdef WIN32
		const std::wstring ws_shareFaustDir = MyDLLDir + L"\\faustlibraries";
		//std::cerr << "MyDLLDir: ";
		//std::wcerr << MyDLLDir << L'\n';
		// convert const wchar_t to char
		// https://stackoverflow.com/a/4387335
		const wchar_t* wc_shareFaustDir = ws_shareFaustDir.c_str();
		// Count required buffer size (plus one for null-terminator).
		size_t size = (wcslen(wc_shareFaustDir) + 1) * sizeof(wchar_t);
		char* char_shareFaustDir = new char[size];
		std::wcstombs(char_shareFaustDir, wc_shareFaustDir, size);

		std::string p(char_shareFaustDir);
		return p;
#else
		// this applies to __APPLE__ and LINUX
		const char* myDLLPath = getMyDLLPath();
        //std::cerr << "myDLLPath: " << myDLLPath << std::endl;
		std::filesystem::path p = std::filesystem::path(myDLLPath);
        p = p.parent_path() / "faustlibraries";
		return p.string();
#endif
	}
	catch (...) {
		std::cerr << "Error getting path to faustlibraries." << std::endl;
	}
	return "";
}

using  myaudiotype = py::array_t<float, py::array::c_style | py::array::forcecast>;

void
FaustProcessor::setSoundfiles(py::dict d) {

	m_isCompiled = false;

	m_SoundfileMap.clear();

	for (auto&& [potentialString, potentialListOfAudio] : d) {

		if (!py::isinstance<py::str>(potentialString)) {
			std::cerr << "Error with FaustProcessor::setSoundfiles. Something was wrong with the keys of the dictionary." << std::endl;
			return;
		}

		auto soundfileName = potentialString.cast<std::string>();

		if (!py::isinstance<py::list>(potentialListOfAudio)) {
			// todo: if it's audio, it's ok. Just use it.
			std::cerr << "Error with FaustProcessor::setSoundfiles. The values of the dictionary must be lists of audio data." << std::endl;
			return;
		}

		py::list listOfAudio = potentialListOfAudio.cast<py::list>();

		for (py::handle potentialAudio : listOfAudio) {

			//if (py::isinstance<myaudiotype>(potentialAudio)) {

				// todo: safer casting?
				auto audioData = potentialAudio.cast<myaudiotype>();

				float* input_ptr = (float*)audioData.data();

				AudioSampleBuffer buffer;

				buffer.setSize(audioData.shape(0), audioData.shape(1));

				for (int y = 0; y < audioData.shape(1); y++) {
					for (int x = 0; x < audioData.shape(0); x++) {
						buffer.setSample(x, y, input_ptr[x * audioData.shape(1) + y]);
					}
				}

				m_SoundfileMap[soundfileName].push_back(buffer);

			//}
			//else {
			//	std::cerr << "key's value's list didn't contain audio data." << std::endl;
			//}
		}
		
	}
}

#endif

//Evan Sinasac - 1081418
//INFO6046 Media Fundamentals (Project 4)
//main.cpp description:
//			        The main purpose of project 4 is to combine streaming sounds from the internet with previous techniques we've
//					used, such as including DSPs, changing the volume, pausing, etc.

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <FMOD/fmod.hpp>

#include <rssgl/GLText.h>

#include <fstream>
#include <vector>

//Globals
GLuint _windowWidth = 1024;
GLuint _windowHeight = 768;

#define BUFFER_SIZE 255
char _buffer[BUFFER_SIZE];

GLFWwindow* _window = NULL;
std::string _appName = "Project 4: Internet Sounds";

FMOD::System* _system = NULL;
FMOD_RESULT _result = FMOD_OK;

RSS::GLText* _text;
GLuint _textRowIndex = 2;

std::vector<FMOD::Sound*> _vecSounds;
std::vector<FMOD::Channel*> _vecChannels;

std::vector<std::string> _vecURLs;
bool _isStarving = false;
FMOD_OPENSTATE _openState = FMOD_OPENSTATE_READY;
std::string _stateString = "";
unsigned int _percentage = 0;
int _numberOfTags = 0;
int _numberOfTagsUpdated = 0;
bool _isPaused = false;
bool _isPlaying = false;
int _soundIndex = 0;

// Project 4 stuff
FMOD::ChannelGroup* _cg1 = nullptr;
FMOD::ChannelGroup* _cg2 = nullptr;
FMOD::ChannelGroup* _cg3 = nullptr;
FMOD::ChannelGroup* _cgMaster = nullptr;
FMOD::ChannelGroup* _selectedChannelGroup = nullptr;

// DSPs, 2 per group
// _cg1
FMOD::DSP* _dspEcho = nullptr;
FMOD::DSP* _dspDistortion = nullptr;
// _cg2
FMOD::DSP* _dspHighpass = nullptr;
FMOD::DSP* _dspLowpass = nullptr;
// _cg3
FMOD::DSP* _dspTremolo = nullptr;
//FMOD::DSP* _dspOscillator = nullptr;
// _cgMaster
//FMOD::DSP* _dspOscillator = nullptr;
FMOD::DSP* _dspChorus = nullptr;

//Callback
static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	float volumeStepSize = 0.02f;

	float volume = 0.0f;
	float volumeMax = 1.0f;
	float volumeMin = 0.0f;

	bool dspActive = false;
	bool dspBypass = false;
	// Echo
	float echoDelay = 500.0f;
	float echoDelayMin = 10.0f;
	float echoDelayMax = 5000.0f;
	float echoDelayStepSize = 10.0f;
	// Distortion
	float distortion = 0.5f;
	float distortionMin = 0.0f;
	float distortionMax = 1.0f;
	float distortionStepSize = 0.02f;
	// Highpass
	float highpassCutoff = 5000.0f;
	float highpassCutoffMin = 1.0f;
	float highpassCutoffMax = 22000.0f;
	float highpassCutoffStepSize = 100.0f;
	// Lowpass
	float lowpassCutoff = 5000.0f;
	float lowpassCutoffMin = 1.0f;
	float lowpassCutoffMax = 22000.0f;
	float lowpassCutoffStepSize = 100.0f;
	//Tremolo
	float tremoloFrequency = 5.0f;
	float tremoloFrequencyMin = 0.1f;
	float tremoloFrequencyMax = 20.0f;
	float tremoloFrequencyStepSize = 0.1f;
	//// Oscillator
	//float oscillatorRate = 220.0f;
	//float oscillatorRateMin = 0.0f;
	//float oscillatorRateMax = 22000.0f;
	//float oscillatorRateStepSize = 10.0f;
	//// Chorus
	float chorusRate = 0.8f;
	float chorusRateMin = 0.0f;
	float chorusRateMax = 20.0f;
	float chorusRateStepSize = 0.1f;

	//Close the window when users press ESC
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		if (_selectedChannelGroup != _cg1/*_selectedChannelGroup == _cg1 || _selectedChannelGroup == _cgMaster*/)
		{
			if (0 < _vecSounds.size())	//error check to make sure we're not trying to play something that doesn't exist
			{
				_selectedChannelGroup = _cg1;
				_cg1->setPaused(false);
				_cg2->setPaused(true);
				_cg3->setPaused(true);
				_soundIndex = 0;
				// Stop bad, breaks link
				//_cg2->stop();
				//_cg3->stop();

				_selectedChannelGroup->isPlaying(&_isPlaying);
				if (!_isPlaying)
				{
					_result = _system->playSound(_vecSounds.at(0), _cg1, false, &_vecChannels[0]);
					if (_result != FMOD_OK)
					{
						fprintf(stderr, "Unable to play sound\n");
					}
				}
			}
		}
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		if (_selectedChannelGroup != _cg2/*_selectedChannelGroup == _cg2 || _selectedChannelGroup == _cgMaster*/)
		{
			if (1 < _vecSounds.size())
			{
				_selectedChannelGroup = _cg2;
				_cg1->setPaused(true);
				_cg2->setPaused(false);
				_cg3->setPaused(true);
				_soundIndex = 1;
				//_cg1->stop();
				//_cg3->stop();

				_selectedChannelGroup->isPlaying(&_isPlaying);
				if (!_isPlaying)
				{
					_result = _system->playSound(_vecSounds.at(1), _cg2, false, &_vecChannels[1]);
					if (_result != FMOD_OK)
					{
						fprintf(stderr, "Unable to play sound\n");
					}
				}
			}
		}
	}
	else if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		if (_selectedChannelGroup != _cg3/*_selectedChannelGroup == _cg3 || _selectedChannelGroup == _cgMaster*/)
		{
			if (2 < _vecSounds.size())
			{
				_selectedChannelGroup = _cg3;
				_cg1->setPaused(true);
				_cg2->setPaused(true);
				_cg3->setPaused(false);
				_soundIndex = 2;
				//_cg1->stop();
				//_cg2->stop();

				_selectedChannelGroup->isPlaying(&_isPlaying);
				if (!_isPlaying)
				{
					_result = _system->playSound(_vecSounds.at(2), _cg3, false, &_vecChannels[2]);
					if (_result != FMOD_OK)
					{
						fprintf(stderr, "Unable to play sound\n");
					}
				}
			}
		}
	}

	//Pause if TAB is pressed
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
		_result = _selectedChannelGroup->getPaused(&_isPaused);
		if (_result != FMOD_OK) {
			fprintf(stderr, "Unable to read pause");
		}
		_result = _selectedChannelGroup->setPaused(!_isPaused);
		if (_result != FMOD_OK) {
			fprintf(stderr, "Unable to set pause");
		}
		_result = _selectedChannelGroup->getPaused(&_isPaused);
		if (_result != FMOD_OK) {
			fprintf(stderr, "Unable to read pause");
		}
	} //end of TAB

	//Keypad input (copied from channel-groups sample code)
	if (key == GLFW_KEY_UP /*&& action == GLFW_PRESS*/) {
		_result = _selectedChannelGroup->getVolume(&volume);
		if (_result != FMOD_OK) {
			fprintf(stderr, "Unable to read volume");
		}
		//Clamp volume
		volume = (volume >= volumeMax) ? volumeMax : (volume + volumeStepSize);
		_result = _selectedChannelGroup->setVolume(volume);
		if (_result != FMOD_OK) {
			fprintf(stderr, "Unable to set volume");
		}
	} //end of UP
	else if (key == GLFW_KEY_DOWN /*&& action == GLFW_PRESS*/) {
		_result = _selectedChannelGroup->getVolume(&volume);
		if (_result != FMOD_OK) {
			fprintf(stderr, "Unable to read volume");
		}
		//Clamp volume
		volume = (volume <= volumeMin) ? volumeMin : (volume - volumeStepSize);
		_result = _selectedChannelGroup->setVolume(volume);
		if (_result != FMOD_OK) {
			fprintf(stderr, "Unable to set volume");
		}
	} //end of DOWN


	// FMOD DSP Bypass
	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{	// First DSP on the selected channel group
		FMOD::DSP* tempDSP;
		bool tempBypassEnabled;
		_result = _selectedChannelGroup->getDSP(0, &tempDSP);
		if (_result != FMOD_OK)
		{
			fprintf(stderr, "Unable to get DSP from selected channel group\n");
		}
		_result = tempDSP->getBypass(&tempBypassEnabled);
		if (_result != FMOD_OK)
		{
			fprintf(stderr, "Unable to get DSP Bypass from tempDSP\n");
		}
		_result = tempDSP->setBypass(!tempBypassEnabled);
		if (_result != FMOD_OK)
		{
			fprintf(stderr, "Unable to set dsp bypass to opposite of current\n");
		}
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{	// second DSP on the selected channel group
		FMOD::DSP* tempDSP;
		bool tempBypassEnabled;
		_result = _selectedChannelGroup->getDSP(1, &tempDSP);
		if (_result != FMOD_OK)
		{
			fprintf(stderr, "Unable to get DSP from selected channel group\n");
		}
		_result = tempDSP->getBypass(&tempBypassEnabled);
		if (_result != FMOD_OK)
		{
			fprintf(stderr, "Unable to get DSP Bypass from tempDSP\n");
		}
		_result = tempDSP->setBypass(!tempBypassEnabled);
		if (_result != FMOD_OK)
		{
			fprintf(stderr, "Unable to set dsp bypass to opposite of current\n");
		}
	}

	// FMOD DSP Modifications
	// QWER are used depending on which channel is active to change the respective DSP modifiers
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{	// FMOD Echo Delay Up
		if (_selectedChannelGroup == _cg1)
		{
			_result = _dspEcho->getParameterFloat(FMOD_DSP_ECHO_DELAY, &echoDelay, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get echo delay.\n"); }
			echoDelay += echoDelayStepSize;
			echoDelay = (echoDelay >= echoDelayMax) ? echoDelayMax : echoDelay;
			_result = _dspEcho->setParameterFloat(FMOD_DSP_ECHO_DELAY, echoDelay);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set echo delay.\n"); }
		}
		else if (_selectedChannelGroup == _cg2)
		{	// FMOD DSP Highpass Cutoff Up
			_result = _dspHighpass->getParameterFloat(FMOD_DSP_HIGHPASS_CUTOFF, &highpassCutoff, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get highpass cutoff.\n"); }
			highpassCutoff += highpassCutoffStepSize;
			highpassCutoff = (highpassCutoff >= highpassCutoffMax) ? highpassCutoffMax : highpassCutoff;
			_result = _dspHighpass->setParameterFloat(FMOD_DSP_HIGHPASS_CUTOFF, highpassCutoff);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set highpass cutoff.\n"); }
		}
		else if (_selectedChannelGroup == _cg3)
		{ // FMOD DSP Tremolo Frequency Up
			_result = _dspTremolo->getParameterFloat(FMOD_DSP_TREMOLO_FREQUENCY, &tremoloFrequency, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get tremolo frequency.\n"); }
			tremoloFrequency += tremoloFrequencyStepSize;
			tremoloFrequency = (tremoloFrequency >= tremoloFrequencyMax) ? tremoloFrequencyMax : tremoloFrequency;
			_result = _dspTremolo->setParameterFloat(FMOD_DSP_TREMOLO_FREQUENCY, tremoloFrequency);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set tremolo frequency.\n"); }
		}
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{	// FMOD Echo Delay Down
		if (_selectedChannelGroup == _cg1)
		{
			_result = _dspEcho->getParameterFloat(FMOD_DSP_ECHO_DELAY, &echoDelay, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get echo delay.\n"); }
			echoDelay -= echoDelayStepSize;
			echoDelay = (echoDelay <= echoDelayMin) ? echoDelayMin : echoDelay;
			_result = _dspEcho->setParameterFloat(FMOD_DSP_ECHO_DELAY, echoDelay);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set echo delay.\n"); }
		}
		else if (_selectedChannelGroup == _cg2)
		{	// FMOD DSP Highpass Cutoff Down
			_result = _dspHighpass->getParameterFloat(FMOD_DSP_HIGHPASS_CUTOFF, &highpassCutoff, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get highpass cutoff.\n"); }
			highpassCutoff -= highpassCutoffStepSize;
			highpassCutoff = (highpassCutoff <= highpassCutoffMin) ? highpassCutoffMin : highpassCutoff;
			_result = _dspHighpass->setParameterFloat(FMOD_DSP_HIGHPASS_CUTOFF, highpassCutoff);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set highpass cutoff.\n"); }
		}
		else if (_selectedChannelGroup == _cg3)
		{	// FMOD DSP Tremolo Frequency Down
			_result = _dspTremolo->getParameterFloat(FMOD_DSP_TREMOLO_FREQUENCY, &tremoloFrequency, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get tremolo frequency.\n"); }
			tremoloFrequency -= tremoloFrequencyStepSize;
			tremoloFrequency = (tremoloFrequency <= tremoloFrequencyMin) ? tremoloFrequencyMin : tremoloFrequency;
			_result = _dspTremolo->setParameterFloat(FMOD_DSP_TREMOLO_FREQUENCY, tremoloFrequency);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set tremolo frequency.\n"); }
		}
	}
	else if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{	// FMOD Distortion Level Up
		if (_selectedChannelGroup == _cg1)
		{
			_result = _dspDistortion->getParameterFloat(FMOD_DSP_DISTORTION_LEVEL, &distortion, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get distortion level.\n"); }
			distortion += distortionStepSize;
			distortion = (distortion >= distortionMax) ? distortionMax : distortion;
			_result = _dspDistortion->setParameterFloat(FMOD_DSP_DISTORTION_LEVEL, distortion);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set distortion level.\n"); }
		}
		else if (_selectedChannelGroup == _cg2)
		{	// FMOD DSP Lowpass Cutoff Up
			_result = _dspLowpass->getParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, &lowpassCutoff, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get lowpass cutoff.\n"); }
			lowpassCutoff += lowpassCutoffStepSize;
			lowpassCutoff = (lowpassCutoff >= lowpassCutoffMax) ? lowpassCutoffMax : lowpassCutoff;
			_result = _dspLowpass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, lowpassCutoff);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set lowpass cutoff.\n"); }
		}
		else if (_selectedChannelGroup == _cg3)
		{	// FMOD DSP Delay Up
			_result = _dspChorus->getParameterFloat(FMOD_DSP_CHORUS_RATE, &chorusRate, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get chorus rate.\n"); }
			chorusRate += chorusRateStepSize;
			chorusRate = (chorusRate >= chorusRateMax) ? chorusRateMax : chorusRate;
			_result = _dspChorus->setParameterFloat(FMOD_DSP_CHORUS_RATE, chorusRate);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set chorus rate.\n"); }
		}
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{	// FMOD Distortion Level Down
		if (_selectedChannelGroup == _cg1)
		{
			_result = _dspDistortion->getParameterFloat(FMOD_DSP_DISTORTION_LEVEL, &distortion, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get distortion level.\n"); }
			distortion -= distortionStepSize;
			distortion = (distortion <= distortionMin) ? distortionMin : distortion;
			_result = _dspDistortion->setParameterFloat(FMOD_DSP_DISTORTION_LEVEL, distortion);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set distortion level.\n"); }
		}
		else if (_selectedChannelGroup == _cg2)
		{	// FMOD DSP Lowpass Cutoff Down
			_result = _dspLowpass->getParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, &lowpassCutoff, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get lowpass cutoff.\n"); }
			lowpassCutoff -= lowpassCutoffStepSize;
			lowpassCutoff = (lowpassCutoff <= lowpassCutoffMin) ? lowpassCutoffMin : lowpassCutoff;
			_result = _dspLowpass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, lowpassCutoff);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set lowpass cutoff.\n"); }
		}
		else if (_selectedChannelGroup == _cg3)
		{	// FMOD DSP Delay Down
			_result = _dspChorus->getParameterFloat(FMOD_DSP_CHORUS_RATE, &chorusRate, nullptr, 0);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to get chorus rate.\n"); }
			chorusRate -= chorusRateStepSize;
			chorusRate = (chorusRate <= chorusRateMin) ? chorusRateMin : chorusRate;
			_result = _dspChorus->setParameterFloat(FMOD_DSP_CHORUS_RATE, chorusRate);
			if (_result != FMOD_OK) { fprintf(stderr, "Unable to set chorus rate.\n"); }
		}
	} //end of R

} //end of input

void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "FMOD Error(%d): %s", error, description);
}

//Function signatures
bool init();
bool initGLFW();
bool initGL();
bool initFMOD();
void shutDown();

bool loadSounds();

int main(int argc, char* argv)
{
	//init all dependencies
	if (!init()) {
		fprintf(stderr, "Unable to initialize app");
		exit(EXIT_FAILURE);
	}

	std::stringstream ss;
	float volume = 0.f;

	unsigned int dspIndex = 0;
	bool dspBypass = false;

	FMOD::Sound* currentSound = nullptr;
	FMOD::Channel* currentChannel = nullptr;

	float echoDelay = 500.0f;
	float distortion = 0.5f;
	float highpassCutoff = 5000.0f;
	float lowpassCutoff = 5000.0f;
	float tremoloFrequency = 5.0f;
	//float delay = 0.0f;
	//float oscillatorRate = 220.0f;
	float chorusRate = 0.8f;

	ss.str("");
	ss << "Press 1 to play: " << _vecURLs[0];
	_text->addLine(ss.str().c_str(), _textRowIndex++);	//to do, add URLs
	ss.str("");
	ss << "Press 2 to play: " << _vecURLs[1];
	_text->addLine(ss.str().c_str(), _textRowIndex++);
	ss.str("");
	ss << "Press 3 to play: " << _vecURLs[2];
	_text->addLine(ss.str().c_str(), _textRowIndex++);
	ss.str("");

	_text->addLine("Press TAB to pause the currently playing channel group", _textRowIndex++);
	_text->addLine("Use Up and Down Arrow Keys to adjust the volume", _textRowIndex++);

	GLuint lastKnownRowIndex = _textRowIndex;

	//Main loop
	while (!glfwWindowShouldClose(_window)) {
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Your code here
		lastKnownRowIndex = _textRowIndex;

		_result = _system->update();
		if (_result != FMOD_OK)
		{
			fprintf(stderr, "Unable to update fmod system.\n");
		}

		// Get open state for active sound
		_result = _vecSounds.at(_soundIndex)->getOpenState(&_openState, &_percentage, &_isStarving, 0);
		if (_result != FMOD_OK)
		{
			fprintf(stderr, "Unable to get open state for sound at index %d.\n", _soundIndex);
		}

		// Get number of tags
		_result = _vecSounds.at(_soundIndex)->getNumTags(&_numberOfTags, &_numberOfTagsUpdated);

		// Iterate number of tags and grab information
		for (unsigned int i = 0; i < _numberOfTags; i++)
		{
			FMOD_TAG tag;
			_result = _vecSounds.at(_soundIndex)->getTag(0, i, &tag);
			if (_result != FMOD_OK)
			{
				fprintf(stderr, "Unable to get tag with index: %d  for sound at index %d.\n", i, _soundIndex);
			}

			// Check for tag type and compensate sample rate if needed.
			// From documentation: when a song changes, the sample rate may also change, so we need to compensate.
			if (tag.type == FMOD_TAGTYPE_FMOD)
			{
				// check if tag indicates sample rate change
				if (!strcmp(tag.name, "Sample Rate Change") && _selectedChannelGroup/*_vecChannels.at(0)*/)
				{
					float frequency = *((float*)tag.data);

					_result = _vecChannels.at(_soundIndex)->setFrequency(frequency);
					if (_result != FMOD_OK)
					{
						fprintf(stderr, "Unable to set frequency for channel at index %d.\n", _soundIndex);
					}
				}
			}
			else if (tag.datatype == FMOD_TAGDATATYPE_STRING)
			{
				char buffer[255];
				sprintf_s(buffer, "%s = '%s' (%d bytes)", tag.name, (char*)tag.data, tag.datalen);
				_text->addLine(buffer, lastKnownRowIndex++);
			}
		}

		// if channel is active get is playing and is paused
		if (_vecChannels.at(_soundIndex))
		{
			//_result = _vecChannels[0]->getPaused(&_isPaused);
			_result = _selectedChannelGroup->getPaused(&_isPaused);
			if (_result != FMOD_OK)
			{
				fprintf(stderr, "Unable to get is paused for channel at index %d.\n", _soundIndex);
			}

			//_result = _vecChannels.at(0)->isPlaying(&_isPlaying);
			_result = _selectedChannelGroup->isPlaying(&_isPlaying);
			if (_result != FMOD_OK)
			{
				fprintf(stderr, "Unable to get is playing for channel group.\n");
			}

			// if we don't receive enough data from source, mute channel
			//_result = _vecChannels.at(0)->setMute(_isStarving);
			_result = _selectedChannelGroup->setMute(_isStarving);
			if (_result != FMOD_OK)
			{
				fprintf(stderr, "Unable to mute channel group.\n");
			}
		}
		else
		{
			// We need to play a sound then
			//_result = _system->playSound(_vecSounds.at(0), 0, false, &_vecChannels.at(0));
			// potentially a lot of errors because it might not be ready since we're streaming the sound from an external resource
		}

		// Check for state and translate that information into a string that we can print in our console
		if (_openState == FMOD_OPENSTATE_BUFFERING)
		{
			_stateString = "Buffering...";
		}
		else if (_openState = FMOD_OPENSTATE_CONNECTING)
		{
			_stateString = "Connecting...";
		}
		else if (_isPaused)
		{
			_stateString = "Paused...";
		}
		else if (_isPlaying)
		{
			_stateString = "Playing...";
		}

		//_text->addLine("", lastKnownRowIndex++);
		_text->addLine(_stateString, lastKnownRowIndex++);

		//Get selected channel group name
		_result = _selectedChannelGroup->getName(_buffer, BUFFER_SIZE);
		if (_result != FMOD_OK) {
			fprintf(stderr, "Unable to read selected channel group name\n");
		}
		ss.str("");
		ss << "Selected channel group name: " << _buffer;
		_text->addLine(ss.str(), lastKnownRowIndex++);

		//Get the volume of the selected channel
		_result = _selectedChannelGroup->getVolume(&volume);
		if (_result != FMOD_OK) {
			fprintf(stderr, "Unable to read selected channel group volume\n");
		}
		ss.str("");
		ss.precision(2);
		ss << "Selected channel group volume: " << volume;
		_text->addLine(ss.str(), lastKnownRowIndex++);

		// Get the dsp bypass state for the selected channel group
		FMOD::DSP* dspTemp;
		FMOD_DSP_TYPE dspType;
		bool dspBypass;
		_result = _selectedChannelGroup->getDSP(0, &dspTemp);
		if (_result != FMOD_OK)
		{
			fprintf(stderr, "Unable to get DSP from current CG.\n");
		}
		else
		{
			dspTemp->getType(&dspType);
			dspTemp->getBypass(&dspBypass);
			ss.str("");
			if (_selectedChannelGroup == _cg1)
			{
				ss.precision(5);
				_result = _dspEcho->getParameterFloat(FMOD_DSP_ECHO_DELAY, &echoDelay, nullptr, 0);
				if (_result != FMOD_OK)
				{
					fprintf(stderr, "Unable to get echo delay\n");
				}
				ss << "DSP Echo Delay is: "
					<< echoDelay
					<< " and Bypass is: "
					<< ((dspBypass) ? "enabled" : "disabled")
					<< "  Press \"O\" to switch, Q to raise and W to lower.";
				_text->addLine(ss.str().c_str(), lastKnownRowIndex++);

				_result = _selectedChannelGroup->getDSP(1, &dspTemp);
				dspTemp->getType(&dspType);
				dspTemp->getBypass(&dspBypass);
				ss.str("");
				_result = _dspDistortion->getParameterFloat(FMOD_DSP_DISTORTION_LEVEL, &distortion, nullptr, 0);
				if (_result != FMOD_OK)
				{
					fprintf(stderr, "Unable to get distortion level\n");
				}
				ss << "DSP Distortion Level is: "
					<< distortion
					<< " and Bypass is: "
					<< ((dspBypass) ? "enabled" : "disabled")
					<< "  Press \"P\" to switch, E to raise and R to lower.";
				_text->addLine(ss.str().c_str(), lastKnownRowIndex++);
			}
			else if (_selectedChannelGroup == _cg2)
			{
				ss.precision(5);
				_result = _dspHighpass->getParameterFloat(FMOD_DSP_HIGHPASS_CUTOFF, &highpassCutoff, nullptr, 0);
				if (_result != FMOD_OK)
				{
					fprintf(stderr, "Unable to get highpass cutoff\n");
				}
				ss << "DSP Highpass Cutoff is: "
					<< highpassCutoff
					<< " and Bypass is: "
					<< ((dspBypass) ? "enabled" : "disabled")
					<< "  Press \"O\" to switch, Q to raise and W to lower.";
				_text->addLine(ss.str().c_str(), lastKnownRowIndex++);

				_result = _selectedChannelGroup->getDSP(1, &dspTemp);
				dspTemp->getType(&dspType);
				dspTemp->getBypass(&dspBypass);
				ss.str("");
				_result = _dspLowpass->getParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, &lowpassCutoff, nullptr, 0);
				if (_result != FMOD_OK)
				{
					fprintf(stderr, "Unable to get lowpass cutoff\n");
				}
				ss << "DSP Lowpass Cutoff is: "
					<< lowpassCutoff
					<< " and Bypass is: "
					<< ((dspBypass) ? "enabled" : "disabled")
					<< "  Press \"P\" to switch, E to raise and R to lower.";
				_text->addLine(ss.str().c_str(), lastKnownRowIndex++);
			}
			else if (_selectedChannelGroup == _cg3)
			{
				ss.precision(5);
				_result = _dspTremolo->getParameterFloat(FMOD_DSP_TREMOLO_FREQUENCY, &tremoloFrequency, nullptr, 0);
				if (_result != FMOD_OK)
				{
					fprintf(stderr, "Unable to get tremolo frequency\n");
				}
				ss << "DSP Tremolo Frequency is: "
					<< tremoloFrequency
					<< " and Bypass is: "
					<< ((dspBypass) ? "enabled" : "disabled")
					<< "  Press \"O\" to switch, Q to raise and W to lower.";
				_text->addLine(ss.str().c_str(), lastKnownRowIndex++);

				_result = _selectedChannelGroup->getDSP(1, &dspTemp);
				dspTemp->getType(&dspType);
				dspTemp->getBypass(&dspBypass);
				ss.str("");
				_result = _dspChorus->getParameterFloat(FMOD_DSP_CHORUS_RATE, &chorusRate, nullptr, 0);
				if (_result != FMOD_OK)
				{
					fprintf(stderr, "Unable to get oscillator rate\n");
				}
				ss << "DSP Chorus Rate is: "
					<< chorusRate
					<< " and Bypass is: "
					<< ((dspBypass) ? "enabled" : "disabled")
					<< "  Press \"P\" to switch, E to raise and R to lower.";
				_text->addLine(ss.str().c_str(), lastKnownRowIndex++);
			}
		}


		ss.str("");
		ss << "Is paused: " << ((_isPaused) ? "YES" : "NO");
		_text->addLine(ss.str(), lastKnownRowIndex++);

		ss.str("");
		ss << "Is playing: " << ((_isPlaying) ? "YES" : "NO");
		_text->addLine(ss.str(), lastKnownRowIndex++);


		ss.str("");
		ss << "Buffering percentage: " << _percentage << "%";
		_text->addLine(ss.str(), lastKnownRowIndex++);



		//_text->addLine("-------------------------------------------------------", lastKnownRowIndex++);
		_text->addLine("Press ESC to exit.", lastKnownRowIndex++);

		_text->render();

		glfwSwapBuffers(_window);
		glfwPollEvents();
	}

	shutDown();
} //end of main


//Function definitions
// Unchanged
bool init() {

	if (!initGLFW()) {
		return false;
	}
	if (!initGL()) {
		return false;
	}

	//GLText has a dependency with OpenGL context
	_text = new RSS::GLText(_appName, _windowWidth, _windowHeight);

	if (!initFMOD()) {
		return false;
	}

	return true;
} //end of init

// Unchanged
bool initGLFW() {

	if (!glfwInit()) {
		fprintf(stderr, "Unable to initalize GLFW");
		return false;
	}

	//set error callback
	glfwSetErrorCallback(glfw_error_callback);

	_window = glfwCreateWindow(_windowWidth, _windowHeight, _appName.c_str(), NULL, NULL);
	if (!_window) {
		fprintf(stderr, "Unable to create GLFW window");
		return false;
	}

	//set keyboard callback
	glfwSetKeyCallback(_window, glfw_key_callback);
	glfwMakeContextCurrent(_window);
	glfwSwapInterval(1);


	return true;
} //end of initGLFW

// Unchanged
bool initGL() {

	if (!gladLoadGL(glfwGetProcAddress)) {
		fprintf(stderr, "Unable to initialize glad");
		return false;
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	return true;
} //end of initGL

bool initFMOD() {
	_result = FMOD::System_Create(&_system);
	if (_result != FMOD_OK) {
		fprintf(stderr, "Unable to create FMOD system");
		return false;
	}

	_result = _system->init(32, FMOD_INIT_NORMAL, NULL);
	if (_result != FMOD_OK) {
		fprintf(stderr, "Unable to initialize FMOD system");
		return false;
	}

	_result = _system->setStreamBufferSize(64 * 1024, FMOD_TIMEUNIT_RAWBYTES);

	if (!loadSounds())
	{
		fprintf(stderr, "Unable to load sounds!\n");
		return false;
	}

	// Gunna put this in loadSounds()
	/*
	* Increase file buffer size
	* Larger values will consume more memory, whereas smaller values may cause buffer under-run / starvation, etc.
	* caused by large delays in disk access (ie netstream) or CPU usage in slow machines, or by trying to play too many streams at once.
	*
	* if FMOD_TIME_UNIT_RAWBYTES is used, the memory allocated is two times the size passed in, because fmod allocates a double buffer.
	*
	*/

	//_result = _system->setStreamBufferSize(64 * 1024, FMOD_TIMEUNIT_RAWBYTES);
	//if (_result != FMOD_OK)
	//{
	//	fprintf(stderr, "Unable to set file buffer size.\n");
	//	return false;
	//}

	//// 
	//// https://streaming.live365.com/a46209?listenerId=Live365-Widget-AdBlock&aw_0_1st
	//// 
	//// https://streaming.live365.com/a06375?listenerId=Live365-Widget-AdBlock&aw_0_1st.playerid=Live365-Widget&aw_0_1st.skey=1637270507
	//_vecURLs.push_back("https://streaming.live365.com/a06375?listenerId=Live365-Widget-AdBlock&aw_0_1st.playerid=Live365-Widget&aw_0_1st.skey=1637270507");

	//FMOD::Sound* sound = nullptr;
	//FMOD::Channel* channel = nullptr;

	//// Create sound
	//// TODO: we might want to increase the default file chunk size by passing a 'custom' exinfo object.
	//_result = _system->createSound(_vecURLs.at(0).c_str(), FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &sound);
	//if (_result != FMOD_OK)
	//{
	//	fprintf(stderr, "Unable to create sound using url at index 0.\n");
	//	return false;
	//}

	//_vecSounds.push_back(sound);
	//_vecChannels.push_back(channel);

	if (_vecSounds.size() <= 0)
	{
		fprintf(stderr, "Unable to create sounds.\n");
		return false;
	}

	//Create groups
	_result = _system->createChannelGroup("Channel Group 1", &_cg1);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to create channel group");
		return false;
	}

	_result = _system->createChannelGroup("Channel Group 2", &_cg2);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to create channel group");
		return false;
	}

	_result = _system->createChannelGroup("Channel Group 3", &_cg3);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to create channel group");
		return false;
	}

	//Get master channel
	_result = _system->getMasterChannelGroup(&_cgMaster);
	if (_result != FMOD_OK) {
		fprintf(stderr, "Unable to get master channel group");
		return false;
	}

	_selectedChannelGroup = _cgMaster;
	//_selectedChannelGroup = _cg1;


	// Create DSP echo on _cg1
	_result = _system->createDSPByType(FMOD_DSP_TYPE_ECHO, &_dspEcho);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to create dsp echo\n");
		return false;
	}
	//Append dsp to channel group
	_result = _cg1->addDSP(0, _dspEcho);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to add dsp to _cg1\n");
		return false;
	}
	//Enable dsp
	_result = _dspEcho->setActive(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to enable _dspEcho\n");
		return false;
	}
	//Bypass dsp for now
	_result = _dspEcho->setBypass(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to set dsp bypass\n");
		return false;
	}

	// Create DSP distortion on _cg1
	_result = _system->createDSPByType(FMOD_DSP_TYPE_DISTORTION, &_dspDistortion);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to create dsp distortion\n");
		return false;
	}
	//Append dsp to channel group
	_result = _cg1->addDSP(1, _dspDistortion);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to add dsp to _cg1\n");
		return false;
	}
	//Enable dsp
	_result = _dspDistortion->setActive(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to enable _dspDistortion\n");
		return false;
	}
	//Bypass dsp for now
	_result = _dspDistortion->setBypass(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to set dsp bypass\n");
		return false;
	}


	// Create DSP highpass on _cg2
	_result = _system->createDSPByType(FMOD_DSP_TYPE_HIGHPASS, &_dspHighpass);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to create dsp highpass");
		return false;
	}
	//Append dsp to channel group
	_result = _cg2->addDSP(0, _dspHighpass);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to add dsp to _cg2\n");
		return false;
	}
	//Enable dsp
	_result = _dspHighpass->setActive(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to enable _dspHighpass\n");
		return false;
	}
	//Bypass dsp for now
	_result = _dspHighpass->setBypass(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to set dsp bypass\n");
		return false;
	}

	// Create DSP lowpass on _cg2
	_result = _system->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &_dspLowpass);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to create dsp lowpass\n");
		return false;
	}
	//Append dsp to channel group
	_result = _cg2->addDSP(1, _dspLowpass);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to add dsp to _cg2\n");
		return false;
	}
	//Enable dsp
	_result = _dspLowpass->setActive(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to enable _dspLowpass");
		return false;
	}
	//Bypass dsp for now
	_result = _dspLowpass->setBypass(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to set dsp bypass");
		return false;
	}


	// Create DSP tremolo on _cg3
	_result = _system->createDSPByType(FMOD_DSP_TYPE_TREMOLO, &_dspTremolo);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to create dsp tremolo\n");
		return false;
	}
	//Append dsp to channel group
	_result = _cg3->addDSP(0, _dspTremolo);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to add dsp to _cg3\n");
		return false;
	}
	//Enable dsp
	_result = _dspTremolo->setActive(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to enable _dspTremolo\n");
		return false;
	}
	//Bypass dsp for now
	_result = _dspTremolo->setBypass(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to set dsp bypass\n");
		return false;
	}

	// Create DSP chorus on _cg3
	_result = _system->createDSPByType(FMOD_DSP_TYPE_CHORUS, &_dspChorus);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to create dsp delay\n");
		return false;
	}
	//Append dsp to channel group
	_result = _cg3->addDSP(1, _dspChorus);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to add dsp to _cg3\n");
		return false;
	}
	//Enable dsp
	_result = _dspChorus->setActive(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to enable _dspDelay\n");
		return false;
	}
	//Bypass dsp for now
	_result = _dspChorus->setBypass(true);
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to set dsp bypass\n");
		return false;
	}


	return true;
}

void shutDown() {

	glfwTerminate();

	// Start by stopping channel(s), my code handles one channel only!
	//if (_vecChannels.at(0))
	//{
	//	_result = _vecChannels.at(0)->stop();
	//	if (_result != FMOD_OK)
	//	{
	//		fprintf(stderr, "Unable to stop channel at index 0.\n");
	//	}
	//}

	// Stop all the channels
	for (unsigned int index = 0; index < _vecChannels.size(); index++)
	{
		if (_vecChannels.at(index))		// can't stop something that doesn't exist
		{
			_result = _vecChannels[index]->stop();
			if (_result != FMOD_OK)
			{
				fprintf(stderr, "Unable to stop channel at index %d.\n", index);
			}
		}
	}

	// for the sound(s) we follow a different approach as we are playing sounds from the internet
	//while (_openState != FMOD_OPENSTATE_READY)
	//{
	//	_result = _system->update();
	//	if (_result != FMOD_OK)
	//	{
	//		fprintf(stderr, "Unable to update fmod system.\n");
	//	}

	//	_result = _vecSounds.at(0)->getOpenState(&_openState, 0, 0, 0);
	//	if (_result != FMOD_OK)
	//	{
	//		fprintf(stderr, "Unable to get open state from sound at index 0.\n");
	//	}
	//}

	for (unsigned int index = 0; index < _vecSounds.size(); index++)
	{
		while (_openState != FMOD_OPENSTATE_READY)
		{
			_result = _system->update();
			if (_result != FMOD_OK)
			{
				fprintf(stderr, "Unable to update fmod system.\n");
			}

			_result = _vecSounds.at(index)->getOpenState(&_openState, 0, 0, 0);
			if (_result != FMOD_OK)
			{
				fprintf(stderr, "Unable to get open state from sound at index 0.\n");
			}
		}
		// Release the sound once it's fine to release it
		_result = _vecSounds[index]->release();
		if (_result != FMOD_OK) {
			fprintf(stderr, "Unable to release sound\n");
		}

		if (index + 1 < _vecSounds.size())
		{
			_result = _vecSounds.at(index + 1)->getOpenState(&_openState, 0, 0, 0);
			if (_result != FMOD_OK)
			{
				fprintf(stderr, "Unable to get open state from sound at index 0.\n");
			}
		}
	}


	//Release sounds
	//std::vector<FMOD::Sound*>::iterator itSounds = _vecSounds.begin();
	//for (itSounds; itSounds != _vecSounds.end(); itSounds++) {
	//	_result = (*itSounds)->release();
	//	if (_result != FMOD_OK) {
	//		fprintf(stderr, "Unable to release sound");
	//	}
	//}

//	// DSPs, 2 per group
//// _cg1
//	FMOD::DSP* _dspEcho = nullptr;
//	FMOD::DSP* _dspDistortion = nullptr;
//	// _cg2
//	FMOD::DSP* _dspHighpass = nullptr;
//	FMOD::DSP* _dspLowpass = nullptr;
//	// _cg3
//	FMOD::DSP* _dspTremolo = nullptr;
//	FMOD::DSP* _dspDelay = nullptr;

	// Remove and release everything on _cg1
	_cg1->removeDSP(_dspEcho);
	_result = _dspEcho->release();
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to release dsp");
	}
	_cg1->removeDSP(_dspDistortion);
	_result = _dspDistortion->release();
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to release dsp");
	}

	// Remove and release everything on _cg2
	_cg2->removeDSP(_dspHighpass);
	_result = _dspHighpass->release();
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to release dsp");
	}
	_cg2->removeDSP(_dspLowpass);
	_result = _dspLowpass->release();
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to release dsp");
	}

	// Remove and release everything on _cg3
	_cg3->removeDSP(_dspTremolo);
	_result = _dspTremolo->release();
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to release dsp");
	}
	_cg3->removeDSP(_dspChorus);
	_result = _dspChorus->release();
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to release dsp");
	}

	// Release the channel groups
	_result = _cg1->release();
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to release _cg1");
	}
	_result = _cg2->release();
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to release _cg2");
	}
	_result = _cg3->release();
	if (_result != FMOD_OK)
	{
		fprintf(stderr, "Unable to release _cg3");
	}


	_result = _system->close();
	if (_result != FMOD_OK) {
		fprintf(stderr, "Unable to close system");
	}

	_result = _system->release();
	if (_result != FMOD_OK) {
		fprintf(stderr, "Unable to release system");
	}

	if (_text) {
		delete _text;
		_text = nullptr;
	}


	exit(EXIT_SUCCESS);
} //end of shutdown

// This will be much different than previous loadSounds.  We're going to be reading the URLS for different sounds we want to stream from the internet
bool loadSounds()
{
	std::stringstream ss;

	ss << SOLUTION_DIR << "common\\assets\\audio\\internetURLs.txt";



	std::ifstream theFile(ss.str());
	ss.str("");


	//if (_result != FMOD_OK)
	//{
	//	fprintf(stderr, "Unable to set file buffer size.\n");
	//	return false;
	//}

	if (!theFile.is_open())
	{
		fprintf(stderr, "Could not open internetURLs.txt");
		return false;
	}

	std::string nextToken;
	unsigned int index = 0;
	while (theFile >> nextToken)
	{
		FMOD::Sound* sound = nullptr;
		FMOD::Channel* channel = nullptr;

		_vecURLs.push_back(nextToken.c_str());
		_result = _system->createSound(_vecURLs.at(index).c_str(), FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &sound);
		if (_result != FMOD_OK)
		{
			fprintf(stderr, "Unable to create sound using url at index: %d.\n", index);
		}

		_vecSounds.push_back(sound);
		_vecChannels.push_back(channel);

		index++;
	} //end while

	theFile.close();
	return true;
} //end of loadSounds
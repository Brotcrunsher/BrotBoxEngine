#include "BBE/SoundManager.h"
#include "BBE/Exceptions.h"
#include "portaudio.h"
#include <algorithm>

bbe::INTERNAL::SoundManager* bbe::INTERNAL::SoundManager::m_pinstance = nullptr;

bbe::INTERNAL::SoundManager* bbe::INTERNAL::SoundManager::getInstance()
{
	return bbe::INTERNAL::SoundManager::m_pinstance;
}

bbe::INTERNAL::SoundManager::SoundManager()
{
	if (m_pinstance != nullptr)
	{
		throw AlreadyCreatedException();
	}
	m_pinstance = this;
}

bbe::INTERNAL::SoundManager::~SoundManager()
{
	if (m_pinstance == this)
	{
		m_pinstance = nullptr;
	}
}

static int soundCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
	bbe::INTERNAL::SoundManager* soundManager = static_cast<bbe::INTERNAL::SoundManager*>(userData);
	soundManager->soundCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);

	return 0;
}

int bbe::INTERNAL::SoundManager::soundCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags)
{
	float* out = (float*)outputBuffer;

	memset(out, 0, sizeof(float) * framesPerBuffer * 2);

	for (unsigned long i = 0; i < framesPerBuffer; i++)
	{
		for (std::pair<const uint64_t, SoundInstanceData>& elem : playingSounds)
		{
			SoundInstanceData& sd = elem.second;
			const Sound* sound = sd.m_psound;
			if (sd.m_markedForDeletion)
			{
				continue;
			}
			if (sd.m_sample >= sound->getAmountOfSamples() && sound->isLooped())
			{
				sd.m_sample = 0;
			}
			if (sd.m_sample >= sound->getAmountOfSamples())
			{
				sd.m_markedForDeletion = true;
			}
			else
			{
				auto sample = sound->getSample(sd.m_sample);
				out[i * 2 + 0] += sample.first * sd.m_volume;
				out[i * 2 + 1] += sample.second * sd.m_volume;
				sd.m_sample++;
			}
		}
	}

	//Cleanup. To avoid sound glitches due to long mutex waits we
	//do this very passively by only doing the cleanup if we happen
	//to get the mutex. If we don't, we try again in the next call.
	if (playingSoundsChangeMutex.try_lock())
	{
		for (auto it = playingSounds.begin(); it != playingSounds.end();)
		{
			if (it->second.m_markedForDeletion)
			{
				it = playingSounds.erase(it);
			}
			else
			{
				it++;
			}
		}
		playingSoundsChangeMutex.unlock();
	}
	return 0;
}

void bbe::INTERNAL::SoundManager::stopSoundWithIndex(uint64_t index)
{
	std::lock_guard<std::mutex> lock(playingSoundsChangeMutex);
	auto it = playingSounds.find(index);
	if (it != playingSounds.end())
	{
		it->second.m_markedForDeletion = true;
	}
}

bool bbe::INTERNAL::SoundManager::isSoundWithIndexPlaying(uint64_t index)
{
	std::lock_guard<std::mutex> lock(playingSoundsChangeMutex);
	auto it = playingSounds.find(index);
	if (it != playingSounds.end())
	{
		return !it->second.m_markedForDeletion;
	}
	return false;
}

void bbe::INTERNAL::SoundManager::init()
{
	Pa_Initialize();

	// In case we ever need microphone support, here is how to use it:
	// PaStreamParameters inputParameters;
	// inputParameters.device = Pa_GetDefaultInputDevice();
	// inputParameters.channelCount = 1;
	// inputParameters.sampleFormat = paFloat32;
	// inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency / 2;
	// inputParameters.hostApiSpecificStreamInfo = nullptr;
	PaStreamParameters outputParameters;
	outputParameters.device = Pa_GetDefaultOutputDevice();
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowInputLatency / 2;
	outputParameters.hostApiSpecificStreamInfo = nullptr;
	PaStream* stream = nullptr;
	PaError err = Pa_OpenStream(&stream,
		nullptr,
		&outputParameters,
		44100,
		256,
		paNoFlag,
		::soundCallback,
		this);
	Pa_StartStream(stream);
}

void bbe::INTERNAL::SoundManager::destroy()
{
	Pa_Terminate();
}

bbe::SoundInstance bbe::INTERNAL::SoundManager::play(const Sound& sound, float volume)
{
	SoundInstanceData sid;
	uint64_t index = getNextIndex();
	sid.m_sample = 0;
	sid.m_psound = &sound;
	sid.m_volume = volume;
	SoundInstance si(index);

	{
		std::lock_guard<std::mutex> lock(playingSoundsChangeMutex);
		playingSounds.insert({ index, sid });
	}

	return si;
}

uint64_t bbe::INTERNAL::SoundManager::getNextIndex()
{
	const uint64_t retVal = nextIndex;
	nextIndex++;
	return retVal;
}

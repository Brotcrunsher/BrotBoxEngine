#ifndef BBE_NO_AUDIO

#include "BBE/SoundManager.h"
#include "BBE/Exceptions.h"
#include <algorithm>
#include <iostream>

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

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

void bbe::INTERNAL::SoundManager::stopSoundWithIndex(uint64_t index)
{
	if (!initSuccessful) return;
	std::lock_guard<std::mutex> lock(playingSoundsChangeMutex);
	auto it = playingSounds.find(index);
	if (it != playingSounds.end())
	{
		it->second.destroy(this);
		playingSounds.erase(index);
	}
}

bool bbe::INTERNAL::SoundManager::isSoundWithIndexPlaying(uint64_t index)
{
	if (!initSuccessful) return false;
	std::lock_guard<std::mutex> lock(playingSoundsChangeMutex);
	auto it = playingSounds.find(index);
	if (it != playingSounds.end())
	{
		SoundInstanceData& sid = it->second;
		ALint state = 0;
		alGetSourcei(it->second.source, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
	}
	return false;
}

ALuint bbe::INTERNAL::SoundManager::getNewBuffer()
{
	if (unusedBuffers.getLength() > 0)
	{
		return unusedBuffers.popBack();
	}
	ALuint buffer = 0;
	alGenBuffers(1, &buffer);
	return buffer;
}

void bbe::INTERNAL::SoundManager::freeBuffer(ALuint buffer)
{
	unusedBuffers.add(buffer);
}

void bbe::INTERNAL::SoundManager::update()
{
	for (auto it = playingSounds.begin(); it != playingSounds.end(); /*no inc*/)
	{
		if (!isSoundWithIndexPlaying(it->first))
		{
			it->second.destroy(this);
			playingSounds.erase(it++);
		}
		else
		{
			++it;
		}
	}

	for (auto it = playingSounds.begin(); it != playingSounds.end(); ++it)
	{
		SoundInstanceData& sid = it->second;
		sid.freeUsedUpBuffers(this);
		if (sid.isBufferLoadingRequired())
		{
			sid.loadNewBuffer(getNewBuffer(), this);
		}
	}
}

void bbe::INTERNAL::SoundManager::init()
{
	ALCdevice* device = alcOpenDevice(nullptr);
	if (!device)
	{
		std::cout << "Could not init Sound Manager! Device was null." << std::endl;
		return;
	}
	ALCcontext* context = alcCreateContext(device, nullptr);
	if (!context || !alcMakeContextCurrent(context))
	{
		if (context) alcDestroyContext(context);
		alcCloseDevice(device);
		std::cout << "Could not init Sound Manager! Failed to create context." << std::endl;
		return;
	}

	if (!alIsExtensionPresent("AL_EXT_float32"))
	{
		std::cout << "Could not init Sound Manager! AL_EXT_float32 not present." << std::endl;
		destroy();
		return;
	}

	initSuccessful = true;
}

void bbe::INTERNAL::SoundManager::destroy()
{
	for (auto [index, data] : playingSounds)
	{
		data.destroy(this);
	}
	playingSounds.clear();
	alDeleteBuffers(unusedBuffers.getLength(), unusedBuffers.getRaw());
	unusedBuffers.clear();
	ALCcontext* context = alcGetCurrentContext();
	if (!context) return;

	ALCdevice* device = alcGetContextsDevice(context);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

bbe::SoundInstance bbe::INTERNAL::SoundManager::play(const SoundDataSource& sound, float volume)
{
	SoundInstanceData sid;
	uint64_t index = getNextIndex();
	sid.m_psound = &sound;
	sid.m_volume = volume;
	SoundInstance si(index);

	if(initSuccessful)
	{
		std::lock_guard<std::mutex> lock(playingSoundsChangeMutex);
		sid.start(getNewBuffer(), this);
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

bool bbe::INTERNAL::SoundInstanceData::areAllSamplesLoaded() const
{
	return m_samples_loaded >= m_psound->getAmountOfSamples();
}

bool bbe::INTERNAL::SoundInstanceData::isBufferLoadingRequired() const
{
	return !areAllSamplesLoaded() && buffers.getLength() < 10;
}

void bbe::INTERNAL::SoundInstanceData::loadNewBuffer(ALuint buffer, SoundManager* sm)
{
	List<bbe::Vector2> data;
	constexpr size_t maxBufferSize = 5000;
	const size_t samplesLeft = m_psound->getAmountOfSamples() - m_samples_loaded;
	const size_t bufferSize = bbe::Math::min(maxBufferSize, samplesLeft);
	data.resizeCapacityAndLength(bufferSize);
	for (size_t i = 0; i < bufferSize; i++)
	{
		data[i] = m_psound->getSample(i + m_samples_loaded);
	}
	m_samples_loaded += bufferSize;

	alBufferData(buffer, AL_FORMAT_STEREO_FLOAT32, data.getRaw(), sizeof(bbe::Vector2) * data.getLength(), m_psound->getHz());
	ALenum err = alGetError();
	if (err != ALC_NO_ERROR)
	{
		std::cout << "Something went wrong when loading buffer contents! " << err << std::endl;
		sm->freeBuffer(buffer);
		throw IllegalStateException();
	}
	buffers.add(buffer);

	alSourceQueueBuffers(source, 1, &buffer);
}

void bbe::INTERNAL::SoundInstanceData::start(ALuint buffer, SoundManager* sm)
{
	alGenSources(1, &source);
	loadNewBuffer(buffer, sm);

	ALenum err = alGetError();
	if (err != ALC_NO_ERROR)
	{
		std::cout << "Something went wrong when creating a ALSource! " << err << std::endl;
		return;
	}
	alSourcePlay(source);
}

void bbe::INTERNAL::SoundInstanceData::freeUsedUpBuffers(SoundManager* sm)
{
	ALint usedUpBuffers = 0;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &usedUpBuffers);

	for(ALint i = 0; i<usedUpBuffers; i++)
	{
		ALuint buffer = buffers[0];
		buffers.removeIndex(0);
		alSourceUnqueueBuffers(source, 1, &buffer);
		sm->freeBuffer(buffer);
	}
}

void bbe::INTERNAL::SoundInstanceData::destroy(SoundManager* sm)
{
	if (source)
	{
		alDeleteSources(1, &source);
		source = 0;
	}
	for (ALuint buffer : buffers)
	{
		sm->freeBuffer(buffer);
	}
	buffers.clear();
}

#endif

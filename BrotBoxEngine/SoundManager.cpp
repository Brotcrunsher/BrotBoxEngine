#ifndef BBE_NO_AUDIO

#include "BBE/SoundManager.h"
#include "BBE/Exceptions.h"
#include <algorithm>
#include <iostream>
#include <mutex>

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

namespace bbe
{
	struct SoundDataSource;
}

// No mutexes:
static bbe::List<ALuint> unusedBuffers;

static std::mutex stopRequestsMutex;
static bbe::List<uint64_t> stopRequests;

static std::mutex listenerMutex;
static bbe::Vector3 listenerPos(0, 0, 0);
static bbe::Vector3 listenerDirection(0, 0, 1);

static std::mutex setPositionRequestMutex;
struct SetPositionRequest
{
	uint64_t index = 0;
	bbe::Vector3 pos;
};
static bbe::List<SetPositionRequest> setPositionRequests;

static std::mutex playRequestsMutex;
struct PlayRequest
{
	uint64_t index = 0;
	const bbe::SoundDataSource* sound = nullptr;
	bool posAvailable = false;
	bbe::Vector3 pos;
	float volume = 0.0f;
};
static bbe::List<PlayRequest> playRequests;

static ALuint getNewBuffer()
{
	if (unusedBuffers.getLength() > 0)
	{
		return unusedBuffers.popBack();
	}
	ALuint buffer = 0;
	alGenBuffers(1, &buffer);
	return buffer;
}

static void freeBuffer(ALuint buffer)
{
	unusedBuffers.add(buffer);
}

struct SoundInstanceData
{
	uint64_t m_samples_loaded = 0;
	const bbe::SoundDataSource* m_psound = nullptr;
	float m_volume = 0;
	ALuint source = 0;
	bbe::List<ALuint> buffers;
	bool playing = true;
	bool posAvailable = false;
	bbe::Vector3 pos;

	bool areAllSamplesLoaded() const
	{
		return m_samples_loaded >= m_psound->getAmountOfSamples();
	}

	bool isBufferLoadingRequired() const
	{
		return !areAllSamplesLoaded() && buffers.getLength() < 10;
	}

	void loadNewBuffer(ALuint buffer)
	{
		constexpr size_t maxBufferSize = 5000;
		const uint32_t channels = m_psound->getAmountOfChannels();
		const size_t samplesLeft = m_psound->getAmountOfSamples() - m_samples_loaded;
		const size_t floatsLeft = samplesLeft * channels;
		const size_t bufferSize = bbe::Math::min(maxBufferSize, floatsLeft);
		bbe::List<float> data;
		data.resizeCapacityAndLengthUninit(bufferSize);
		int openAlType = 0;
		switch (channels)
		{
		case(1):
			openAlType = AL_FORMAT_MONO_FLOAT32;
			break;
		case(2):
			openAlType = AL_FORMAT_STEREO_FLOAT32;
			break;
		default:
			throw bbe::IllegalArgumentException();
		}
		for (size_t i = 0; i < bufferSize; i += channels)
		{
			for (uint32_t channel = 0; channel < channels; channel++)
			{
				data[i + channel] = m_psound->getSample(m_samples_loaded, channel);
			}
			m_samples_loaded++;
		}

		alBufferData(buffer, openAlType, data.getRaw(), sizeof(float) * data.getLength(), m_psound->getHz());
		ALenum err = alGetError();
		if (err != ALC_NO_ERROR)
		{
			std::cout << "Something went wrong when loading buffer contents! " << err << std::endl;
			freeBuffer(buffer);
			throw bbe::IllegalStateException();
		}
		buffers.add(buffer);

		alSourceQueueBuffers(source, 1, &buffer);
	}

	void start(ALuint buffer, const bbe::Vector3* pos)
	{
		alGenSources(1, &source);
		loadNewBuffer(buffer);

		ALenum err = alGetError();
		if (err != ALC_NO_ERROR)
		{
			std::cout << "Something went wrong when creating a ALSource! " << err << std::endl;
			return;
		}
		if (pos)
		{
			alSource3f(source, AL_POSITION, pos->x, pos->y, pos->z);
		}
		else
		{
			alSource3f(source, AL_POSITION, 0, 0, 0);
			alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
		}
		alSourcePlay(source);
	}

	void freeUsedUpBuffers()
	{
		ALint usedUpBuffers = 0;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &usedUpBuffers);

		for (ALint i = 0; i < usedUpBuffers; i++)
		{
			ALuint buffer = buffers[0];
			buffers.removeIndex(0);
			alSourceUnqueueBuffers(source, 1, &buffer);
			freeBuffer(buffer);
		}
	}

	void destroy()
	{
		if (source)
		{
			alDeleteSources(1, &source);
			source = 0;
		}
		for (ALuint buffer : buffers)
		{
			freeBuffer(buffer);
		}
		buffers.clear();
	}
};
static std::mutex playingSoundsMutex;
static std::map<uint64_t, SoundInstanceData> playingSounds;

static void destroySoundSystem()
{
	for (auto [index, data] : playingSounds)
	{
		data.destroy();
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

static bool initSoundSystem()
{
	ALCdevice* device = alcOpenDevice(nullptr);
	if (!device)
	{
		std::cout << "Could not init Sound Manager! Device was null." << std::endl;
		return false;
	}
	ALCcontext* context = alcCreateContext(device, nullptr);
	if (!context || !alcMakeContextCurrent(context))
	{
		if (context) alcDestroyContext(context);
		alcCloseDevice(device);
		std::cout << "Could not init Sound Manager! Failed to create context." << std::endl;
		return false;
	}

	if (!alIsExtensionPresent("AL_EXT_float32"))
	{
		std::cout << "Could not init Sound Manager! AL_EXT_float32 not present." << std::endl;
		destroySoundSystem();
		return false;
	}
	
	return true;
}

static void updateSoundSystem()
{
	{
		std::lock_guard<std::mutex> guard(listenerMutex);
		alListener3f(AL_POSITION, listenerPos.x, listenerPos.y, listenerPos.z);

		// Look direction followed by the Up direction
		float arr[] = { listenerDirection.x, listenerDirection.y, listenerDirection.z, 0, 0, 1 };

		alListenerfv(AL_ORIENTATION, arr);
	}

	std::lock_guard<std::mutex> playingSoundsGuard(playingSoundsMutex);
	{
		std::lock_guard<std::mutex> guard(setPositionRequestMutex);
		for (const SetPositionRequest& spr : setPositionRequests)
		{
			auto it = playingSounds.find(spr.index);
			if (it != playingSounds.end())
			{
				it->second.pos = spr.pos;
			}
		}
		setPositionRequests.clear();
	}

	{
		std::lock_guard<std::mutex> guard(playRequestsMutex);
		for (const PlayRequest& pr : playRequests)
		{
			SoundInstanceData sid;
			sid.m_psound = pr.sound;
			sid.m_volume = pr.volume;
			sid.pos = pr.pos;
			sid.posAvailable = pr.posAvailable;

			sid.start(getNewBuffer(), pr.posAvailable ? &pr.pos : nullptr);
			playingSounds.insert({ pr.index, sid });
		}
		playRequests.clear();
	}

	for (auto it = playingSounds.begin(); it != playingSounds.end(); ++it)
	{
		SoundInstanceData& sid = it->second;
		sid.freeUsedUpBuffers();
		if (sid.isBufferLoadingRequired())
		{
			sid.loadNewBuffer(getNewBuffer());
		}

		ALint state = 0;
		alGetSourcei(it->second.source, AL_SOURCE_STATE, &state);
		sid.playing = state == AL_PLAYING;

		if (sid.playing && sid.posAvailable)
		{
			alSource3f(sid.source, AL_POSITION, sid.pos.x, sid.pos.y, sid.pos.z);
		}
	}

	for (auto it = playingSounds.begin(); it != playingSounds.end(); /*no inc*/)
	{
		if (!it->second.playing)
		{
			it->second.destroy();
			playingSounds.erase(it++);
		}
		else
		{
			++it;
		}
	}

	{
		std::lock_guard<std::mutex> guard(stopRequestsMutex);
		for (int64_t index : stopRequests)
		{
			auto it = playingSounds.find(index);
			if (it != playingSounds.end())
			{
				it->second.destroy();
				playingSounds.erase(index);
			}
		}
		stopRequests.clear();
	}
}

static std::mutex endRequestedMutex;
static bool endRequested = false;
static void requestEnd()
{
	std::lock_guard<std::mutex> guard(endRequestedMutex);
	endRequested = true;
}
static bool isEndRequested()
{
	std::lock_guard<std::mutex> guard(endRequestedMutex);
	return endRequested;
}

static void soundSystemMain()
{
	if (!initSoundSystem())
	{
		destroySoundSystem();
		return;
	}
	while (!isEndRequested())
	{
		updateSoundSystem();
		std::this_thread::sleep_for(std::chrono::milliseconds(1)); // A more relaxed version of yield...
	}
	destroySoundSystem();
}

static bbe::INTERNAL::SoundManager* m_pinstance = nullptr;

// ***************************************************************************************************************
// ***                                                                                                         ***
// ***    CAREFUL: Below this line, no al functions must be called directly, as we are not on the al thread!   ***
// ***                                                                                                         ***
// ***************************************************************************************************************

bbe::INTERNAL::SoundManager* bbe::INTERNAL::SoundManager::getInstance()
{
	return m_pinstance;
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
	std::lock_guard<std::mutex> guard(stopRequestsMutex);
	stopRequests.add(index);
}

bool bbe::INTERNAL::SoundManager::isSoundWithIndexPlaying(uint64_t index)
{
	// TODO: Aquiring this lock is potentially quite expensive, because the sound thread is running constantly
	std::lock_guard<std::mutex> guard(playingSoundsMutex);
	auto it = playingSounds.find(index);
	if (it != playingSounds.end())
	{
		SoundInstanceData& sid = it->second;
		return sid.playing;
	}
	return false;
}

void bbe::INTERNAL::SoundManager::setPosition(uint64_t index, const bbe::Vector3& pos)
{
	std::lock_guard<std::mutex> guard(setPositionRequestMutex);
	setPositionRequests.add(SetPositionRequest{
		index,
		pos
		});
}

void bbe::INTERNAL::SoundManager::setSoundListener(const bbe::Vector3& pos, const bbe::Vector3& lookDirection)
{
	std::lock_guard<std::mutex> guard(listenerMutex);
	listenerPos = pos;
	listenerDirection = lookDirection;
}

void bbe::INTERNAL::SoundManager::update()
{
#ifdef __EMSCRIPTEN__
	updateSoundSystem();
#else
	// Do nothing, the thread updates itself.
#endif
}

void bbe::INTERNAL::SoundManager::init()
{
#ifdef __EMSCRIPTEN__
	initSoundSystem();
#else
	soundSystemThread = std::thread(soundSystemMain);
#endif
}

void bbe::INTERNAL::SoundManager::destroy()
{
#ifdef __EMSCRIPTEN__
	destroySoundSystem();
#else
	requestEnd();
	soundSystemThread.join();
#endif
}

bbe::SoundInstance bbe::INTERNAL::SoundManager::play(const SoundDataSource& sound, const bbe::Vector3* pos, float volume)
{
	uint64_t index = getNextIndex();

	{
		std::lock_guard<std::mutex> guard(playRequestsMutex);
		playRequests.add(PlayRequest{
			index,
			&sound,
			pos != nullptr,
			pos != nullptr ? *pos : bbe::Vector3(),
			volume
			});
	}

	return SoundInstance(index);
}

uint64_t bbe::INTERNAL::SoundManager::getNextIndex()
{
	const uint64_t retVal = nextIndex;
	nextIndex++;
	return retVal;
}

#endif

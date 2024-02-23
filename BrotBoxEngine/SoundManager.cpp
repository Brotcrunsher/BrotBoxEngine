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
static bbe::List<ALuint> buffers;

static std::mutex stopRequestsMutex;
static bbe::List<uint64_t> stopRequests;

static std::mutex listenerMutex;
static bbe::Vector3 listenerPos(0, 0, 0);
static bbe::Vector3 listenerDirection(0, 0, 1);
static bbe::Vector3 listenerPosPreviousBufferCalculation = listenerPos;
static bbe::Vector3 listenerDirectionPreviousBufferCalculation = listenerDirection;

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

static ALuint mainSource = 0;
static bool previouslyDied = false;

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
	float m_volume = 1.0f;
	bool posAvailable = false;
	bbe::Vector3 pos;
	bbe::Vector3 posPreviousLoad;

	bool stopRequested = false;
	bool readyForDelete = false;

	void loadBuffer(bbe::Vector2 *samples, size_t numSamples)
	{
		const uint32_t channels = m_psound->getAmountOfChannels();
		for (size_t i = 0; i < numSamples; i++)
		{
			if (m_samples_loaded == m_psound->getAmountOfSamples()) break;

			bbe::Vector2 sample;
			float percentage = (float)i / (float)numSamples;

			if (channels == 1)
			{
				float leftMult = 1.0f;
				float rightMult = 1.0f;

				if (posAvailable)
				{
					bbe::Vector3 lerpedPos = bbe::Math::interpolateLinear(posPreviousLoad, pos, percentage);
					bbe::Vector3 lerpedListenerPos = bbe::Math::interpolateLinear(listenerPosPreviousBufferCalculation, listenerPos, percentage);

					bbe::Vector3 toListener = lerpedListenerPos - lerpedPos;
					bbe::Vector2 toListener2d = bbe::Vector2(toListener.x, toListener.y);
					bbe::Vector2 toListener2dNorm = toListener2d.normalize();

					// TODO: Slerp instead?
					bbe::Vector3 lerpedDirection = bbe::Math::interpolateLinear(listenerDirectionPreviousBufferCalculation, listenerDirection, percentage).normalize();

					bbe::Vector2 listenerRight = bbe::Vector2(-lerpedDirection.y, lerpedDirection.x).normalize();

					//TODO: This isn't quite right. When looking directly at a sound, it still is coming more from one side than the other
					rightMult = toListener2dNorm * listenerRight;
					leftMult = 1.0f - rightMult;

					float distSq = toListener.getLengthSq();
					if (distSq < 1.0f) distSq = 1.0f;
					rightMult /= distSq;
					leftMult /= distSq;
				}

				float val = m_psound->getSample(m_samples_loaded, 0);
				sample = bbe::Vector2(leftMult * val, rightMult * val);
			}
			else if (channels == 2)
			{
				if (posAvailable)
				{
					// A position for a multi channel sound is currently unsupported.
					throw bbe::IllegalStateException();
				}
				sample = bbe::Vector2(
					m_psound->getSample(m_samples_loaded, 0), 
					m_psound->getSample(m_samples_loaded, 1));
			}
			else
			{
				throw bbe::IllegalStateException();
			}

			if (stopRequested)
			{
				sample *= (1.0f - percentage);
			}
			samples[i] += sample * m_volume;

			m_samples_loaded++;
		}

		if (stopRequested)
		{
			readyForDelete = true;
		}
		posPreviousLoad = pos;
	}

	bool isPlaying() const
	{
		return m_samples_loaded < m_psound->getAmountOfSamples();
	}
};
static std::mutex playingSoundsMutex;
static std::map<uint64_t, SoundInstanceData> playingSounds;

static void destroySoundSystem()
{
	if (mainSource)
	{
		alDeleteSources(1, &mainSource);
		mainSource = 0;
	}
	for (ALuint buffer : buffers)
	{
		freeBuffer(buffer);
	}
	buffers.clear();
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

static ALint freeUsedUpBuffers()
{
	ALint usedUpBuffers = 0;
	alGetSourcei(mainSource, AL_BUFFERS_PROCESSED, &usedUpBuffers);

	for (ALint i = 0; i < usedUpBuffers; i++)
	{
		ALuint buffer = buffers[0];
		buffers.removeIndex(0);
		alSourceUnqueueBuffers(mainSource, 1, &buffer);
		freeBuffer(buffer);
	}
	return usedUpBuffers;
}

static void loadAllBuffers()
{
	bbe::Array<bbe::Vector2, 1000> samples;

	for (auto it = playingSounds.begin(); it != playingSounds.end(); ++it)
	{
		SoundInstanceData& sid = it->second;
		sid.loadBuffer(samples.getRaw(), samples.getLength());
		if (sid.m_psound->getHz() != 44100)
		{
			bbe::String errorMsg = "";
			errorMsg += sid.m_psound->getHz();
			errorMsg += " Hz not supported";
			throw bbe::IllegalStateException(errorMsg.getRaw());
		}
	}

	ALuint buffer = getNewBuffer();
	alBufferData(buffer, AL_FORMAT_STEREO_FLOAT32, samples.getRaw(), sizeof(bbe::Vector2) * samples.getLength(), 44100); // TODO HZ
	ALenum err = alGetError();
	if (err != ALC_NO_ERROR)
	{
		std::cout << "Something went wrong when loading buffer contents! " << err << std::endl;
		freeBuffer(buffer);
		throw bbe::IllegalStateException();
	}
	buffers.add(buffer);

	alSourceQueueBuffers(mainSource, 1, &buffer);

	listenerPosPreviousBufferCalculation = listenerPos;
	listenerDirectionPreviousBufferCalculation = listenerDirection;
}

static void refreshBuffers()
{
	ALint usedBuffers = freeUsedUpBuffers();
	for (ALint i = 0; i < usedBuffers; i++)
	{
		loadAllBuffers();
	}
	if (usedBuffers)
	{
		ALenum state;
		alGetSourcei(mainSource, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING)
		{
			// TODO: Is there a way to do this automatically?
			static uint32_t totalRestarts = 0;
			totalRestarts++;
			std::cout << "Sound died. Restarting. Total Restarts: " << totalRestarts << " Total sounds: " << playingSounds.size() << std::endl;
			previouslyDied = true;
			alSourcePlay(mainSource);
		}
	}
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

	alGenSources(1, &mainSource);
	{
		std::lock_guard<std::mutex> playingSoundsGuard(playingSoundsMutex);
		for(size_t i = 0; i<2; i++) loadAllBuffers();
	}
	alSourcePlay(mainSource);
	ALenum err = alGetError();
	if (err != ALC_NO_ERROR)
	{
		std::cout << "Something went wrong when creating a ALSource! " << err << std::endl;
		return false;
	}

	return true;
}

static void updateSoundSystem()
{
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

			playingSounds.insert({ pr.index, sid });
		}
		playRequests.clear();
	}

	for (auto it = playingSounds.begin(); it != playingSounds.end(); /*no inc*/)
	{
		if (!it->second.isPlaying() || it->second.readyForDelete)
		{
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
				it->second.stopRequested = true;
			}
		}
		stopRequests.clear();
	}

	std::lock_guard<std::mutex> guard(listenerMutex);
	refreshBuffers();
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
		if (!previouslyDied)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); // A more relaxed version of yield...
		}
		else
		{
			std::this_thread::yield();
		}
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
	{
		std::lock_guard<std::mutex> guard(stopRequestsMutex);
		if (stopRequests.contains(index)) return false;
	}
	{
		std::lock_guard<std::mutex> guard(playRequestsMutex);
		for (size_t i = 0; i < playRequests.getLength(); i++)
		{
			if (playRequests[i].index == index) return true;
		}
	}
	// TODO: Aquiring this lock is potentially quite expensive, because the sound thread is running constantly
	std::lock_guard<std::mutex> guard(playingSoundsMutex);
	auto it = playingSounds.find(index);
	if (it != playingSounds.end())
	{
		SoundInstanceData& sid = it->second;
		return sid.isPlaying() && !sid.stopRequested && !sid.readyForDelete;
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

size_t bbe::INTERNAL::SoundManager::getAmountOfPlayingSounds() const
{
	std::lock_guard<std::mutex> playingSoundsGuard(playingSoundsMutex);
	return playingSounds.size();
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

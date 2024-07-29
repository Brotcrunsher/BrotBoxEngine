#ifndef BBE_NO_AUDIO

#include "BBE/SoundManager.h"
#include "BBE/Error.h"
#include "BBE/Logging.h"
#include "BBE/WriterReaderBuffer.h"
#include <algorithm>
#include <iostream>
#include <mutex>
#include <chrono>

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#ifdef WIN32
#include <Windows.h>
#include <mmdeviceapi.h>
#endif

namespace bbe
{
	class SoundDataSource;
}

// No mutexes:
using BufferContents = bbe::Array<bbe::Vector2, 1024>;

static bbe::List<ALuint> staticSources;

static bbe::WriterReaderBuffer<uint64_t, 1024> stopRequests;
static auto stopRequestsReader = stopRequests.reader();

static bbe::WriterReaderBuffer<uint64_t, 1024> removedIds;
static auto removedIdsReader = removedIds.reader();

static ALuint restartCycle = 0;

struct ListenerData
{
	bbe::Vector3 pos = bbe::Vector3(0, 0, 0);
	bbe::Vector3 dir = bbe::Vector3(0, 0, 1);
};
static bbe::WriterReaderBuffer<ListenerData, 1024> newListenerData;
static auto newListenerDataReader = newListenerData.reader();
static ListenerData listener;
static ListenerData listenerPrevious;
static std::atomic_bool restartRequest = false;

struct SetPositionRequest
{
	uint64_t index = 0;
	bbe::Vector3 pos;
};
static bbe::WriterReaderBuffer<SetPositionRequest, 1024> setPositionRequests;
static auto setPositionRequestsReader = setPositionRequests.reader();

struct PlayRequest
{
	uint64_t index = 0;
	const bbe::SoundDataSource* sound = nullptr;
	bool posAvailable = false;
	bbe::Vector3 pos;
	float volume = 0.0f;
};
static bbe::WriterReaderBuffer<PlayRequest, 1024> playRequests;
static auto playRequestsReader = playRequests.reader();

static ALuint mainSource = 0;
ALCdevice* device = nullptr;
static bool previouslyDied = false;

namespace eh /* = error handled */
{
	void buildErrorString(bbe::String& s)
	{
		// Do nothing
	}

	template <typename T, typename... Types>
	void buildErrorString(bbe::String& s, T t1, Types... ts)
	{
		s += t1;
		s += " ";
		buildErrorString(s, ts...);
	}

	template <typename... Types>
	void handleErrors(const char* funcName, Types... ts)
	{
		ALenum err = ::alGetError();
		if (err != AL_NO_ERROR)
		{
			bbe::String msg = "SoundManager crashed. Function: ";
			msg += funcName;
			msg += "Variables: ";
			buildErrorString(msg, ts...);
			bbe::Crash(bbe::Error::IllegalState, msg.getRaw());
		}
	}

	void alGenBuffers(ALsizei n, ALuint* buffers)
	{
		::alGenBuffers(n, buffers);
		handleErrors("alGenBuffers", n, (size_t)buffers);
	}

	void alDeleteBuffers(ALsizei n, const ALuint* buffers)
	{
		::alDeleteBuffers(n, buffers);
		handleErrors("alDeleteBuffers", n, (size_t)buffers);
	}

	void alGenSources(ALsizei n, ALuint* sources)
	{
		::alGenSources(n, sources);
		handleErrors("alGenSources", n, (size_t)sources);
	}

	void alDeleteSources(ALsizei n, const ALuint* sources)
	{
		::alDeleteSources(n, sources);
		handleErrors("alDeleteSources", n, (size_t)sources);
	}

	void alGetSourcei(ALuint source, ALenum param, ALint* value)
	{
		::alGetSourcei(source, param, value);
		handleErrors("alGetSourcei", source, param, (size_t)value);
	}

	void alSourceUnqueueBuffers(ALuint src, ALsizei nb, ALuint* buffers)
	{
		::alSourceUnqueueBuffers(src, nb, buffers);
		handleErrors("alSourceUnqueueBuffers", src, nb, (size_t)buffers);
	}

	void alBufferData(ALuint buffer, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq)
	{
		::alBufferData(buffer, format, data, size, freq);
		handleErrors("alBufferData", buffer, format, (size_t)data, size, freq);
	}

	void alSourceQueueBuffers(ALuint src, ALsizei nb, const ALuint* buffers)
	{
		::alSourceQueueBuffers(src, nb, buffers);
		handleErrors("alSourceQueueBuffers", src, nb, (size_t)buffers);
	}

	void alSourcePlay(ALuint source)
	{
		::alSourcePlay(source);
		handleErrors("alSourcePlay", source);
	}
}

static ALuint getNewBuffer()
{
	ALuint buffer = 0;
	eh::alGenBuffers(1, &buffer);
	return buffer;
}

static void freeBuffer(ALuint buffer)
{
	eh::alDeleteBuffers(1, &buffer);
}

static ALuint getNewStaticSource()
{
	ALuint source = 0;
	eh::alGenSources(1, &source);
	return source;
}

static void freeStaticSource(ALuint source)
{
	eh::alDeleteSources(1, &source);
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
	ALuint source = 0;

	void loadDynamicBuffer(const bbe::SoundDataSourceDynamic* sdsd, bbe::Vector2* samples, size_t numSamples)
	{
		const uint32_t channels = sdsd->getAmountOfChannels();
		for (size_t i = 0; i < numSamples; i++)
		{
			bbe::Vector2 sample;
			float percentage = (float)i / (float)numSamples;

			if (channels == 1)
			{
				float leftMult = 1.0f;
				float rightMult = 1.0f;

				if (posAvailable)
				{
					bbe::Vector3 lerpedPos = bbe::Math::interpolateLinear(posPreviousLoad, pos, percentage);
					bbe::Vector3 lerpedListenerPos = bbe::Math::interpolateLinear(listenerPrevious.pos, listener.pos, percentage);

					bbe::Vector3 toListener = lerpedListenerPos - lerpedPos;
					bbe::Vector2 toListener2d = bbe::Vector2(toListener.x, toListener.y);
					bbe::Vector2 toListener2dNorm = toListener2d.normalize();

					// TODO: Slerp instead?
					bbe::Vector3 lerpedDirection = bbe::Math::interpolateLinear(listenerPrevious.dir, listener.dir, percentage).normalize();

					bbe::Vector2 listenerRight = bbe::Vector2(-lerpedDirection.y, lerpedDirection.x).normalize();

					//TODO: This isn't quite right. When looking directly at a sound, it still is coming more from one side than the other
					rightMult = toListener2dNorm * listenerRight;
					leftMult = 1.0f - rightMult;

					float distSq = toListener.getLengthSq();
					if (distSq < 1.0f) distSq = 1.0f;
					rightMult /= distSq;
					leftMult /= distSq;
				}

				float val = sdsd->getSample(m_samples_loaded, 0);
				sample = bbe::Vector2(leftMult * val, rightMult * val);
			}
			else if (channels == 2)
			{
				if (posAvailable)
				{
					// A position for a multi channel sound is currently unsupported.
					bbe::Crash(bbe::Error::IllegalState);
				}
				sample = bbe::Vector2(
					sdsd->getSample(m_samples_loaded, 0),
					sdsd->getSample(m_samples_loaded, 1));
			}
			else
			{
				bbe::Crash(bbe::Error::IllegalState);
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
		if (const bbe::SoundDataSourceDynamic* sdsd = dynamic_cast<const bbe::SoundDataSourceDynamic*>(m_psound))
		{
			return true;
		}
		else if (const bbe::SoundDataSourceStatic* sdss = dynamic_cast<const bbe::SoundDataSourceStatic*>(m_psound))
		{
			ALint state = 0;
			eh::alGetSourcei(source, AL_SOURCE_STATE, &state);
			return state == AL_PLAYING;
		}
		else
		{
			bbe::Crash(bbe::Error::IllegalState);
		}
	}
};
static std::map<uint64_t, SoundInstanceData> playingSounds;

static void destroySoundSystem()
{
	if (mainSource)
	{
		eh::alDeleteSources(1, &mainSource);
		mainSource = 0;
	}
	for (auto sound : playingSounds)
	{
		removedIds.add(sound.first);
	}
	playingSounds.clear();
	for (ALuint source : staticSources)
	{
		freeStaticSource(source);
	}
	staticSources.clear();
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
	eh::alGetSourcei(mainSource, AL_BUFFERS_PROCESSED, &usedUpBuffers);
	static bbe::List<ALuint> buffersBuffer;
	if (usedUpBuffers > buffersBuffer.getLength())
	{
		buffersBuffer.resizeCapacityAndLength(usedUpBuffers);
	}
	eh::alSourceUnqueueBuffers(mainSource, usedUpBuffers, buffersBuffer.getRaw());

	for (ALint i = 0; i < usedUpBuffers; i++)
	{
		ALuint buffer = buffersBuffer[i];
		freeBuffer(buffer);
	}
	return usedUpBuffers;
}

static void loadAllBuffers()
{
	BufferContents samples;

	for (auto it = playingSounds.begin(); it != playingSounds.end(); ++it)
	{
		SoundInstanceData& sid = it->second;
		if (const bbe::SoundDataSourceDynamic* sdsd = dynamic_cast<const bbe::SoundDataSourceDynamic*>(sid.m_psound))
		{
			sid.loadDynamicBuffer(sdsd, samples.getRaw(), samples.getLength());
			if (sid.m_psound->getHz() != 44100)
			{
				bbe::String errorMsg = "";
				errorMsg += sid.m_psound->getHz();
				errorMsg += " Hz not supported";
				bbe::Crash(bbe::Error::IllegalState, errorMsg.getRaw());
			}
		}
	}

	ALuint buffer = getNewBuffer();
	eh::alBufferData(buffer, AL_FORMAT_STEREO_FLOAT32, samples.getRaw(), sizeof(bbe::Vector2) * samples.getLength(), 44100); // TODO HZ
	eh::alSourceQueueBuffers(mainSource, 1, &buffer);

	listenerPrevious = listener;
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
		eh::alGetSourcei(mainSource, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING)
		{
			// TODO: Is there a way to do this automatically?
			static uint32_t totalRestarts = 0;
			totalRestarts++;
			BBELOGLN("Sound died. Restarting. Total Restarts: " << totalRestarts << " Total sounds: " << playingSounds.size());
			previouslyDied = true;
			eh::alSourcePlay(mainSource);
		}
	}
}

static bool initSoundSystem()
{
	restartCycle++;
	device = alcOpenDevice(nullptr);
	if (!device)
	{
		BBELOGLN("Could not init Sound Manager! Device was null.");
		return false;
	}
	ALCcontext* context = alcCreateContext(device, nullptr);
	if (!context || !alcMakeContextCurrent(context))
	{
		if (context) alcDestroyContext(context);
		alcCloseDevice(device);
		BBELOGLN("Could not init Sound Manager! Failed to create context.");
		return false;
	}

	if (!alIsExtensionPresent("AL_EXT_float32"))
	{
		BBELOGLN("Could not init Sound Manager! AL_EXT_float32 not present.");
		destroySoundSystem();
		return false;
	}

	eh::alGenSources(1, &mainSource);
	{
		for(size_t i = 0; i<10; i++) loadAllBuffers();
	}
	eh::alSourcePlay(mainSource);
	ALenum err = alGetError();
	if (err != ALC_NO_ERROR)
	{
		BBELOGLN("Something went wrong when creating a ALSource! " << err);
		return false;
	}

	return true;
}

static void updateSoundSystem()
{
	{
		while(setPositionRequestsReader.hasNext())
		{
			const SetPositionRequest& spr = setPositionRequestsReader.next();
			auto it = playingSounds.find(spr.index);
			if (it != playingSounds.end())
			{
				it->second.pos = spr.pos;
			}
		}
	}

	{
		while(playRequestsReader.hasNext())
		{
			const PlayRequest& pr = playRequestsReader.next();
			SoundInstanceData sid;
			sid.m_psound = pr.sound;
			sid.m_volume = pr.volume;
			sid.pos = pr.pos;
			sid.posAvailable = pr.posAvailable;

			if (const bbe::SoundDataSourceStatic* SDSS = dynamic_cast<const bbe::SoundDataSourceStatic*>(pr.sound))
			{
				if (!SDSS->INTERNAL_buffer || SDSS->INTERNAL_restartCycle != restartCycle)
				{
					eh::alGenBuffers(1, &SDSS->INTERNAL_buffer);
					const bbe::List<float>* samples = SDSS->getRaw();
					ALenum format;
					switch (SDSS->getAmountOfChannels())
					{
					case 1:
						format = AL_FORMAT_MONO_FLOAT32;
						break;
					case 2:
						format = AL_FORMAT_STEREO_FLOAT32;
						break;
					default:
						bbe::Crash(bbe::Error::IllegalState);
					}
					eh::alBufferData(SDSS->INTERNAL_buffer, AL_FORMAT_STEREO_FLOAT32, samples->getRaw(), sizeof(float) * samples->getLength(), SDSS->getHz());
					SDSS->INTERNAL_restartCycle = restartCycle;
				}

				ALuint source = getNewStaticSource();
				staticSources.add(source);
				eh::alSourceQueueBuffers(source, 1, &SDSS->INTERNAL_buffer);
				eh::alSourcePlay(source);
				sid.source = source;
			}

			playingSounds.insert({ pr.index, sid });
		}
	}

	for (auto it = playingSounds.begin(); it != playingSounds.end(); /*no inc*/)
	{
		if (!it->second.isPlaying() || it->second.readyForDelete)
		{
			removedIds.add(it->first);
			playingSounds.erase(it++);
		}
		else
		{
			++it;
		}
	}

	{
		while (stopRequestsReader.hasNext())
		{
			const int64_t& index = stopRequestsReader.next();
			auto it = playingSounds.find(index);
			if (it != playingSounds.end())
			{
				it->second.stopRequested = true;
			}
		}
	}

	for (size_t i = 0; i < staticSources.getLength(); i++)
	{
		ALint state = 0;
		ALenum err = alGetError();
		if (err != AL_NO_ERROR)
		{
			bbe::Crash(bbe::Error::IllegalState, "Some uncaught AL error happened.");
		}
		alGetSourcei(staticSources[i], AL_SOURCE_STATE, &state);
		err = alGetError();
		if (err == AL_INVALID_NAME)
		{
			// TODO: This shouldn't happen! Unfortunately it does though. Fix!
			BBELOGLN("Warning: Invalid AL Source.");
			state = AL_INITIAL;
		}
		if (state != AL_PLAYING)
		{
			freeStaticSource(staticSources[i]);
			staticSources.removeIndex(i);
			break;
		}
	}

	while (newListenerDataReader.hasNext())
	{
		listener = newListenerDataReader.next();
	}
	refreshBuffers();
}

static std::atomic<bool> endRequested = false;
static void requestEnd()
{
	endRequested = true;
}
static bool isEndRequested()
{
	return endRequested;
}


static void innerSoundSystemMain()
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

		if (restartRequest)
		{
			restartRequest = false;
			destroySoundSystem();
			initSoundSystem();
		}
	}
	destroySoundSystem();
}

static void soundSystemMain()
{
	BBE_TRY_RELEASE
	{
		innerSoundSystemMain();
	}
	BBE_CATCH_RELEASE
	{
		bbe::Crash(bbe::Error::UnhandledException, "Sound Thread");
	}
}

static bbe::INTERNAL::SoundManager* m_pinstance = nullptr;

// ***************************************************************************************************************
// ***                                                                                                         ***
// ***    CAREFUL: Below this line, no al functions must be called directly, as we are not on the al thread!   ***
// ***                                                                                                         ***
// ***************************************************************************************************************

#ifdef _WIN32
std::mutex restartTpMutex;
std::chrono::time_point<std::chrono::steady_clock> restartTp;
bool armed = false;
void scheduleRestart()
{
	std::lock_guard lg(restartTpMutex);
	restartTp = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	armed = true;
}

bool testScheduledRestart()
{
	std::lock_guard lg(restartTpMutex);
	if (armed && std::chrono::steady_clock::now() > restartTp)
	{
		armed = false;
		return true;
	}
	return false;
}


class CMMNotificationClient : public IMMNotificationClient
{
private:
	LONG refCount = 1;

public:
	// IUnknown methods -- AddRef, Release, and QueryInterface
	ULONG STDMETHODCALLTYPE AddRef()
	{
		return InterlockedIncrement(&refCount);
	}
	ULONG STDMETHODCALLTYPE Release()
	{
		ULONG ulRef = InterlockedDecrement(&refCount);
		if (0 == ulRef)
		{
			delete this;
		}
		return ulRef;
	}
	HRESULT STDMETHODCALLTYPE QueryInterface(
		REFIID riid, VOID** ppvInterface)
	{
		return E_NOINTERFACE;
	}

	HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
		EDataFlow flow, ERole role,
		LPCWSTR pwstrDeviceId)
	{
		scheduleRestart();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId)
	{
		scheduleRestart();
		return S_OK;
	};

	HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId)
	{
		scheduleRestart();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(
		LPCWSTR pwstrDeviceId,
		DWORD dwNewState)
	{
		scheduleRestart();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(
		LPCWSTR pwstrDeviceId,
		const PROPERTYKEY key)
	{
		scheduleRestart();
		return S_OK;
	}
};
#endif

static bbe::List<uint64_t> playingIds;

bbe::INTERNAL::SoundManager* bbe::INTERNAL::SoundManager::getInstance()
{
	return m_pinstance;
}

bbe::INTERNAL::SoundManager::SoundManager()
{
	if (m_pinstance != nullptr)
	{
		bbe::Crash(bbe::Error::AlreadyCreated);
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
	stopRequests.add(index);
	playingIds.removeSingle(index);
}

bool bbe::INTERNAL::SoundManager::isSoundWithIndexPlaying(uint64_t index)
{
	return playingIds.contains(index);
}

void bbe::INTERNAL::SoundManager::setPosition(uint64_t index, const bbe::Vector3& pos)
{
	setPositionRequests.add(SetPositionRequest{
		index,
		pos
		});
}

void bbe::INTERNAL::SoundManager::setSoundListener(const bbe::Vector3& pos, const bbe::Vector3& lookDirection)
{
	newListenerData.add({ pos, lookDirection });
}

size_t bbe::INTERNAL::SoundManager::getAmountOfPlayingSounds() const
{
	return playingIds.getLength();
}

void bbe::INTERNAL::SoundManager::update()
{
#ifdef __EMSCRIPTEN__
	updateSoundSystem();
#else
	// Do nothing, the thread updates itself.
#endif
	while (removedIdsReader.hasNext())
	{
		playingIds.removeSingle(removedIdsReader.next());
	}
#ifdef _WIN32
	if (testScheduledRestart())
	{
		restart();
	}
#endif
}

void bbe::INTERNAL::SoundManager::init()
{
#ifdef __EMSCRIPTEN__
	initSoundSystem();
#else
	soundSystemThread = std::thread(soundSystemMain);
#ifdef WIN32
	BOOL prioSuccess = ::SetThreadPriority(soundSystemThread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
	if (!prioSuccess)
	{
		debugBreak();
	}

	IMMDeviceEnumerator* _pEnumerator;
	HRESULT hr = S_OK;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
		NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IMMDeviceEnumerator),
		(void**)&_pEnumerator);
	hr = _pEnumerator->RegisterEndpointNotificationCallback(new CMMNotificationClient());

	_pEnumerator->Release();
#endif
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

void bbe::INTERNAL::SoundManager::restart()
{
	restartRequest = true;
}

bbe::SoundInstance bbe::INTERNAL::SoundManager::play(const SoundDataSource& sound, const bbe::Vector3* pos, float volume)
{
	uint64_t index = getNextIndex();

	playRequests.add(PlayRequest{
		index,
		&sound,
		pos != nullptr,
		pos != nullptr ? *pos : bbe::Vector3(),
		volume
		});

	playingIds.add(index);

	return SoundInstance(index);
}

uint64_t bbe::INTERNAL::SoundManager::getNextIndex()
{
	const uint64_t retVal = nextIndex;
	nextIndex++;
	return retVal;
}

#endif

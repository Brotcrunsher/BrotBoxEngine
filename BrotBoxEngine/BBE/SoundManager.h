#pragma once

#ifndef BBE_NO_AUDIO

#include <cstdint>
#include <map>
#include <mutex>
#include "../BBE/Sound.h"
#include "../BBE/SoundInstance.h"
#include "AL/al.h"

struct PaStreamCallbackTimeInfo;
typedef unsigned long PaStreamCallbackFlags;

namespace bbe
{
	namespace INTERNAL
	{
		struct SoundInstanceData
		{
			uint64_t m_samples_loaded = 0;
			const SoundDataSource* m_psound = nullptr;
			float m_volume = 0;
			ALuint source = 0;
			bbe::List<ALuint> buffers;

			bool areAllSamplesLoaded() const;
			bool isBufferLoadingRequired() const;
			void loadNewBuffer(ALuint buffer, SoundManager* sm);
			void start(ALuint buffer, SoundManager* sm);
			void freeUsedUpBuffers(SoundManager* sm);
			void destroy(SoundManager* sm);
		};

		class SoundManager
		{
		private:
			static SoundManager* m_pinstance;
			uint64_t nextIndex = 1;
			uint64_t getNextIndex();

			std::mutex playingSoundsChangeMutex;
			std::map<uint64_t, SoundInstanceData> playingSounds;
			bool initSuccessful = false;

			bbe::List<ALuint> unusedBuffers;

		public:
			static SoundManager* getInstance();

			SoundManager();
			~SoundManager();

			SoundManager           (const SoundManager& ) = delete;
			SoundManager           (      SoundManager&&) = delete;
			SoundManager& operator=(const SoundManager& ) = delete;
			SoundManager& operator=(      SoundManager&&) = delete;

			void update();
			void init();
			void destroy();
			SoundInstance play(const SoundDataSource& sound, float volume);
			int soundCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

			void stopSoundWithIndex(uint64_t index);
			bool isSoundWithIndexPlaying(uint64_t index);

			ALuint getNewBuffer();
			void freeBuffer(ALuint buffer);
		};
	}
}

#endif

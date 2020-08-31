#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include "../BBE/Sound.h"
#include "../BBE/SoundInstance.h"

class PaStreamCallbackTimeInfo;
typedef unsigned long PaStreamCallbackFlags;

namespace bbe
{
	namespace INTERNAL
	{
		struct SoundInstanceData
		{
			uint64_t m_sample = 0;
			const Sound* m_psound = nullptr;
			bool m_markedForDeletion = false;
			float m_volume = 0;
		};

		class SoundManager
		{
		private:
			static SoundManager* m_pinstance;
			uint64_t nextIndex = 1;
			uint64_t getNextIndex();

			std::mutex playingSoundsChangeMutex;
			std::map<uint64_t, SoundInstanceData> playingSounds;

		public:
			static SoundManager* getInstance();

			SoundManager();
			~SoundManager();

			SoundManager           (const SoundManager& ) = delete;
			SoundManager           (      SoundManager&&) = delete;
			SoundManager& operator=(const SoundManager& ) = delete;
			SoundManager& operator=(      SoundManager&&) = delete;

			void init();
			void destroy();
			SoundInstance play(const Sound& sound, float volume);
			int soundCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

			void stopSoundWithIndex(uint64_t index);
			bool isSoundWithIndexPlaying(uint64_t index);
		};
	}
}

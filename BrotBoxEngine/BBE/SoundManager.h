#pragma once

#ifndef BBE_NO_AUDIO

#include <cstdint>
#include <map>
#include <thread>
#include "../BBE/Sound.h"
#include "../BBE/SoundInstance.h"
#include "../BBE/Vector3.h"

namespace bbe
{
	namespace INTERNAL
	{
		class SoundManager
		{
		private:
#ifndef __EMSCRIPTEN__
			std::thread soundSystemThread;
#endif
			uint64_t nextIndex = 1;
			uint64_t getNextIndex();

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
			SoundInstance play(const SoundDataSource& sound, const bbe::Vector3* pos, float volume);

			void stopSoundWithIndex(uint64_t index);
			bool isSoundWithIndexPlaying(uint64_t index);
			void setPosition(uint64_t index, const bbe::Vector3& pos);

			void setSoundListener(const bbe::Vector3& pos, const bbe::Vector3& lookDirection);
		};
	}
}

#endif

#pragma once

#ifndef BBE_NO_AUDIO

#include <cstdint>

namespace bbe
{
	class Sound;
	namespace INTERNAL
	{
		class SoundManager;
	}

	class SoundInstance
	{
		friend class Sound;
		friend class bbe::INTERNAL::SoundManager;
	private:
		uint64_t m_index = 0;

		SoundInstance(uint64_t index);

	public:
		SoundInstance() = default;
		SoundInstance           (const SoundInstance& ) = default;
		SoundInstance           (      SoundInstance&&) = default;
		SoundInstance& operator=(const SoundInstance& ) = default;
		SoundInstance& operator=(      SoundInstance&&) = default;

		void stop();

		bool isPlaying();
	};
}

#endif

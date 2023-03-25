#pragma once

#ifndef BBE_NO_AUDIO

#include <cstdint>
#include "../BBE/Vector3.h"

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

		void setPosition(const bbe::Vector3& pos);

		bool isPlaying();
	};
}

#endif

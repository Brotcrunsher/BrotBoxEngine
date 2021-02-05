#pragma once

#ifndef BBE_NO_AUDIO

#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/SoundInstance.h"

namespace bbe
{
	enum class SoundLoadFormat
	{
		AUTOMATIC = 0,
		MP3 = 1,
	};

	class Sound
	{
	private:
		bool             m_loaded   = false;
		bbe::List<float> m_data     = {};
		bool             m_looped   = false;
		uint32_t         m_channels = 0;

		void loadMp3(const bbe::String& path);

	public:
		Sound();
		Sound(const bbe::String& path, SoundLoadFormat soundLoadFormat = SoundLoadFormat::AUTOMATIC);

		void load(const bbe::String& path, SoundLoadFormat soundLoadFormat = SoundLoadFormat::AUTOMATIC);

		bool isLoaded() const;
		bool isLooped() const;
		uint32_t getChannels() const;
		std::pair<float, float> getSample(size_t i) const;
		size_t getAmountOfSamples() const;

		void setLooped(bool looped);

		SoundInstance play(float volume = 1);
	};
}

#endif

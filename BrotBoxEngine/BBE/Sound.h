#pragma once

#ifndef BBE_NO_AUDIO

#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/SoundInstance.h"
#include "../BBE/Vector2.h"

namespace bbe
{
	enum class SoundLoadFormat
	{
		AUTOMATIC = 0,
		MP3 = 1,
	};

	class SoundDataSource
	{
	private:
		bool             m_looped   = false;
	public:
		virtual bbe::Vector2 getSample(size_t i) const = 0;
		virtual size_t getAmountOfSamples() const = 0;
		virtual uint32_t getHz() const = 0;

		bool isLooped() const;
		void setLooped(bool looped);
		
		SoundInstance play(float volume = 1) const;
	};

	class Sound : 
		public SoundDataSource
	{
	private:
		bool             m_loaded   = false;
		bbe::List<float> m_data     = {};
		uint32_t         m_channels = 0;
		uint32_t         m_hz       = 0;

		void loadMp3(const bbe::List<unsigned char>& data);

	public:
		Sound();
		Sound(const bbe::String& path, SoundLoadFormat soundLoadFormat = SoundLoadFormat::AUTOMATIC);

		void load(const bbe::String& path, SoundLoadFormat soundLoadFormat = SoundLoadFormat::AUTOMATIC);
		void load(const bbe::List<unsigned char> &data, SoundLoadFormat soundLoadFormat = SoundLoadFormat::AUTOMATIC);

		bool isLoaded() const;
		uint32_t getChannels() const;
		virtual bbe::Vector2 getSample(size_t i) const override;
		virtual size_t getAmountOfSamples() const override;
		virtual uint32_t getHz() const override;
	};
}

#endif

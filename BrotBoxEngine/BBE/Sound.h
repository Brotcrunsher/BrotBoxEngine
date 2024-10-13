#pragma once

#ifndef BBE_NO_AUDIO

#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/SoundInstance.h"
#include "../BBE/Vector2.h"

#include "AL/al.h"

namespace bbe
{
	enum class SoundLoadFormat
	{
		AUTOMATIC = 0,
		MP3 = 1,
		RAW_MONO_FLOAT_44100 = 2,
	};

	class SoundDataSource
	{
	public:
		virtual uint32_t getAmountOfChannels() const = 0;
		virtual uint32_t getHz() const = 0;

		SoundInstance play(float volume = 1) const;
		SoundInstance play(const bbe::Vector3& pos, float volume = 1) const;
	};

	class SoundDataSourceDynamic : public SoundDataSource
	{
	public:
		virtual float getSample(size_t i, uint32_t channel) const = 0;
	};

	class SoundDataSourceStatic : public SoundDataSource
	{
	private:
		bool             m_looped = false;

	public:
		bool isLooped() const;
		void setLooped(bool looped);

		virtual const bbe::List<float>* getRaw() const = 0;

		mutable ALuint INTERNAL_buffer = 0;
		mutable ALuint INTERNAL_restartCycle = 0;
	};

	class Sound : 
		public SoundDataSourceStatic
	{
	private:
		bool             m_loaded   = false;
		bbe::List<float> m_data     = {};
		uint32_t         m_channels = 0;
		uint32_t         m_hz       = 0;

		void loadMp3(const bbe::ByteBuffer& data);
		void loadRawMonoFloat44100(const bbe::ByteBuffer& data);

	public:
		Sound();
		explicit Sound(const bbe::String& path, SoundLoadFormat soundLoadFormat = SoundLoadFormat::AUTOMATIC);

		void load(const bbe::String& path, SoundLoadFormat soundLoadFormat = SoundLoadFormat::AUTOMATIC);
		void load(const bbe::ByteBuffer &data, SoundLoadFormat soundLoadFormat = SoundLoadFormat::AUTOMATIC);
		void load(const bbe::List<char>& data, SoundLoadFormat soundLoadFormat = SoundLoadFormat::AUTOMATIC);
		void load(const bbe::List<float>& data, SoundLoadFormat soundLoadFormat = SoundLoadFormat::RAW_MONO_FLOAT_44100);

		bool isLoaded() const;
		uint32_t getChannels() const;
		virtual uint32_t getAmountOfChannels() const override;
		virtual uint32_t getHz() const override;

		const bbe::List<float>* getRaw() const override;
	};
}

#endif

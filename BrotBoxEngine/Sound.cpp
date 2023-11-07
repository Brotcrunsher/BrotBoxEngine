#include "BBE/Sound.h"
#include "BBE/SoundManager.h"
#include "BBE/Exceptions.h"
#include "BBE/SimpleFile.h"
#define MINIMP3_IMPLEMENTATION
#ifndef BBE_NO_AUDIO

#include "minimp3_ex.h"

void bbe::Sound::loadMp3(const bbe::ByteBuffer& data)
{
	mp3dec_t mp3d = {};
	mp3dec_file_info_t info = {};
	const int err = mp3dec_load_buf(&mp3d, data.getRaw(), data.getLength(), &info, NULL, NULL);
	if (err)
	{
		free(info.buffer);
		switch (err)
		{
		case MP3D_E_PARAM:
			throw IllegalArgumentException();
		case MP3D_E_IOERROR:
			throw LoadException();
		case MP3D_E_MEMORY:
			throw OutOfMemoryException();
		case MP3D_E_DECODE:
			throw DecodeException();
		default:
			throw UnknownException();
		}
	}
	else
	{
		if (info.channels > 2)
		{
			// Currently only up to 2 channels are supported.
			free(info.buffer);
			throw IllegalArgumentException();
		}

		m_hz = info.hz;

		m_data.resizeCapacityAndLength(info.samples);
		for (size_t i = 0; i < info.samples; i++)
		{
			m_data[i] = (float)info.buffer[i] / (float)0x7FFF; // Mini MP3 stores the sound data as int16_t, but we want it as float in range [-1, +1]
		}
		m_channels = info.channels;
	}
	free(info.buffer);
}

bbe::Sound::Sound()
{
	// do nothing
}

bbe::Sound::Sound(const bbe::String& path, SoundLoadFormat soundLoadFormat)
{
	load(path, soundLoadFormat);
}

void bbe::Sound::load(const bbe::String& path, SoundLoadFormat soundLoadFormat)
{
	bbe::ByteBuffer data = bbe::simpleFile::readBinaryFile(path);
	load(data, soundLoadFormat);
}

void bbe::Sound::load(const bbe::ByteBuffer& data, SoundLoadFormat soundLoadFormat)
{
	if (isLoaded())
	{
		throw AlreadyCreatedException();
	}

	if (soundLoadFormat == SoundLoadFormat::AUTOMATIC)
	{
		//TODO Once more formats are supported, add determination logic here.
		soundLoadFormat = SoundLoadFormat::MP3;
	}

	switch (soundLoadFormat)
	{
	case SoundLoadFormat::MP3:
		loadMp3(data);
		break;
	default:
		throw IllegalArgumentException();
	}

	m_loaded = true;
}

bool bbe::Sound::isLoaded() const
{
	return m_loaded;
}

bool bbe::SoundDataSource::isLooped() const
{
	return m_looped;
}

uint32_t bbe::Sound::getChannels() const
{
	if (!m_loaded) throw NotInitializedException();
	return m_channels;
}

float bbe::Sound::getSample(size_t i, uint32_t channel) const
{
	if (!m_loaded) throw NotInitializedException();
	if (channel >= m_channels) throw IllegalArgumentException();

	return m_data[i * m_channels + channel];
}

size_t bbe::Sound::getAmountOfSamples() const
{
	return m_data.getLength() / m_channels;
}

uint32_t bbe::Sound::getAmountOfChannels() const
{
	return m_channels;
}

uint32_t bbe::Sound::getHz() const
{
	return m_hz;
}

void bbe::SoundDataSource::setLooped(bool looped)
{
	m_looped = looped;
}

bbe::SoundInstance bbe::SoundDataSource::play(float volume) const
{
	return bbe::INTERNAL::SoundManager::getInstance()->play(*this, nullptr, volume);
}

bbe::SoundInstance bbe::SoundDataSource::play(const bbe::Vector3& pos, float volume) const
{
	return bbe::INTERNAL::SoundManager::getInstance()->play(*this, &pos, volume);
}

#endif

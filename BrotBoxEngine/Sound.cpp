#include "BBE/Sound.h"
#include "BBE/SoundManager.h"
#include "BBE/Exceptions.h"
#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"

void bbe::Sound::loadMp3(const bbe::String& path)
{
	mp3dec_t mp3d = {};
	mp3dec_file_info_t info = {};
	const int err = mp3dec_load(&mp3d, path.getRaw(), &info, NULL, NULL);
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
		if (info.hz != 44100)
		{
			// Currently only 44100 hz sounds are supported.
			free(info.buffer);
			throw IllegalArgumentException();
		}

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
		loadMp3(path);
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

bool bbe::Sound::isLooped() const
{
	return m_looped;
}

uint32_t bbe::Sound::getChannels() const
{
	if (!m_loaded) throw NotInitializedException();
	return m_channels;
}

std::pair<float, float> bbe::Sound::getSample(size_t i) const
{
	if (!m_loaded) throw NotInitializedException();

	if (m_channels == 1)
	{
		return std::pair<float, float>(m_data[i], m_data[i]);
	}
	else if (m_channels == 2)
	{
		return std::pair<float, float>(m_data[i * 2 + 0], m_data[i * 2 + 1]);
	}
	else
	{
		// Only 1 and 2 channels are supported
		throw IllegalStateException();
	}
}

size_t bbe::Sound::getAmountOfSamples() const
{
	return m_data.getLength() / m_channels;
}

void bbe::Sound::setLooped(bool looped)
{
	m_looped = looped;
}

bbe::SoundInstance bbe::Sound::play(float volume)
{
	return bbe::INTERNAL::SoundManager::getInstance()->play(*this, volume);
}

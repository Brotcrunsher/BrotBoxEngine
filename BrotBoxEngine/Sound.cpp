#include "BBE/Sound.h"
#include "BBE/SoundManager.h"
#include "BBE/Error.h"
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
			bbe::Crash(bbe::Error::IllegalArgument);
		case MP3D_E_IOERROR:
			bbe::Crash(bbe::Error::IllegalState);
		case MP3D_E_MEMORY:
			bbe::Crash(bbe::Error::OutOfMemory);
		case MP3D_E_DECODE:
			bbe::Crash(bbe::Error::Decode);
		default:
			bbe::Crash(bbe::Error::Unknown);
		}
	}
	else
	{
		if (info.channels > 2)
		{
			// Currently only up to 2 channels are supported.
			free(info.buffer);
			bbe::Crash(bbe::Error::IllegalArgument);
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

void bbe::Sound::loadRawMonoFloat44100(const bbe::ByteBuffer& data)
{
	if (data.getLength() % sizeof(float) != 0) bbe::Crash(bbe::Error::IllegalArgument, "Not a multiple of float!");

	m_data.resizeCapacityAndLengthUninit(data.getLength() / sizeof(float));
	memcpy(m_data.getRaw(), data.getRaw(), data.getLength());

	m_hz = 44100;
	m_channels = 1;
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
		bbe::Crash(bbe::Error::AlreadyCreated);
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
	case SoundLoadFormat::RAW_MONO_FLOAT_44100:
		loadRawMonoFloat44100(data);
		break;
	default:
		bbe::Crash(bbe::Error::IllegalArgument);
	}

	m_loaded = true;
}

void bbe::Sound::load(const bbe::List<char>& data, SoundLoadFormat soundLoadFormat)
{
	bbe::ByteBuffer buffer((bbe::byte*)data.getRaw(), data.getLength());
	load(buffer, soundLoadFormat);
}


void bbe::Sound::load(const bbe::List<float>& data, SoundLoadFormat soundLoadFormat)
{
	if (soundLoadFormat != SoundLoadFormat::RAW_MONO_FLOAT_44100) bbe::Crash(bbe::Error::IllegalArgument, "Only RAW_MONO_FLOAT_44100 currently supported!");
	bbe::ByteBuffer buffer((bbe::byte*)data.getRaw(), data.getLength() * sizeof(float));
	load(buffer, soundLoadFormat);
}

bool bbe::Sound::isLoaded() const
{
	return m_loaded;
}

bool bbe::SoundDataSourceStatic::isLooped() const
{
	return m_looped;
}

uint32_t bbe::Sound::getChannels() const
{
	if (!m_loaded) bbe::Crash(bbe::Error::NotInitialized);
	return m_channels;
}

uint32_t bbe::Sound::getAmountOfChannels() const
{
	return m_channels;
}

uint32_t bbe::Sound::getHz() const
{
	return m_hz;
}

const bbe::List<float>* bbe::Sound::getRaw() const
{
	return &m_data;
}

bbe::ByteBuffer bbe::Sound::toWav() const
{
	bbe::ByteBuffer buffer;

	int32_t byteRate = m_hz * 2 * m_channels;
	int32_t dataSize = m_data.getLength() * 2;
	
	// Write the header
	buffer.writeNullString("RIFF", false);
	int32_t chunkSize = 36 + dataSize;
	buffer.write(chunkSize);
	buffer.writeNullString("WAVE", false);
	buffer.writeNullString("fmt ", false);
	int32_t subchunk1Size = 16;
	buffer.write(subchunk1Size);
	int16_t audioFormat = 1;
	buffer.write(audioFormat);
	int16_t numChannels = m_channels;
	buffer.write(numChannels);
	auto hz = m_hz;
	buffer.write(hz);
	buffer.write(byteRate);
	int16_t blockAlign = numChannels * 2;
	buffer.write(blockAlign);
	int16_t bitsPerSample = 16;
	buffer.write(bitsPerSample);
	buffer.writeNullString("data", false);
	buffer.write(dataSize);
	
	// Write the data
	for (float sample : m_data) {
		int16_t pcm_value = static_cast<int16_t>(sample * 32767.0f);
		buffer.write(pcm_value);
	}

	return buffer;
}

bbe::Duration bbe::Sound::getDuration() const
{
	return Duration::fromMilliseconds(m_data.getLength() / getAmountOfChannels() * 1000 / getHz());
}

void bbe::SoundDataSourceStatic::setLooped(bool looped)
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

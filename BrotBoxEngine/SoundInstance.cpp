#ifndef BBE_NO_AUDIO

#include "BBE/SoundInstance.h"
#include "BBE/SoundManager.h"

bbe::SoundInstance::SoundInstance(uint64_t index)
	: m_index(index)
{
}

void bbe::SoundInstance::stop()
{
	bbe::INTERNAL::SoundManager::getInstance()->stopSoundWithIndex(m_index);
}

void bbe::SoundInstance::setPosition(const bbe::Vector3& pos)
{
	bbe::INTERNAL::SoundManager::getInstance()->setPosition(m_index, pos);
}

bool bbe::SoundInstance::isPlaying()
{
	return bbe::INTERNAL::SoundManager::getInstance()->isSoundWithIndexPlaying(m_index);
}

#endif

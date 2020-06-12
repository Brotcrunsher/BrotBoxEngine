#include "BBE/GameTime.h"

bbe::GameTime::GameTime()
{
	reset();
}

float bbe::GameTime::tick()
{
	auto currentTickTimestamp = std::chrono::high_resolution_clock::now();

	float timeSinceLastTick = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTickTimestamp - m_lastTickTimestamp).count() / 1000000000.0f;
	m_lastTickTimestamp = currentTickTimestamp;
	m_amountOfTicks++;

	return timeSinceLastTick;
}

float bbe::GameTime::timeSinceStartSeconds()
{
	float timeSinceLastTick = std::chrono::duration_cast<std::chrono::nanoseconds>(m_lastTickTimestamp - m_gameStartTimestamp).count() / 1000000000.0f;

	return timeSinceLastTick;
}

float bbe::GameTime::timeSinceStartMilliseconds()
{
	float timeSinceLastTick = (float)std::chrono::duration_cast<std::chrono::milliseconds>(m_lastTickTimestamp - m_gameStartTimestamp).count();

	return timeSinceLastTick;
}

uint64_t bbe::GameTime::getAmountOfTicks()
{
	return m_amountOfTicks;
}

void bbe::GameTime::reset()
{
	m_gameStartTimestamp = std::chrono::high_resolution_clock::now();
	m_lastTickTimestamp = m_gameStartTimestamp;
	m_amountOfTicks = 0;
}

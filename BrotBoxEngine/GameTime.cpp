#include "stdafx.h"
#include "BBE/GameTime.h"

bbe::GameTime::GameTime()
{
	reset();
}

float bbe::GameTime::tick()
{
	auto currentTickTimestamp = std::chrono::high_resolution_clock::now();

	float timeSinceLastTick = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTickTimestamp - m_lastTickTimestamp).count() / 1000000000.0;
	m_lastTickTimestamp = currentTickTimestamp;

	return timeSinceLastTick;
}

float bbe::GameTime::timeSinceStartSeconds()
{
	float timeSinceLastTick = std::chrono::duration_cast<std::chrono::nanoseconds>(m_lastTickTimestamp - m_gameStartTimestamp).count() / 1000000000.0;

	return timeSinceLastTick;
}

float bbe::GameTime::timeSinceStartMilliseconds()
{
	float timeSinceLastTick = (float)std::chrono::duration_cast<std::chrono::milliseconds>(m_lastTickTimestamp - m_gameStartTimestamp).count();

	return timeSinceLastTick;
}

void bbe::GameTime::reset()
{
	m_gameStartTimestamp = std::chrono::high_resolution_clock::now();
	m_lastTickTimestamp = m_gameStartTimestamp;
}

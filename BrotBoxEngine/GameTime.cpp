#include "stdafx.h"
#include "GameTime.h"

bbe::GameTime::GameTime()
{
	reset();
}

float bbe::GameTime::tick()
{
	auto currentTickTimestamp = std::chrono::high_resolution_clock::now();

	float timeSinceLastTick = std::chrono::duration_cast<std::chrono::milliseconds>(currentTickTimestamp - m_lastTickTimestamp).count() / 1000.0f;
	m_lastTickTimestamp = currentTickTimestamp;

	return timeSinceLastTick;
}

float bbe::GameTime::timeSinceStartSeconds()
{
	float timeSinceLastTick = std::chrono::duration_cast<std::chrono::milliseconds>(m_lastTickTimestamp - m_gameStartTimestamp).count() / 1000.0f;

	return timeSinceLastTick;
}

float bbe::GameTime::timeSinceStartMilliseconds()
{
	float timeSinceLastTick = std::chrono::duration_cast<std::chrono::milliseconds>(m_lastTickTimestamp - m_gameStartTimestamp).count();

	return timeSinceLastTick;
}

void bbe::GameTime::reset()
{
	m_gameStartTimestamp = std::chrono::high_resolution_clock::now();
	m_lastTickTimestamp = m_gameStartTimestamp;
}

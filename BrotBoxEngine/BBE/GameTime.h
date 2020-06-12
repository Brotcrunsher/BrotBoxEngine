#pragma once

#include <chrono>
#include <cstdint>

namespace bbe
{
	class GameTime
	{
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_gameStartTimestamp;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTickTimestamp;
		uint64_t m_amountOfTicks = 0;

	public:
		GameTime();

		float tick();
		float timeSinceStartSeconds();
		float timeSinceStartMilliseconds();
		uint64_t getAmountOfTicks();
		void reset();
	};
}

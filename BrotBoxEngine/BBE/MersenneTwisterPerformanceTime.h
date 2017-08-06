#pragma once

#include "../BBE/MersenneTwister.h"
#include "../BBE/CPUWatch.h"
#include <iostream>

namespace bbe
{
	namespace test
	{
		void testMersenneTwisterBBE()
		{
			mt19937 mt;
			CPUWatch watch;
			uint32_t val = 0;
			for (unsigned long long i = 0; i < 1000ull * 1000ull * 1000ull * 7ull; i++)
			{
				val += mt.next();
			}
			std::cout << "Mersenne Twister BBE Time: " << watch.getTimeExpiredSeconds() << std::endl;
		}

		void testMersenneTwisterSTL()
		{
			std::random_device ranDev;
			std::mt19937 mt(ranDev());

			std::uniform_int_distribution<uint32_t> dist(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());

			CPUWatch watch;
			uint32_t val = 0;
			for (int i = 0; i < 1000 * 1000 * 128; i++)
			{
				val += dist(mt);
			}
			std::cout << "Mersenne Twister STL Time: " << watch.getTimeExpiredSeconds() << std::endl;

		}
	}
}
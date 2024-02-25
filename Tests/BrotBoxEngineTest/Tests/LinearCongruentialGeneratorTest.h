#pragma once

#include "BBE/LinearCongruentialGenerator.h"
#include <iostream>


namespace bbe
{
	namespace test
	{
		void testLinearCongruentailGenerators()
		{
			bbe::LCG32 lcg;

			uint32_t seed1 = 1;
			uint64_t seed2 = 2;

			lcg.setSeed(seed1);
			lcg.setSeed(seed2);


			for (int i = 0; i < 16; i++)
			{
				BBELOGLN(lcg.next());
			}
		}
	}
}
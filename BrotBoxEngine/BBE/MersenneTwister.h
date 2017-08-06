#pragma once

#include <stdint.h>
#include <ctime>

namespace bbe
{
	template<typename FieldType, uint16_t N, int M, int R, int A, int F, int U, int D, int S, int B, int T, int C, int L>
	class MersenneTwisterBase
	{
	private:
		const unsigned long long MASK_LOWER = (1ull << R) - 1;
		const unsigned long long MASK_UPPER = (1ull << R);

		uint16_t index;
		FieldType mt[N];

		inline void twistIteration(uint32_t i)
		{
			uint32_t x = (mt[i] & MASK_UPPER) + (mt[(i + 1) % N] & MASK_LOWER);

			uint32_t xA = x >> 1;

			if (x & 1)
			{
				xA ^= A;
			}

			mt[i] = mt[(i + M) % N] ^ xA;
		}

		void twist()
		{
			for (uint32_t i = 0; i < N - 1; i++)
			{
				twistIteration(i);
			}

			twistIteration(N  - 1);

			index = 0;
		}

	public:
		MersenneTwisterBase()
		{
			std::time_t timeStamp = std::time(nullptr);
			setSeed((FieldType)timeStamp);
		}

		void setSeed(FieldType seed)
		{
			mt[0] = seed;

			for (uint32_t i = 1; i < N; i++)
			{
				mt[i] = (F * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i);
			}

			//obscurity magic number, decreases the predictability of the MT a little.
			mt[28] ^= 0xBBEBBEBB;

			index = N;

			for (uint32_t i = 0; i < 1000 * 1000 * 9; i++)
			{
				next();
			}
		}

		FieldType next()
		{
			if (index >= N)
			{
				twist();
			}

			FieldType y = mt[index];
			index++;

			y ^= (y >> U) & D;
			y ^= (y << S) & B;
			y ^= (y << T) & C;
			y ^= (y >> L);

			return y;
		}
	};

	typedef MersenneTwisterBase<uint32_t, 624, 397, 31, 0x9908B0DF, 1812433253, 11, 0xFFFFFFFF, 7, 0x9D2C5680, 15, 0xEFC60000, 18> mt19937;
}

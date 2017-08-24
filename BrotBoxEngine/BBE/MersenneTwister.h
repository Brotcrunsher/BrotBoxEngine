#pragma once

#include <stdint.h>
#include <ctime>

namespace bbe
{
	template<typename FieldType, uint16_t N, int M, int R, int A, int F, int U, int S, int B, int T, int C, int L>
	class MersenneTwisterBase
	{
	private:
		const uint64_t MASK_LOWER = (1ull << R) - 1;
		const uint64_t MASK_UPPER = (1ull << R);

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

			twistIteration(N - 1);	//Helps the compiler to calculate % N

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

			FieldType x = mt[index];
			index++;

			x ^= (x >> U);
			x ^= (x << S) & B;
			x ^= (x << T) & C;
			x ^= (x >> L);

			return y;
		}
	};

	//                         FieldType   N    M   R        A          F       U   S      B       T       C       L
	typedef MersenneTwisterBase<uint32_t, 624, 397, 31, 0x9908B0DF, 1812433253, 11, 7, 0x9D2C5680, 15, 0xEFC60000, 18> mt19937;
}

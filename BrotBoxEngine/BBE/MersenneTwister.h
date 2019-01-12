#pragma once

#include <stdint.h>
#include <ctime>

namespace bbe
{
	template<typename FieldType, uint16_t N, int M, int R, unsigned int A, int F, int U, int S, unsigned int B, int T, unsigned int C, int L>
	class MersenneTwisterBase
	{
	private:
		static constexpr uint64_t MASK_LOWER = (1ull << R) - 1;
		static constexpr uint64_t MASK_UPPER = (1ull << R);

		uint16_t  m_index;
		FieldType m_mt[N];

		inline void twistIteration(uint32_t i)
		{
			uint32_t x = (m_mt[i] & MASK_UPPER) + (m_mt[(i + 1) % N] & MASK_LOWER);

			uint32_t xA = x >> 1;

			if (x & 1)
			{
				xA ^= A;
			}

			m_mt[i] = m_mt[(i + M) % N] ^ xA;
		}

		void twist()
		{
			for (uint32_t i = 0; i < N - 1; i++)
			{
				twistIteration(i);
			}

			twistIteration(N - 1);	//Helps the compiler to calculate % N

			m_index = 0;
		}

	public:
		MersenneTwisterBase()
		{
			std::time_t timeStamp = std::time(nullptr);
			setSeed((FieldType)timeStamp);
		}

		void setSeed(FieldType seed)
		{
			m_mt[0] = seed;

			for (uint32_t i = 1; i < N; i++)
			{
				m_mt[i] = (F * (m_mt[i - 1] ^ (m_mt[i - 1] >> 30)) + i);
			}

			//obscurity magic number, decreases the predictability of the MT a little.
			m_mt[28] ^= 0xBBEBBEBB;

			m_index = N;

			for (uint32_t i = 0; i < 1000 * 1000 * 9; i++)
			{
				next();
			}
		}

		FieldType next()
		{
			if (m_index >= N)
			{
				twist();
			}

			FieldType x = m_mt[m_index];
			m_index++;

			x ^= (x >> U);
			x ^= (x << S) & B;
			x ^= (x << T) & C;
			x ^= (x >> L);

			return x;
		}
	};

	//                         FieldType   N    M   R        A            F       U   S      B       T       C       L
	typedef MersenneTwisterBase<uint32_t, 624, 397, 31, 0x9908B0DF, 1812433253, 11, 7, 0x9D2C5680, 15, 0xEFC60000, 18> mt19937;
}

#pragma once

#include <stdint.h>
#include <ctime>

namespace bbe
{
	template <typename InternalType, typename ExternalType, InternalType M, InternalType A, InternalType B, InternalType MASK>
	class LinearCongruentialGeneratorBase
	{
	private:
		InternalType m_lastNumber;

	public:

		LinearCongruentialGeneratorBase()
		{
			std::time_t timeStamp = std::time(nullptr);
			m_lastNumber = (InternalType)timeStamp;
		}

		void setSeed(InternalType seed)
		{
			m_lastNumber = seed;
		}

		template<typename dummyInternalType = InternalType>
		typename std::enable_if<!std::is_same<InternalType, ExternalType>::value, void>::type
		setSeed(ExternalType seed)
		{
			static_assert(std::is_same<dummyInternalType, InternalType>::value, "Do not specify dummy template argument!");
			m_lastNumber = (InternalType)seed;
		}

		ExternalType next()
		{
			m_lastNumber = (A * m_lastNumber + B) % M;
			return (ExternalType)(m_lastNumber & MASK);
		}
	};

	//typedef LinearCongruentialGeneratorBase<uint64_t, uint64_t, std::numeric_limits<uint64_t>::max(), 6364136223846793005L, 1442695040888963407L, 0xffffffffffffffff> LCG64Raw;
	//typedef LinearCongruentialGeneratorBase<uint32_t, uint32_t, std::numeric_limits<uint32_t>::max(), 134775813           , 1                   , 0xffffffff        > LCG32Raw;
	typedef LinearCongruentialGeneratorBase<uint64_t, uint32_t, std::numeric_limits<uint64_t>::max(), 6364136223846793005L, 1442695040888963407L, 0xffffffff        > LCG32;
}

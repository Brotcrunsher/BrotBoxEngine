#pragma once

#include <random>
#include "../BBE/DataType.h"
#include "../BBE/List.h"

namespace bbe {
	class Random
	{
	private:
		std::random_device m_ranDev;
		std::mt19937 m_mt;
		
		template<typename T>
		T randomInteger()
		{
			//UNTESTED
			std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
			return dist(m_mt);
		}

		template<typename T>
		T randomInteger(T max)
		{
			//UNTESTED
			std::uniform_int_distribution<T> dist(0, max - 1);
			return dist(m_mt);
		}

		template<typename T>
		T randomFloat()
		{
			//UNTESTED
			std::uniform_real_distribution<T> dist(0.0, 1.0);
			return dist(m_mt);
		}

		template<typename T>
		T randomFloat(T max)
		{
			//UNTESTED
			std::uniform_real_distribution<T> dist(0.0, max);
			return dist(m_mt);
		}

	public:
		explicit Random()
			: m_ranDev(), m_mt(m_ranDev())
		{
			//DO NOTHING
		}

		byte randomByte()
		{
			//UNTESTED
			return (byte)(randomInteger<unsigned short>() & 0xff);
		}

		byte randomByte(byte max)
		{
			//UNTESTED
			return (byte)(randomInteger<unsigned short>(max) & 0xff);
		}

		char randomChar()
		{
			//UNTESTED
			return (char)(randomInteger<short>() & 0xff);
		}

		short randomShort()
		{
			//UNTESTED
			return randomInteger<short>();
		}

		short randomShort(short max)
		{
			//UNTESTED
			return randomInteger<short>(max);
		}

		unsigned short randomUShort()
		{
			//UNTESTED
			return randomInteger<unsigned short>();
		}

		unsigned short randomUShort(unsigned short max)
		{
			//UNTESTED
			return randomInteger<unsigned short>(max);
		}

		int randomInt()
		{
			//UNTESTED
			return randomInteger<int>();
		}

		int randomInt(int max)
		{
			//UNTESTED
			return randomInteger<int>(max);
		}

		unsigned int randomUInt()
		{
			//UNTESTED
			return randomInteger<unsigned int>();
		}

		unsigned int randomUInt(unsigned int max)
		{
			//UNTESTED
			return randomInteger<unsigned int>(max);
		}

		long randomLong()
		{
			//UNTESTED
			return randomInteger<long>();
		}

		long randomLong(long max)
		{
			//UNTESTED
			return randomInteger<long>(max);
		}

		unsigned long randomULong()
		{
			//UNTESTED
			return randomInteger<unsigned long>();
		}

		unsigned long randomULong(unsigned long max)
		{
			//UNTESTED
			return randomInteger<unsigned long>(max);
		}
		
		float randomFloat()
		{
			//UNTESTED
			return randomFloat<float>();
		}

		float randomFloat(float max)
		{
			//UNTESTED
			return randomFloat<float>(max);
		}

		double randomDouble()
		{
			//UNTESTED
			return randomFloat<double>();
		}

		double randomDouble(double max)
		{
			//UNTESTED
			return randomFloat<double>(max);
		}

		bool randomBool()
		{
			//UNTESTED
			return randomFloat() > 0.5f;
		}

		void setSeed(unsigned int seed)
		{
			m_mt.seed(seed);
		}
	};
}

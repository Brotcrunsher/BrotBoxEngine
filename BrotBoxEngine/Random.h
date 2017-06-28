#pragma once

#include <random>
#include "DataType.h"
#include "List.h"

namespace bbe {
	class Random
	{
	private:
		std::random_device ranDev;
		std::mt19937 mt;
		
		template<typename T>
		T randomInteger()
		{
			std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
			return dist(mt);
		}

		template<typename T>
		T randomInteger(T max)
		{
			std::uniform_int_distribution<T> dist(0, max - 1);
			return dist(mt);
		}

		template<typename T>
		T randomFloat()
		{
			std::uniform_real_distribution<T> dist(0.0, 1.0);
			return dist(mt);
		}

		template<typename T>
		T randomFloat(T max)
		{
			std::uniform_real_distribution<T> dist(0.0, max);
			return dist(mt);
		}

	public:
		explicit Random()
			: ranDev(), mt(ranDev())
		{
			//DO NOTHING
		}

		byte randomByte()
		{
			return (byte)(randomInteger<unsigned short>() & 0xff);
		}

		byte randomByte(byte max)
		{
			return (byte)(randomInteger<unsigned short>(max) & 0xff);
		}

		char randomChar()
		{
			return (char)(randomInteger<short>() & 0xff);
		}

		short randomShort()
		{
			return randomInteger<short>();
		}

		short randomShort(short max)
		{
			return randomInteger<short>(max);
		}

		unsigned short randomUShort()
		{
			return randomInteger<unsigned short>();
		}

		unsigned short randomUShort(unsigned short max)
		{
			return randomInteger<unsigned short>(max);
		}

		int randomInt()
		{
			return randomInteger<int>();
		}

		int randomInt(int max)
		{
			return randomInteger<int>(max);
		}

		unsigned int randomUInt()
		{
			return randomInteger<unsigned int>();
		}

		unsigned int randomUInt(unsigned int max)
		{
			return randomInteger<unsigned int>(max);
		}

		long randomLong()
		{
			return randomInteger<long>();
		}

		long randomLong(long max)
		{
			return randomInteger<long>(max);
		}

		unsigned long randomULong()
		{
			return randomInteger<unsigned long>();
		}

		unsigned long randomULong(unsigned long max)
		{
			return randomInteger<unsigned long>(max);
		}
		
		float randomFloat()
		{
			return randomFloat<float>();
		}

		float randomFloat(float max)
		{
			return randomFloat<float>(max);
		}

		double randomDouble()
		{
			return randomFloat<double>();
		}

		double randomDouble(double max)
		{
			return randomFloat<double>(max);
		}

		bool randomBool()
		{
			return randomFloat() > 0.5f;
		}
	};
}
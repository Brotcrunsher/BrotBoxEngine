#pragma once

#include <random>
#include "../BBE/DataType.h"
#include "../BBE/List.h"
#include "../BBE/Vector2.h"
#include "../BBE/Vector3.h"
#include "../BBE/Vector4.h"

namespace bbe {
	class Random
	{
	private:
		std::random_device m_ranDev;
		std::mt19937       m_mt;
		
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

		Vector2 randomVector2()
		{
			//UNTESTED
			return Vector2(randomFloat(), randomFloat());
		}

		Vector2 randomVector2(float max)
		{
			//UNTESTED
			return Vector2(randomFloat(max), randomFloat(max));
		}

		Vector2 randomVector2(float maxX, float maxY)
		{
			//UNTESTED
			return Vector2(randomFloat(maxX), randomFloat(maxY));
		}

		Vector3 randomVector3()
		{
			//UNTESTED
			return Vector3(randomFloat(), randomFloat(), randomFloat());
		}

		Vector3 randomVector3(float max)
		{
			//UNTESTED
			return Vector3(randomFloat(max), randomFloat(max), randomFloat(max));
		}

		Vector3 randomVector3InUnitSphere()
		{
			while (true)
			{
				float x = randomFloat() * 2 - 1;
				float y = randomFloat() * 2 - 1;
				float z = randomFloat() * 2 - 1;
				Vector3 retVal(x, y, z);
				if (retVal.getLengthSq() <= 1)
				{
					return retVal;
				}
			}
		}

		Vector2 randomVector2InUnitSphere()
		{
			while (true)
			{
				float x = randomFloat() * 2 - 1;
				float y = randomFloat() * 2 - 1;
				Vector2 retVal(x, y);
				if (retVal.getLengthSq() <= 1)
				{
					return retVal;
				}
			}
		}

		Vector4 randomVector4()
		{
			//UNTESTED
			return Vector4(randomFloat(), randomFloat(), randomFloat(), randomFloat());
		}

		Vector4 randomVector4(float max)
		{
			//UNTESTED
			return Vector4(randomFloat(max), randomFloat(max), randomFloat(max), randomFloat(max));
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

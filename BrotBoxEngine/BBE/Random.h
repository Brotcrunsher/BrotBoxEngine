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
		T randomInteger_()
		{
			//UNTESTED
			std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
			return dist(m_mt);
		}

		template<typename T>
		T randomInteger_(T max)
		{
			//UNTESTED
			std::uniform_int_distribution<T> dist(0, max - 1);
			return dist(m_mt);
		}

		template<typename T>
		T randomFloat_()
		{
			//UNTESTED
			std::uniform_real_distribution<T> dist(0.0, 1.0);
			return dist(m_mt);
		}

		template<typename T>
		T randomFloat_(T max)
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

		Vector3 randomVector3(float maxX, float maxY, float maxZ)
		{
			//UNTESTED
			return Vector3(randomFloat(maxX), randomFloat(maxY), randomFloat(maxZ));
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

		Vector2 randomVector2OnUnitSphere()
		{
			return Vector2(1, 0).rotate(randomFloat() * bbe::Math::TAU);
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
			return (byte)(randomInteger_<unsigned short>() & 0xff);
		}

		byte randomByte(byte max)
		{
			//UNTESTED
			return (byte)(randomInteger_<unsigned short>(max) & 0xff);
		}

		char randomChar()
		{
			//UNTESTED
			return (char)(randomInteger_<short>() & 0xff);
		}

		short randomShort()
		{
			//UNTESTED
			return randomInteger_<short>();
		}

		short randomShort(short max)
		{
			//UNTESTED
			return randomInteger_<short>(max);
		}

		unsigned short randomUShort()
		{
			//UNTESTED
			return randomInteger_<unsigned short>();
		}

		unsigned short randomUShort(unsigned short max)
		{
			//UNTESTED
			return randomInteger_<unsigned short>(max);
		}

		int randomInt()
		{
			//UNTESTED
			return randomInteger_<int>();
		}

		int randomInt(int max)
		{
			// max is exclusive! [0, max)
			return randomInteger_<int>(max);
		}

		unsigned int randomUInt()
		{
			//UNTESTED
			return randomInteger_<unsigned int>();
		}

		unsigned int randomUInt(unsigned int max)
		{
			//UNTESTED
			return randomInteger_<unsigned int>(max);
		}

		long randomLong()
		{
			//UNTESTED
			return randomInteger_<long>();
		}

		long randomLong(long max)
		{
			//UNTESTED
			return randomInteger_<long>(max);
		}

		unsigned long randomULong()
		{
			//UNTESTED
			return randomInteger_<unsigned long>();
		}

		unsigned long randomULong(unsigned long max)
		{
			//UNTESTED
			return randomInteger_<unsigned long>(max);
		}
		
		float randomFloat()
		{
			//UNTESTED
			return randomFloat_<float>();
		}

		float randomFloat(float max)
		{
			//UNTESTED
			return randomFloat_<float>(max);
		}

		double randomDouble()
		{
			//UNTESTED
			return randomFloat_<double>();
		}

		double randomDouble(double max)
		{
			//UNTESTED
			return randomFloat_<double>(max);
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

		template<typename Container>
		auto sampleContainer(Container& container)
		{
			return container[randomInt(container.getLength())];
		}

		template<typename T>
		struct SampleBallsInBagPair
		{
			T element;
			uint32_t amountOfBallsInBag = 0;
		};
		template<typename Container>
		auto sampleContainerWithBag(Container& container)
		{
			uint32_t totalBalls = 0;
			for (size_t i = 0; i < container.getLength(); i++)
			{
				totalBalls += container[i].amountOfBallsInBag;
			}
			const uint32_t randomBall = randomInt(totalBalls);
			uint32_t ballsPicked = 0;
			for (size_t i = 0; i < container.getLength(); i++)
			{
				ballsPicked += container[i].amountOfBallsInBag;
				if (ballsPicked >= randomBall)
				{
					return container[i].element;
				}
			}
			bbe::Crash(bbe::Error::IllegalState);
		}
	};
}

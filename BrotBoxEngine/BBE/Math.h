#pragma once

#include <cstdio>
#include <limits>
#include "../BBE/List.h"

namespace bbe
{
	template<typename T> class Vector2_t;
	using Vector2 = Vector2_t<float>;
	using Vector2i = Vector2_t<int32_t>;
	class Vector3;
	class Vector4;

	namespace Math
	{
		namespace INTERNAL
		{
			void startMath();
			constexpr std::size_t TABLE_SIZES = 1024 * 64;
			extern double sinTable[TABLE_SIZES];
			extern double cosTable[TABLE_SIZES];
			extern double tanTable[TABLE_SIZES];
		}

		constexpr  int32_t BIGGEST_PRIME_32_SIGNED   = 2147483647;
		constexpr uint32_t BIGGEST_PRIME_32_UNSIGNED = 4294967295;


		constexpr double PI_d             = 3.1415926535897932384626433832795028841;
		constexpr double TAU_d            = 6.2831853071795864769252867665590057682;
		constexpr double GOLDEN_RATIO_d   = 1.6180339887498948482045868343656381177;
		constexpr float PI                = (float)PI_d;
		constexpr float TAU               = (float)TAU_d;
		constexpr float GOLDEN_RATIO      = (float)GOLDEN_RATIO_d;
		constexpr float E                 = 2.71828182845f;
		constexpr float SQRT2             = 1.41421356237f;
		constexpr float SQRT2INV          = 0.70710678118f;
		constexpr float INFINITY_POSITIVE = std::numeric_limits<float>::infinity();
		constexpr float INFINITY_NEGATIVE = -std::numeric_limits<float>::infinity();
		constexpr float NaN               = std::numeric_limits<float>::quiet_NaN();

		double pow(double base, double expo);

		double cos(double val);
		float acos(float val);
		double sin(double val);
		float asin(float val);
		double tan(double val);
		float atan(float val);
		float sqrt(float val);
		template<typename T>
		T mod(T val, T mod)
		{
			if (val >= 0)
			{
				if (val < mod)
				{
					return val;
				}
				int div = (int)(val / mod);
				return val - div * mod;
			}
			else
			{
				val = val - (int)(val / mod) * mod + mod;
				return val - (int)(val / mod) * mod;
			}
		}
		float pingpong(float val, float border);

		constexpr float toRadians(float val)
		{
			return val * 0.0174532925f;
		}

		constexpr float toDegrees(float val)
		{
			return val * 57.295779513f;
		}

		float sigmoid(float val);
		float hyperbolicTangent(float val);

		float floor(float val);
		float ceil(float val);
		float round(float val);
		float square(float val);
		template <typename T>
		T clamp(T val, T min, T max)
		{
			return val < min ? min : (val > max ? max : val);
		}
		float clamp01(float val);
		float normalDist(float x, float u, float o);
		bool  isInRange(float val, float min, float max);
		bool  isInRangeStrict(float val, float min, float max);
		bool  isInRange01(float val);
		bool  isInRange01Strict(float val);
		template <typename T>
		T abs(T val)
		{
			return val < 0 ? -val : val;
		}
		template <typename T>
		T max(T val1, T val2)
		{
			return val1 > val2 ? val1 : val2;
		}
		float max(float val1, float val2, float val3);
		float max(float val1, float val2, float val3, float val4);
		template <typename T>
		T min(T val1, T val2)
		{
			return val1 < val2 ? val1 : val2;
		}
		float min(float val1, float val2, float val3);
		float min(float val1, float val2, float val3, float val4);
		template <typename T>
		T maxAbs(T val1, T val2)
		{
			return max(abs(val1), abs(val2));
		}
		float maxAbs(float val1, float val2, float val3);
		template <typename T>
		T minAbs(T val1, T val2)
		{
			return min(abs(val1), abs(val2));
		}
		float minAbs(float val1, float val2, float val3);
		template <typename T>
		T maxAbsKeepSign(T val1, T val2)
		{
			if (abs(val1) > abs(val2))
			{
				return val1;
			}
			else
			{
				return val2;
			}
		}
		float maxAbsKeepSign(float val1, float val2, float val3);
		template <typename T>
		T minAbsKeepSign(T val1, T val2)
		{
			if (abs(val1) < abs(val2))
			{
				return val1;
			}
			else
			{
				return val2;
			}
		}
		float minAbsKeepSign(float val1, float val2, float val3);
		template <typename T>
		bool floatEquals(T val1, T val2, T epsilon)
		{
			return (val1 - val2) > epsilon ? false : ((val1 - val2) < -epsilon ? false : true);
		}
		float isNaN(float val);
		float isInfinity(float val);
		float isPositiveInfinity(float val);
		float isNegativeInfinity(float val);
		template<typename T>
		constexpr T nextMultiple(const T &multipleOf, const T &value)
		{
			return (value + multipleOf - 1) - ((value + multipleOf - 1) % multipleOf);
		}

		bool isOdd(int val);
		bool isEven(int val);

		int log2Floor(unsigned int val);
		float log2(float val);
		float logn(float val);
		float log10(float val);

		float interpolateLinear(float a, float b, float t);
		float interpolateBool(float a, float b, float t);
		float interpolateCosine(float a, float b, float t);
		float interpolateCubic(float preA, float a, float b, float postB, float t);
		float interpolateBezier(float a, float b, float t, float control);
		float interpolateHermite(float a, float b, float t, float tangent1, float tangent2);

		bool isLeftTurn(const bbe::Vector2& a, const bbe::Vector2& b, const bbe::Vector2& c);
		bbe::List<bbe::Vector2> getConvexHull(const bbe::List<bbe::Vector2>& points);
		const bbe::Vector2* getClosest(const bbe::Vector2& pos, const bbe::List<bbe::Vector2>& points);
		      bbe::Vector2* getClosest(const bbe::Vector2& pos,       bbe::List<bbe::Vector2>& points);
		
		template<typename Vec>
		bbe::List<Vec> project(const bbe::List<Vec>& points, const Vec& projection)
		{
			bbe::List<Vec> retVal;
			retVal.resizeCapacityAndLength(points.getLength());

			for (size_t i = 0; i < points.getLength(); i++)
			{
				retVal[i] = points[i].project(projection);
			}

			return retVal;
		}

		Vector2 interpolateLinear(Vector2 a, Vector2 b, float t);
		Vector2 interpolateBool(Vector2 a, Vector2 b, float t);
		Vector2 interpolateCosine(Vector2 a, Vector2 b, float t);
		Vector2 interpolateCubic(Vector2 preA, Vector2 a, Vector2 b, Vector2 postB, float t);
		Vector2 interpolateBezier(Vector2 a, Vector2 b, float t, Vector2 control);
		Vector2 interpolateBezier(Vector2 a, Vector2 b, float t, const bbe::List<Vector2> &controlPoints);
		Vector2 interpolateHermite(Vector2 a, Vector2 b, float t, Vector2 tangent1, Vector2 tangent2);

		Vector2 minComponent(const bbe::List<Vector2>& vectors);
		Vector2 maxComponent(const bbe::List<Vector2>& vectors);
		Vector2 minAbsComponent(const bbe::List<Vector2>& vectors);
		Vector2 maxAbsComponent(const bbe::List<Vector2>& vectors);
		Vector2 average(const bbe::List<Vector2>& vectors);
		Vector2 medianComponent(const bbe::List<Vector2>& vectors);

		// An integer space filling curve that returns elements in this order:
		// 0149
		// 325
		// 786
		Vector2i squareCantor(uint32_t index);

		Vector3 minComponent(const bbe::List<Vector3>& vectors);
		Vector3 maxComponent(const bbe::List<Vector3>& vectors);
		Vector3 average(const bbe::List<Vector3>& vectors);

		Vector3 interpolateLinear(Vector3 a, Vector3 b, float t);
		Vector3 interpolateBool(Vector3 a, Vector3 b, float t);
		Vector3 interpolateCosine(Vector3 a, Vector3 b, float t);
		Vector3 interpolateCubic(Vector3 preA, Vector3 a, Vector3 b, Vector3 postB, float t);
		Vector3 interpolateBezier(Vector3 a, Vector3 b, float t, Vector3 control);
		Vector3 interpolateHermite(Vector3 a, Vector3 b, float t, Vector3 tangent1, Vector3 tangent2);

		Vector4 interpolateLinear(Vector4 a, Vector4 b, float t);
		Vector4 interpolateBool(Vector4 a, Vector4 b, float t);
		Vector4 interpolateCosine(Vector4 a, Vector4 b, float t);
		Vector4 interpolateCubic(Vector4 preA, Vector4 a, Vector4 b, Vector4 postB, float t);
		Vector4 interpolateBezier(Vector4 a, Vector4 b, float t, Vector4 control);
		Vector4 interpolateHermite(Vector4 a, Vector4 b, float t, Vector4 tangent1, Vector4 tangent2);


	}
}
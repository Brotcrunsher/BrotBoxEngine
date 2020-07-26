#pragma once

#include <cstdio>
#include <limits>
#include "../BBE/List.h"

namespace bbe
{
	class Vector2;
	class Vector3;
	class Vector4;

	namespace Math
	{
		namespace INTERNAL
		{
			void startMath();
			constexpr std::size_t TABLE_SIZES = 1024 * 64;
			extern float sinTable[TABLE_SIZES];
			extern float cosTable[TABLE_SIZES];
			extern float tanTable[TABLE_SIZES];
		}

		constexpr  int32_t BIGGEST_PRIME_32_SIGNED   = 2147483647;
		constexpr uint32_t BIGGEST_PRIME_32_UNSIGNED = 4294967295;


		constexpr float PI                = 3.14159265359f;
		constexpr float TAU               = 6.28318530718f;
		constexpr float E                 = 2.71828182845f;
		constexpr float SQRT2             = 1.41421356237f;
		constexpr float SQRT2INV          = 0.70710678118f;
		constexpr float INFINITY_POSITIVE = std::numeric_limits<float>::infinity();
		constexpr float INFINITY_NEGATIVE = -std::numeric_limits<float>::infinity();
		constexpr float NaN               = std::numeric_limits<float>::quiet_NaN();

		float cos(float val);
		float acos(float val);
		float sin(float val);
		float asin(float val);
		float tan(float val);
		float atan(float val);
		float sqrt(float val);
		float mod(float val, float mod);
		float pingpong(float val, float border);

		constexpr float toRadians(float val)
		{
			return val * 0.0174532925f;
		}

		constexpr float toDegrees(float val)
		{
			return val * 57.295779513f;
		}

		float floor(float val);
		float ceil(float val);
		float round(float val);
		float square(float val);
		float clamp(float val, float min, float max);
		float clamp01(float val);
		float normalDist(float x, float u, float o);
		bool  isInRange(float val, float min, float max);
		bool  isInRangeStrict(float val, float min, float max);
		bool  isInRange01(float val);
		bool  isInRange01Strict(float val);
		float abs(float val);
		float (max)(float val1, float val2);
		float (max)(float val1, float val2, float val3);
		float (max)(float val1, float val2, float val3, float val4);
		float (min)(float val1, float val2);
		float (min)(float val1, float val2, float val3);
		float (min)(float val1, float val2, float val3, float val4);
		float maxAbs(float val1, float val2);
		float maxAbs(float val1, float val2, float val3);
		float minAbs(float val1, float val2);
		float minAbs(float val1, float val2, float val3);
		float maxAbsKeepSign(float val1, float val2);
		float maxAbsKeepSign(float val1, float val2, float val3);
		float minAbsKeepSign(float val1, float val2);
		float minAbsKeepSign(float val1, float val2, float val3);
		bool floatEquals(float val1, float val2, float epsilon);
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
#pragma once

#include <cstdio>

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
			constexpr std::size_t TABLE_SIZES = 2048;
			extern float sinTable[TABLE_SIZES];
			extern float cosTable[TABLE_SIZES];
			extern float tanTable[TABLE_SIZES];
		}

		extern const float PI;
		extern const float TAU;
		extern const float E;
		extern const float SQRT2;
		extern const float SQRT2INV;
		extern const float INFINITY_POSITIVE;
		extern const float INFINITY_NEGATIVE;
		extern const float NaN;

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
		float max(float val1, float val2);
		float max(float val1, float val2, float val3);
		float max(float val1, float val2, float val3, float val4);
		float min(float val1, float val2);
		float min(float val1, float val2, float val3);
		float min(float val1, float val2, float val3, float val4);
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

		Vector2 interpolateLinear(Vector2 a, Vector2 b, float t);
		Vector2 interpolateBool(Vector2 a, Vector2 b, float t);
		Vector2 interpolateCosine(Vector2 a, Vector2 b, float t);
		Vector2 interpolateCubic(Vector2 preA, Vector2 a, Vector2 b, Vector2 postB, float t);
		Vector2 interpolateBezier(Vector2 a, Vector2 b, float t, Vector2 control);

		Vector3 interpolateLinear(Vector3 a, Vector3 b, float t);
		Vector3 interpolateBool(Vector3 a, Vector3 b, float t);
		Vector3 interpolateCosine(Vector3 a, Vector3 b, float t);
		Vector3 interpolateCubic(Vector3 preA, Vector3 a, Vector3 b, Vector3 postB, float t);
		Vector3 interpolateBezier(Vector3 a, Vector3 b, float t, Vector3 control);

		Vector4 interpolateLinear(Vector4 a, Vector4 b, float t);
		Vector4 interpolateBool(Vector4 a, Vector4 b, float t);
		Vector4 interpolateCosine(Vector4 a, Vector4 b, float t);
		Vector4 interpolateCubic(Vector4 preA, Vector4 a, Vector4 b, Vector4 postB, float t);
		Vector4 interpolateBezier(Vector4 a, Vector4 b, float t, Vector4 control);


	}
}
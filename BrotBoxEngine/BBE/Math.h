#pragma once


namespace bbe
{
	namespace Math
	{
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
		float sqrt(float val);
		float mod(float val, float mod);
		float pingpong(float val, float border);

		float floor(float val);
		float ceil(float val);
		float round(float val);
		float square(float val);
		float clamp(float val, float min, float max);
		float clamp01(float val);
		bool  isInRange(float val, float min, float max);
		bool  isInRangeStrict(float val, float min, float max);
		bool  isInRange01(float val);
		bool  isInRange01Strict(float val);
		float abs(float val);
		float max(float val1, float val2);
		float min(float val1, float val2);
		float maxAbs(float val1, float val2);
		float minAbs(float val1, float val2);
		float maxAbsKeepSign(float val1, float val2);
		float minAbsKeepSign(float val1, float val2);
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
	}
}
#include "stdafx.h"
#include "BBE/Math.h"
#include <limits>
#include <cmath>


const float bbe::Math::PI = 3.14159265359f;
const float bbe::Math::TAU = 6.28318530718f;
const float bbe::Math::E = 2.71828182845f;
const float bbe::Math::SQRT2 = 1.41421356237f;
const float bbe::Math::SQRT2INV = 0.70710678118f;
const float bbe::Math::INFINITY_POSITIVE = std::numeric_limits<float>::infinity();
const float bbe::Math::INFINITY_NEGATIVE = -std::numeric_limits<float>::infinity();
const float bbe::Math::NaN = std::numeric_limits<float>::quiet_NaN();;

float bbe::Math::cos(float val)
{
	return ::cos(val);
}

float bbe::Math::acos(float val)
{
	return ::acos(val);
}

float bbe::Math::sin(float val)
{
	return ::sin(val);
}

float bbe::Math::asin(float val)
{
	return ::asin(val);
}

float bbe::Math::sqrt(float val)
{
	return ::sqrt(val);
}

float bbe::Math::floor(float val)
{
	int iVal = (int)val;
	if (val >= 0)
	{
		return (float)iVal;
	}
	else
	{
		if (iVal == val)
		{
			return iVal;
		}
		else
		{
			return iVal - 1;
		}
	}
}

float bbe::Math::ceil(float val)
{
	return -floor(-val);
}

float bbe::Math::round(float val)
{
	return val < 0 ? (int)(val - 0.5) : (int)(val + 0.5);
}

float bbe::Math::square(float val)
{
	return val * val;
}

float bbe::Math::clamp(float val, float min, float max)
{
	return val < min ? min : (val > max ? max : val);
}

float bbe::Math::minAbs(float val1, float val2)
{
	return min(abs(val1), abs(val2));
}

float bbe::Math::maxAbsKeepSign(float val1, float val2)
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

float bbe::Math::minAbsKeepSign(float val1, float val2)
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

bool bbe::Math::floatEquals(float val1, float val2, float epsilon)
{
	return (val1 - val2) > epsilon ? false : ((val1 - val2) < -epsilon ? false : true);
}

float bbe::Math::isNaN(float val)
{
	return val != val;
}

float bbe::Math::isInfinity(float val)
{
	return ::isinf(val);
}

float bbe::Math::isPositiveInfinity(float val)
{
	return ::isinf(val) && val > 0;
}

float bbe::Math::isNegativeInfinity(float val)
{
	return ::isinf(val) && val < 0;
}

bool bbe::Math::isOdd(int val)
{
	return (val & 1) == 1;
}

bool bbe::Math::isEven(int val)
{
	return (val & 1) == 0;
}

float bbe::Math::clamp01(float val)
{
	return (val < 0) ? 0 : (val > 1 ? 1 : val);
}

float bbe::Math::mod(float val, float mod)
{
	return ::fmod(val, mod);
}

float bbe::Math::pingpong(float val, float border)
{
	val = mod(val, border * 2);
	if (border > 0)
	{
		return border - abs(val - border);
	}
	else
	{
		return border + abs(val - border);
	}
}

bool bbe::Math::isInRange(float val, float min, float max)
{
	return val >= min && val <= max;
}

bool bbe::Math::isInRangeStrict(float val, float min, float max)
{
	return val > min && val < max;
}

bool bbe::Math::isInRange01(float val)
{
	return val >= 0 && val <= 1;
}

bool bbe::Math::isInRange01Strict(float val)
{
	return val > 0 && val < 1;
}

float bbe::Math::abs(const float val)
{
	return val < 0 ? -val : val;
}

float bbe::Math::max(float val1, float val2)
{
	return val1 > val2 ? val1 : val2;
}

float bbe::Math::min(float val1, float val2)
{
	return val1 < val2 ? val1 : val2;
}

float bbe::Math::maxAbs(float val1, float val2)
{
	return max(abs(val1), abs(val2));
}

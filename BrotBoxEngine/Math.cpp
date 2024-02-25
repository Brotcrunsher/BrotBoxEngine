#include "BBE/Math.h"
#include "BBE/Vector2.h"
#include "BBE/Vector3.h"
#include "BBE/Vector4.h"
#include <cmath>

double bbe::Math::pow(double base, double expo)
{
	return ::pow(base, expo);
}

double bbe::Math::cos(double val)
{
	return ::cos(val);
}

float bbe::Math::acos(float val)
{
	return ::acos(val);
}

double bbe::Math::sin(double val)
{
	return ::sin(val);
}

float bbe::Math::asin(float val)
{
	return ::asin(val);
}

double bbe::Math::tan(double val)
{
	return ::tan(val);
}

float bbe::Math::atan(float val)
{
	return ::atan(val);
}

float bbe::Math::sqrt(float val)
{
	return ::sqrt(val);
}

float bbe::Math::sigmoid(float val)
{
	return 1.f / (1 + static_cast<float>(pow(bbe::Math::E, -val)));
}

float bbe::Math::hyperbolicTangent(float val)
{
	return sigmoid(val) * 2.f - 1.f;
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
			return (float)iVal;
		}
		else
		{
			return (float)(iVal - 1);
		}
	}
}

float bbe::Math::ceil(float val)
{
	return -floor(-val);
}

float bbe::Math::round(float val)
{
	return val < 0 ? (float)(int)(val - 0.5) : (float)(int)(val + 0.5);
}

float bbe::Math::square(float val)
{
	return val * val;
}

float bbe::Math::minAbs(float val1, float val2, float val3)
{
	//UNTESTED
	val1 = abs(val1);
	val2 = abs(val2);
	val3 = abs(val3);

	if (val2 < val1) val1 = val2;
	if (val3 < val1) val1 = val3;

	return val1;
}

float bbe::Math::maxAbsKeepSign(float val1, float val2, float val3)
{
	//UNTESTED
	float aVal1 = abs(val1);
	float aVal2 = abs(val2);
	float aVal3 = abs(val3);

	if (aVal2 > aVal1) val1 = val2;
	if (aVal3 > aVal1) val1 = val3;

	return val1;
}

float bbe::Math::minAbsKeepSign(float val1, float val2, float val3)
{
	//UNTESTED
	float aVal1 = abs(val1);
	float aVal2 = abs(val2);
	float aVal3 = abs(val3);

	if (aVal2 < aVal1) val1 = val2;
	if (aVal3 < aVal1) val1 = val3;

	return val1;
}

bool bbe::Math::isNaN(float val)
{
	return val != val;
}

bool bbe::Math::isInfinity(float val)
{
	return std::isinf(val);
}

bool bbe::Math::isPositiveInfinity(float val)
{
	return std::isinf(val) && val > 0;
}

bool bbe::Math::isNegativeInfinity(float val)
{
	return std::isinf(val) && val < 0;
}

bool bbe::Math::isOdd(int val)
{
	return (val & 1) == 1;
}

bool bbe::Math::isEven(int val)
{
	return (val & 1) == 0;
}

int bbe::Math::log2Floor(unsigned int val)
{
	//UNTESTED
	//Should be the fastest possible way to calculate a floored log 2 of a number. 
	//Inspired by https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
	unsigned int retVal = 0;
	if (val & 0xFFFF0000)
	{
		val >>= 16;
		retVal |= 16;
	}
	if (val & 0xFF00)
	{
		val >>= 8;
		retVal |= 8;
	}
	if (val & 0xF0)
	{
		val >>= 4;
		retVal |= 4;
	}
	if (val & 0xC)
	{
		val >>= 2;
		retVal |= 2;
	}
	if (val & 0x2)
	{
		//val >>= 1; Has no effect.
		retVal |= 1;
	}
	return retVal;
}

float bbe::Math::log2(float val)
{
	//UNTESTED
	return log2f(val);
}

float bbe::Math::logn(float val)
{
	//UNTESTED
	return log(val);
}

float bbe::Math::log10(float val)
{
	//UNTESTED
	return log10f(val);
}

float bbe::Math::clamp01(float val)
{
	return (val < 0) ? 0 : (val > 1 ? 1 : val);
}

float bbe::Math::normalDist(float x, float u, float o)
{
	const float MULT = 1.0f / bbe::Math::sqrt(2 * bbe::Math::PI);
	float partExponent = (x - u) / o;
	float exponent = -0.5f * partExponent * partExponent;
	return MULT * ::pow(bbe::Math::E, exponent);
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

float bbe::Math::max(float val1, float val2, float val3)
{
	//UNTESTED
	if (val2 > val1) val1 = val2;
	if (val3 > val1) val1 = val3;
	return val1;
}

float bbe::Math::max(float val1, float val2, float val3, float val4)
{
	//UNTESTED
	if (val2 > val1) val1 = val2;
	if (val3 > val1) val1 = val3;
	if (val4 > val1) val1 = val4;
	return val1;
}

float bbe::Math::min(float val1, float val2, float val3)
{
	//UNTESTED
	if (val2 < val1) val1 = val2;
	if (val3 < val1) val1 = val3;
	return val1;
}

float bbe::Math::min(float val1, float val2, float val3, float val4)
{
	//UNTESTED
	if (val2 < val1) val1 = val2;
	if (val3 < val1) val1 = val3;
	if (val4 < val1) val1 = val4;
	return val1;
}

float bbe::Math::maxAbs(float val1, float val2, float val3)
{
	//UNTESTED
	val1 = abs(val1);
	val2 = abs(val2);
	val3 = abs(val3);

	if (val2 > val1) val1 = val2;
	if (val3 > val1) val1 = val3;
	return val1;
}



float bbe::Math::interpolateLinear(float a, float b, float t)
{
	//UNTESTED
	if (a == b) return a;
	return a * (1 - t) + b * t;
}

float bbe::Math::interpolateBool(float a, float b, float t)
{
	//UNTESTED
	if (t >= 0.5) return b;
	else return a;
}

float bbe::Math::interpolateCosine(float a, float b, float t)
{
	//UNTESTED
	if (a == b) return a;
	return interpolateLinear(a, b, (1 - static_cast<float>(cos(t * PI_d))) / 2);
}

float bbe::Math::interpolateCubic(float preA, float a, float b, float postB, float t)
{
	//UNTESTED
	if (a == preA && a == b && a == postB) return a;
	float t2 = t * t;
	float w0 = postB - b - preA + a;
	float w1 = preA - a - w0;
	float w2 = b - preA;
	float w3 = a;

	return (w0*t*t2 + w1*t2 + w2*t + w3);
}

float bbe::Math::interpolateBezier(float a, float b, float t, float control)
{
	//UNTESTED
	if (a == b && a == control) return a;
	float t2 = t * t;
	return b*t2 + 2 * control * t - 2 * control*t2 + a - 2 * a*t + a*t2;
}

float bbe::Math::interpolateHermite(float a, float b, float t, float tangent1, float tangent2)
{
	const float t2 = t * t;
	const float t3 = t2 * t;
	const float tm = t - 1;
	const float tm2 = tm * tm;

	return (1 - 3 * t2 + 2 * t3) * a
		+ t2 * (3 - 2 * t) * b
		+ t * tm2 * tangent1
		+ t2 * tm * tangent2;
}

bool bbe::Math::isLeftTurn(const bbe::Vector2& a, const bbe::Vector2& b, const bbe::Vector2& c)
{
	const bbe::Vector2 aToB = b - a;
	const bbe::Vector2 bToC = c - b;
	return aToB.isLeft(bToC);
}

bbe::List<bbe::Vector2> bbe::Math::getConvexHull(const bbe::List<bbe::Vector2>& points)
{
	if (points.getLength() < 3) return {};

	auto copy = points;
	copy.sort([](const bbe::Vector2& a, const bbe::Vector2& b)
		{
			if (a.x < b.x) return true;
			else if (a.x > b.x) return false;
			else
			{
				return a.y < b.y;
			}
		});

	bbe::List<bbe::Vector2> retVal;
	retVal.add(copy[0]);
	retVal.add(copy[1]);
	for (size_t i = 2; i < copy.getLength(); i++)
	{
		while (retVal.getLength() >= 2 && bbe::Math::isLeftTurn(retVal[retVal.getLength() - 1], retVal[retVal.getLength() - 2], copy[i]))
		{
			retVal.removeIndex(retVal.getLength() - 1);
		}
		if (retVal[retVal.getLength() - 1] != copy[i])
		{
			retVal.add(copy[i]);
		}
	}

	retVal.add(copy[copy.getLength() - 2]);
	for (size_t i = copy.getLength() - 3; i != (size_t)-1; i--)
	{
		while (retVal.getLength() >= 2 && bbe::Math::isLeftTurn(retVal[retVal.getLength() - 1], retVal[retVal.getLength() - 2], copy[i]))
		{
			retVal.removeIndex(retVal.getLength() - 1);
		}
		if (retVal[retVal.getLength() - 1] != copy[i])
		{
			retVal.add(copy[i]);
		}
	}
	if (retVal.getLength() > 0) retVal.removeIndex(retVal.getLength() - 1);

	return retVal;
}

const bbe::Vector2* bbe::Math::getClosest(const bbe::Vector2& pos, const bbe::List<bbe::Vector2>& points)
{
	if (points.getLength() == 0) return nullptr;

	float minDist = pos.getDistanceTo(points[0]);
	const bbe::Vector2* minVec = &points[0];

	for (size_t i = 1; i < points.getLength(); i++)
	{
		const float dist = pos.getDistanceTo(points[i]);
		if (dist < minDist)
		{
			minDist = dist;
			minVec = &points[i];
		}
	}
	return minVec;
}

bbe::Vector2* bbe::Math::getClosest(const bbe::Vector2& pos, bbe::List<bbe::Vector2>& points)
{
	const bbe::List<bbe::Vector2>& cPoints = points;
	const bbe::Vector2* minVec = getClosest(pos, cPoints);
	return const_cast<bbe::Vector2*>(minVec);
}

bbe::Vector2 bbe::Math::interpolateLinear(Vector2 a, Vector2 b, float t)
{
	return Vector2(
		interpolateLinear(a.x, b.x, t),
		interpolateLinear(a.y, b.y, t)
	);
}

bbe::Vector2 bbe::Math::interpolateBool(Vector2 a, Vector2 b, float t)
{
	return Vector2(
		interpolateBool(a.x, b.x, t),
		interpolateBool(a.y, b.y, t)
	);
}

bbe::Vector2 bbe::Math::interpolateCosine(Vector2 a, Vector2 b, float t)
{
	return Vector2(
		interpolateCosine(a.x, b.x, t),
		interpolateCosine(a.y, b.y, t)
	);
}

bbe::Vector2 bbe::Math::interpolateCubic(Vector2 preA, Vector2 a, Vector2 b, Vector2 postB, float t)
{
	return Vector2(
		interpolateCubic(preA.x, a.x, b.x, postB.x, t),
		interpolateCubic(preA.y, a.y, b.y, postB.y, t)
	);
}

bbe::Vector2 bbe::Math::interpolateBezier(Vector2 a, Vector2 b, float t, Vector2 control)
{
	return Vector2(
		interpolateBezier(a.x, b.x, t, control.x),
		interpolateBezier(a.y, b.y, t, control.y)
	);
}

bbe::Vector2 bbe::Math::interpolateBezier(Vector2 a, Vector2 b, float t, const bbe::List<Vector2> &controlPoints)
{
	if (controlPoints.getLength() == 0) return bbe::Math::interpolateLinear(a, b, t);
	if (controlPoints.getLength() == 1) return bbe::Math::interpolateBezier(a, b, t, controlPoints[0]);

	bbe::List<Vector2> intermediatePoints(controlPoints.getLength() + 2);
	intermediatePoints.add(a);
	intermediatePoints.addArray(controlPoints.getRaw(), controlPoints.getLength());
	intermediatePoints.add(b);

	const size_t originalLength = intermediatePoints.getLength();

	for (size_t i = 0; i < originalLength - 1; i++)
	{
		for (size_t k = 0; k < originalLength - i - 1; k++)
		{
			intermediatePoints[k] = bbe::Math::interpolateLinear(intermediatePoints[k], intermediatePoints[k + 1], t);
		}
	}

	return intermediatePoints[0];
}

bbe::Vector2 bbe::Math::interpolateHermite(Vector2 a, Vector2 b, float t, Vector2 tangent1, Vector2 tangent2)
{
	return Vector2(
		interpolateHermite(a.x, b.x, t, tangent1.x, tangent2.x),
		interpolateHermite(a.y, b.y, t, tangent1.y, tangent2.y)
	);
}

bbe::Vector2 bbe::Math::minComponent(const bbe::List<Vector2>& vectors)
{
	bbe::Vector2 retVal(INFINITY_POSITIVE, INFINITY_POSITIVE);

	for (const Vector2& vec : vectors)
	{
		if (vec.x < retVal.x) retVal.x = vec.x;
		if (vec.y < retVal.y) retVal.y = vec.y;
	}

	return retVal;
}

bbe::Vector2i bbe::Math::squareCantor(uint32_t index)
{
	if (index == 0) return bbe::Vector2i(0, 0);
	const uint32_t sqrt = (uint32_t)bbe::Math::sqrt(index); // 2
	const uint32_t sq = sqrt * sqrt; // 1
	const uint32_t distOnHull = index - sq; // 2

	if (distOnHull <= sqrt) // 2 <= 3
	{
		return Vector2i(sqrt, distOnHull); //2 1
	}
	else
	{
		return Vector2i(distOnHull - sqrt - 1, sqrt); // 2, 1
	}
}

bbe::Vector3 bbe::Math::minComponent(const bbe::List<Vector3>& vectors)
{
	bbe::Vector3 retVal(INFINITY_POSITIVE, INFINITY_POSITIVE, INFINITY_POSITIVE);

	for (const Vector3& vec : vectors)
	{
		if (vec.x < retVal.x) retVal.x = vec.x;
		if (vec.y < retVal.y) retVal.y = vec.y;
		if (vec.z < retVal.z) retVal.z = vec.z;
	}

	return retVal;
}

bbe::Vector2 bbe::Math::maxComponent(const bbe::List<Vector2>& vectors)
{
	bbe::Vector2 retVal(INFINITY_NEGATIVE, INFINITY_NEGATIVE);

	for (const Vector2& vec : vectors)
	{
		if (vec.x > retVal.x) retVal.x = vec.x;
		if (vec.y > retVal.y) retVal.y = vec.y;
	}

	return retVal;
}

bbe::Vector3 bbe::Math::maxComponent(const bbe::List<Vector3>& vectors)
{
	bbe::Vector3 retVal(INFINITY_NEGATIVE, INFINITY_NEGATIVE, INFINITY_NEGATIVE);

	for (const Vector3& vec : vectors)
	{
		if (vec.x > retVal.x) retVal.x = vec.x;
		if (vec.y > retVal.y) retVal.y = vec.y;
		if (vec.z > retVal.z) retVal.z = vec.z;
	}

	return retVal;
}

bbe::Vector2 bbe::Math::minAbsComponent(const bbe::List<Vector2>& vectors)
{
	bbe::Vector2 retVal(INFINITY_POSITIVE, INFINITY_POSITIVE);

	for (const Vector2& vec : vectors)
	{
		if (vec.x < abs(retVal.x)) retVal.x = abs(vec.x);
		if (vec.y < abs(retVal.y)) retVal.y = abs(vec.y);
	}

	return retVal;
}

bbe::Vector2 bbe::Math::maxAbsComponent(const bbe::List<Vector2>& vectors)
{
	bbe::Vector2 retVal(INFINITY_NEGATIVE, INFINITY_NEGATIVE);

	for (const Vector2& vec : vectors)
	{
		if (vec.x > abs(retVal.x)) retVal.x = abs(vec.x);
		if (vec.y > abs(retVal.y)) retVal.y = abs(vec.y);
	}

	return retVal;
}

bbe::Vector3 bbe::Math::average(const bbe::List<Vector3>& vectors)
{
	bbe::Vector3 retVal = bbe::Vector3();

	for (const bbe::Vector3& v : vectors)
	{
		retVal += v;
	}

	return retVal / static_cast<float>(vectors.getLength());
}

bbe::Vector2 bbe::Math::medianComponent(const bbe::List<Vector2>& vectors)
{
	bbe::List<float> xs;
	bbe::List<float> ys;

	for (const bbe::Vector2& v : vectors)
	{
		if (!isNaN(v.x)) xs.add(v.x);
		if (!isNaN(v.y)) ys.add(v.y);
	}
	xs.sort();
	ys.sort();

	bbe::Vector2 retVal(NaN, NaN);

	if (xs.getLength() > 0) retVal.x = xs[xs.getLength() / 2];
	if (ys.getLength() > 0) retVal.y = ys[ys.getLength() / 2];

	return retVal;
}

bbe::Vector3 bbe::Math::interpolateLinear(Vector3 a, Vector3 b, float t)
{
	return Vector3(
		interpolateLinear(a.x, b.x, t),
		interpolateLinear(a.y, b.y, t),
		interpolateLinear(a.z, b.z, t)
	);
}

bbe::Vector3 bbe::Math::interpolateBool(Vector3 a, Vector3 b, float t)
{
	return Vector3(
		interpolateBool(a.x, b.x, t),
		interpolateBool(a.y, b.y, t),
		interpolateBool(a.z, b.z, t)
	);
}

bbe::Vector3 bbe::Math::interpolateCosine(Vector3 a, Vector3 b, float t)
{
	return Vector3(
		interpolateCosine(a.x, b.x, t),
		interpolateCosine(a.y, b.y, t),
		interpolateCosine(a.z, b.z, t)
	);
}

bbe::Vector3 bbe::Math::interpolateCubic(Vector3 preA, Vector3 a, Vector3 b, Vector3 postB, float t)
{
	return Vector3(
		interpolateCubic(preA.x, a.x, b.x, postB.x, t),
		interpolateCubic(preA.y, a.y, b.y, postB.y, t),
		interpolateCubic(preA.z, a.z, b.z, postB.z, t)
	);
}

bbe::Vector3 bbe::Math::interpolateBezier(Vector3 a, Vector3 b, float t, Vector3 control)
{
	return Vector3(
		interpolateBezier(a.x, b.x, t, control.x),
		interpolateBezier(a.y, b.y, t, control.y),
		interpolateBezier(a.z, b.z, t, control.z)
	);
}

bbe::Vector3 bbe::Math::interpolateHermite(Vector3 a, Vector3 b, float t, Vector3 tangent1, Vector3 tangent2)
{
	return Vector3(
		interpolateHermite(a.x, b.x, t, tangent1.x, tangent2.x),
		interpolateHermite(a.y, b.y, t, tangent1.y, tangent2.y),
		interpolateHermite(a.z, b.z, t, tangent1.z, tangent2.z)
	);
}

bbe::Vector4 bbe::Math::interpolateLinear(Vector4 a, Vector4 b, float t)
{
	return Vector4(
		interpolateLinear(a.x, b.x, t),
		interpolateLinear(a.y, b.y, t),
		interpolateLinear(a.z, b.z, t),
		interpolateLinear(a.w, b.w, t)
	);
}

bbe::Vector4 bbe::Math::interpolateBool(Vector4 a, Vector4 b, float t)
{
	return Vector4(
		interpolateBool(a.x, b.x, t),
		interpolateBool(a.y, b.y, t),
		interpolateBool(a.z, b.z, t),
		interpolateBool(a.w, b.w, t)
	);
}

bbe::Vector4 bbe::Math::interpolateCosine(Vector4 a, Vector4 b, float t)
{
	return Vector4(
		interpolateCosine(a.x, b.x, t),
		interpolateCosine(a.y, b.y, t),
		interpolateCosine(a.z, b.z, t),
		interpolateCosine(a.w, b.w, t)
	);
}

bbe::Vector4 bbe::Math::interpolateCubic(Vector4 preA, Vector4 a, Vector4 b, Vector4 postB, float t)
{
	return Vector4(
		interpolateCubic(preA.x, a.x, b.x, postB.x, t),
		interpolateCubic(preA.y, a.y, b.y, postB.y, t),
		interpolateCubic(preA.z, a.z, b.z, postB.z, t),
		interpolateCubic(preA.w, a.w, b.w, postB.w, t)
	);
}

bbe::Vector4 bbe::Math::interpolateBezier(Vector4 a, Vector4 b, float t, Vector4 control)
{
	return Vector4(
		interpolateBezier(a.x, b.x, t, control.x),
		interpolateBezier(a.y, b.y, t, control.y),
		interpolateBezier(a.z, b.z, t, control.z),
		interpolateBezier(a.w, b.w, t, control.w)
	);
}

bbe::Vector4 bbe::Math::interpolateHermite(Vector4 a, Vector4 b, float t, Vector4 tangent1, Vector4 tangent2)
{
	return Vector4(
		interpolateHermite(a.x, b.x, t, tangent1.x, tangent2.x),
		interpolateHermite(a.y, b.y, t, tangent1.y, tangent2.y),
		interpolateHermite(a.z, b.z, t, tangent1.z, tangent2.z),
		interpolateHermite(a.w, b.w, t, tangent1.w, tangent2.w)
	);
}

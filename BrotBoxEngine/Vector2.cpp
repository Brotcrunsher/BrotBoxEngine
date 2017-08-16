#include "stdafx.h"
#include "BBE/Vector2.h"
#include "BBE/Math.h"
#include "BBE/Math.h"
#include "BBE/Exceptions.h"

bbe::Vector2::Vector2()
	: x(0), y(0)
{
}

bbe::Vector2::Vector2(float x, float y)
	: x(x), y(y)
{
}

bbe::Vector2::Vector2(float xy)
	: x(xy), y(xy)
{
}

bbe::Vector2 bbe::Vector2::operator*(float scalar) const
{
	return Vector2(x * scalar, y * scalar);
}

float bbe::Vector2::operator*(const Vector2 & other) const
{
	return x * other.x + y * other.y;
}

bbe::Vector2 bbe::Vector2::operator/(float scalar) const
{
	return Vector2(x / scalar, y / scalar);
}

bbe::Vector2 bbe::Vector2::operator+(const Vector2 & other) const
{
	return Vector2(x + other.x, y + other.y);
}

bbe::Vector2 bbe::Vector2::operator-(const Vector2 & other) const
{
	return Vector2(x - other.x, y - other.y);
}

bbe::Vector2 bbe::Vector2::operator-() const
{
	return Vector2(-x, -y);
}

bbe::Vector2 bbe::Vector2::createVector2OnUnitCircle(float radians)
{
	float x = Math::cos(radians);
	float y = Math::sin(radians);
	return Vector2(x, y);
}

float & bbe::Vector2::operator[](int index)
{
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	default:
		throw IllegalIndexException();
	}
}

const float & bbe::Vector2::operator[](int index) const
{
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	default:
		throw IllegalIndexException();
	}
}

bool bbe::Vector2::operator==(const Vector2 & other) const
{
	return x == other.x && y == other.y;
}

bool bbe::Vector2::operator!=(const Vector2 & other) const
{
	return !(operator==(other));
}

bool bbe::Vector2::operator>(const Vector2 & other) const
{
	return getLengthSq() > other.getLengthSq();
}

bool bbe::Vector2::operator>=(const Vector2 & other) const
{
	return getLengthSq() >= other.getLengthSq();
}

bool bbe::Vector2::operator<(const Vector2 & other) const
{
	return getLengthSq() < other.getLengthSq();
}

bool bbe::Vector2::operator<=(const Vector2 & other) const
{
	return getLengthSq() <= other.getLengthSq();
}

bool bbe::Vector2::equals(const Vector2 & other, float epsilon) const
{
	return Math::floatEquals(x, other.x, epsilon)
		&& Math::floatEquals(y, other.y, epsilon);
}

bool bbe::Vector2::isSameLength(const Vector2 & other, float epsilon) const
{
	return Math::floatEquals(getLengthSq(), other.getLengthSq(), epsilon);
}

bool bbe::Vector2::isSameDirection(const Vector2 & other) const
{
	return operator*(other) > 0;
}

bool bbe::Vector2::isOppositeDirection(const Vector2 & other) const
{
	return operator*(other) < 0;
}

bool bbe::Vector2::isLeft(const Vector2 & other) const
{
	return (rotate90CounterClockwise() * other) > 0;
}

bool bbe::Vector2::isRight(const Vector2 & other) const
{
	return (rotate90Clockwise() * other) > 0;
}

bool bbe::Vector2::isContainingNaN() const
{
	return Math::isNaN(x) || Math::isNaN(y);
}

bool bbe::Vector2::isContainingInfinity() const
{
	return Math::isInfinity(x) || Math::isInfinity(y);
}

bool bbe::Vector2::isUnit(float epsilon) const
{
	float length = getLength();
	return length > 1 - epsilon && length < 1 + epsilon;
}

bool bbe::Vector2::isCloseTo(const Vector2 & other, float maxDistance) const
{
	return (operator-(other)).getLength() <= maxDistance;
}

bool bbe::Vector2::isZero() const
{
	return x == 0 && y == 0;
}

bbe::Vector2 bbe::Vector2::rotate(float radians) const
{
	float sin = Math::sin(radians);
	float cos = Math::cos(radians);

	return Vector2(
		x * cos - y * sin,
		x * sin + y * cos
	);
}

bbe::Vector2 bbe::Vector2::rotate(float radians, const Vector2 &center) const
{
	Vector2 rotVec = operator-(center);
	rotVec = rotVec.rotate(radians);
	return rotVec + center;
}

bbe::Vector2 bbe::Vector2::rotate90Clockwise() const
{
	return Vector2(-y, x);
}

bbe::Vector2 bbe::Vector2::rotate90CounterClockwise() const
{
	return Vector2(y, -x);
}

bbe::Vector2 bbe::Vector2::setLenght(float length) const
{
	return normalize()*length;
}

bbe::Vector2 bbe::Vector2::normalize() const
{
	float length = getLength();
	if (length == 0)
	{
		return Vector2(1, 0);
	}
	return Vector2(x / length, y / length);
}

bbe::Vector2 bbe::Vector2::abs() const
{
	return Vector2(Math::abs(x), Math::abs(y));
}

bbe::Vector2 bbe::Vector2::clampComponents(float min, float max) const
{
	return Vector2(Math::clamp(x, min, max), Math::clamp(y, min, max));
}

bbe::Vector2 bbe::Vector2::project(const Vector2 & other) const
{
	float scalar = operator*(other);
	scalar /= other.getLengthSq();
	return other * scalar;
}

bbe::Vector2 bbe::Vector2::reflect(const Vector2 & normal) const
{
	Vector2 normalized = normal.normalize();
	return operator-(normalized * 2 * (operator*(normalized)));
}

float bbe::Vector2::getLength() const
{
	return Math::sqrt(getLengthSq());
}

float bbe::Vector2::getLengthSq() const
{
	return x * x + y * y;
}

float bbe::Vector2::getDistanceTo(const Vector2 & other) const
{
	return (operator-(other)).getLength();
}

float bbe::Vector2::getMax() const
{
	return Math::max(x, y);
}

float bbe::Vector2::getMin() const
{
	return Math::min(x, y);
}

float bbe::Vector2::getMaxAbs() const
{
	return Math::maxAbs(x, y);
}

float bbe::Vector2::getMinAbs() const
{
	return Math::minAbs(x, y);
}

float bbe::Vector2::getMaxAbsKeepSign() const
{
	//UNTESTED
	return Math::maxAbsKeepSign(x, y);
}

float bbe::Vector2::getMinAbsKeepSign() const
{
	//UNTESTED
	return Math::minAbsKeepSign(x, y);
}

float bbe::Vector2::getAngle() const
{
	float angle = Math::acos(x / getLength());
	if (y >= 0)
	{
		return angle;
	}
	else
	{
		return Math::TAU - angle;
	}
}

float bbe::Vector2::getAngle(const Vector2 & other) const
{
	float angle = Math::acos(operator*(other) / getLength() / other.getLength());
	if (isLeft(other))
	{
		return angle;
	}
	else
	{
		return Math::TAU - angle;
	}
}

bbe::Vector2 bbe::Vector2::xx() const
{
	return Vector2(x, x);
}

bbe::Vector2 bbe::Vector2::xy() const
{
	return Vector2(x, y);
}

bbe::Vector2 bbe::Vector2::yx() const
{
	return Vector2(y, x);
}

bbe::Vector2 bbe::Vector2::yy() const
{
	return Vector2(y, y);
}

#include "stdafx.h"
#include "BBE/Math.h"
#include "BBE/Vector3.h"
#include "BBE/Vector2.h"
#include "BBE/Matrix4.h"
#include "BBE/Exceptions.h"

bbe::Vector3::Vector3()
	: x(0), y(0), z(0)
{
	//UNTESTED
}

bbe::Vector3::Vector3(float x, float y, float z)
	: x(x), y(y), z(z)
{
	//UNTESTED
}

bbe::Vector3::Vector3(float x, const Vector2 &yz)
	: x(x), y(yz.x), z(yz.y)
{
	//UNTESTED
}

bbe::Vector3::Vector3(const Vector2 &xy, float z)
	: x(xy.x), y(xy.y), z(z)
{
	//UNTESTED
}

bbe::Vector3 bbe::Vector3::operator+(const Vector3 & other) const
{
	//UNTESTED
	return Vector3(x + other.x, y + other.y, z + other.z);
}

bbe::Vector3 bbe::Vector3::operator-(const Vector3 & other) const
{
	//UNTESTED
	return Vector3(x - other.x, y - other.y, z - other.z);
}

bbe::Vector3 bbe::Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}

bbe::Vector3 bbe::Vector3::operator*(float scalar) const
{
	//UNTESTED
	return Vector3(x * scalar, y * scalar, z * scalar);
}

bbe::Vector3 bbe::Vector3::operator/(float scalar) const
{
	//UNTESTED
	return Vector3(x / scalar, y / scalar, z / scalar);
}

float bbe::Vector3::operator*(const Vector3 & other) const
{
	//UNTESTED
	return x * other.x + y * other.y + z * other.z;
}

float & bbe::Vector3::operator[](int index)
{
	//UNTESTED
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	default:
		throw IllegalIndexException();
	}
}

const float & bbe::Vector3::operator[](int index) const
{
	//UNTESTED
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	default:
		throw IllegalIndexException();
	}
}

bool bbe::Vector3::operator==(const Vector3 & other) const
{
	//UNTESTED
	return x == other.x && y == other.y && z == other.z;
}

bool bbe::Vector3::operator!=(const Vector3 & other) const
{
	//UNTESTED
	return !operator==(other);
}

bool bbe::Vector3::operator>(const Vector3 & other) const
{
	//UNTESTED
	return getLength() > other.getLength();
}

bool bbe::Vector3::operator>=(const Vector3 & other) const
{
	//UNTESTED
	return getLength() >= other.getLength();
}

bool bbe::Vector3::operator<(const Vector3 & other) const
{
	//UNTESTED
	return getLength() < other.getLength();
}

bool bbe::Vector3::operator<=(const Vector3 & other) const
{
	//UNTESTED
	return getLength() <= other.getLength();
}

bool bbe::Vector3::equals(const Vector3 & other, float epsilon) const
{
	//UNTESTED
	return Math::floatEquals(x, other.x, epsilon)
		&& Math::floatEquals(y, other.y, epsilon)
		&& Math::floatEquals(z, other.z, epsilon);
}

bool bbe::Vector3::isSameLength(const Vector3 & other, float epsilon) const
{
	//UNTESTED
	return Math::floatEquals(getLength(), other.getLength(), epsilon);
}

bool bbe::Vector3::isSameDirection(const Vector3 & other) const
{
	//UNTESTED
	return operator*(other) > 0;
}

bool bbe::Vector3::isOppositeDirection(const Vector3 & other) const
{
	//UNTESTED
	return operator*(other) < 0;
}

bool bbe::Vector3::isLeft(const Vector3 & other) const
{
	return (other.y * x - other.x * y) > 0;
}

bool bbe::Vector3::isRight(const Vector3 & other) const
{
	return (-other.y * x + other.x * y) > 0;
}

bool bbe::Vector3::isContainingNaN() const
{
	//UNTESTED
	return Math::isNaN(x)
		|| Math::isNaN(y)
		|| Math::isNaN(z);
}

bool bbe::Vector3::isContainingInfinity() const
{
	//UNTESTED
	return Math::isInfinity(x)
		|| Math::isInfinity(y)
		|| Math::isInfinity(z);
}

bool bbe::Vector3::isUnit(float epsilon) const
{
	//UNTESTED
	return Math::floatEquals(getLength(), 1.0f, epsilon);
}

bool bbe::Vector3::isCloseTo(const Vector3 & other, float maxDistance) const
{
	//UNTESTED
	return operator-(other).getLengthSq() <= maxDistance * maxDistance;
}

bool bbe::Vector3::isZero() const
{
	//UNTESTED
	return x == 0 && y == 0 && z == 0;
}

bbe::Vector3 bbe::Vector3::rotate(float radians, const Vector3 & axisOfRotation) const
{
	return Matrix4::createRotationMatrix(radians, axisOfRotation) * (*this);
}

bbe::Vector3 bbe::Vector3::rotate(float radians, const Vector3 & axisOfRotation, const Vector3 & center) const
{
	Vector3 rotVec = operator-(center);
	rotVec = rotVec.rotate(radians, axisOfRotation);
	return rotVec + center;
}

bbe::Vector3 bbe::Vector3::setLenght(float length) const
{
	//UNTESTED
	return normalize() * length;
}

bbe::Vector3 bbe::Vector3::normalize() const
{
	//UNTESTED
	float length = getLength();
	if (length == 0)
	{
		return Vector3(1, 0, 0);
	}
	return Vector3(x / length, y / length, z / length);
}

bbe::Vector3 bbe::Vector3::abs() const
{
	//UNTESTED
	return Vector3(Math::abs(x), Math::abs(y), Math::abs(z));
}

bbe::Vector3 bbe::Vector3::clampComponents(float min, float max) const
{
	//UNTESTED
	return Vector3(
		Math::clamp(x, min, max),
		Math::clamp(y, min, max),
		Math::clamp(z, min, max)
	);
}

bbe::Vector3 bbe::Vector3::project(const Vector3 & other) const
{
	float scalar = operator*(other);
	scalar /= other.getLengthSq();
	return other * scalar;
}

bbe::Vector3 bbe::Vector3::reflect(const Vector3 & normal) const
{
	Vector3 normalized = normal.normalize();
	return operator-(normalized * 2 * (operator*(normalized)));
}

bbe::Vector3 bbe::Vector3::cross(const Vector3 & other) const
{
	return Vector3(
		y * other.z - other.y * z,
		z * other.x - other.z * x,
		x * other.y - other.x * y
	);
}

float bbe::Vector3::getLength() const
{
	//UNTESTED
	return Math::sqrt(getLengthSq());
}

float bbe::Vector3::getLengthSq() const
{
	//UNTESTED
	return x * x + y * y + z * z;
}

float bbe::Vector3::getDistanceTo(const Vector3 & other) const
{
	//UNTESTED
	return operator-(other).getLength();
}

float bbe::Vector3::getMax() const
{
	return Math::max(x, y, z);
}

float bbe::Vector3::getMin() const
{
	return Math::min(x, y, z);
}

float bbe::Vector3::getMaxAbs() const
{
	return Math::maxAbs(x, y, z);
}

float bbe::Vector3::getMinAbs() const
{
	return Math::minAbs(x, y, z);
}

float bbe::Vector3::getMaxAbsKeepSign() const
{
	return Math::maxAbsKeepSign(x, y, z);
}

float bbe::Vector3::getMinAbsKeepSign() const
{
	return Math::minAbsKeepSign(x, y, z);
}

float bbe::Vector3::getAngle() const
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

float bbe::Vector3::getAngle(const Vector3 & other) const
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


//Begin Swizzles
bbe::Vector2 bbe::Vector3::xx() const
{
	return Vector2(x, x);
}

bbe::Vector2 bbe::Vector3::xy() const
{
	return Vector2(x, y);
}

bbe::Vector2 bbe::Vector3::xz() const
{
	return Vector2(x, z);
}

bbe::Vector2 bbe::Vector3::yx() const
{
	return Vector2(y, x);
}

bbe::Vector2 bbe::Vector3::yy() const
{
	return Vector2(y, y);
}

bbe::Vector2 bbe::Vector3::yz() const
{
	return Vector2(y, z);
}

bbe::Vector2 bbe::Vector3::zx() const
{
	return Vector2(z, x);
}

bbe::Vector2 bbe::Vector3::zy() const
{
	return Vector2(z, y);
}

bbe::Vector2 bbe::Vector3::zz() const
{
	return Vector2(z, z);
}

bbe::Vector3 bbe::Vector3::xxx() const
{
	return Vector3(x, x, x);
}

bbe::Vector3 bbe::Vector3::xxy() const
{
	return Vector3(x, x, y);
}

bbe::Vector3 bbe::Vector3::xxz() const
{
	return Vector3(x, x, z);
}

bbe::Vector3 bbe::Vector3::xyx() const
{
	return Vector3(x, y, x);
}

bbe::Vector3 bbe::Vector3::xyy() const
{
	return Vector3(x, y, y);
}

bbe::Vector3 bbe::Vector3::xyz() const
{
	return Vector3(x, y, z);
}

bbe::Vector3 bbe::Vector3::xzx() const
{
	return Vector3(x, z, x);
}

bbe::Vector3 bbe::Vector3::xzy() const
{
	return Vector3(x, z, y);
}

bbe::Vector3 bbe::Vector3::xzz() const
{
	return Vector3(x, z, z);
}

bbe::Vector3 bbe::Vector3::yxx() const
{
	return Vector3(y, x, x);
}

bbe::Vector3 bbe::Vector3::yxy() const
{
	return Vector3(y, x, y);
}

bbe::Vector3 bbe::Vector3::yxz() const
{
	return Vector3(y, x, z);
}

bbe::Vector3 bbe::Vector3::yyx() const
{
	return Vector3(y, y, x);
}

bbe::Vector3 bbe::Vector3::yyy() const
{
	return Vector3(y, y, y);
}

bbe::Vector3 bbe::Vector3::yyz() const
{
	return Vector3(y, y, z);
}

bbe::Vector3 bbe::Vector3::yzx() const
{
	return Vector3(y, z, x);
}

bbe::Vector3 bbe::Vector3::yzy() const
{
	return Vector3(y, z, y);
}

bbe::Vector3 bbe::Vector3::yzz() const
{
	return Vector3(y, z, z);
}

bbe::Vector3 bbe::Vector3::zxx() const
{
	return Vector3(z, x, x);
}

bbe::Vector3 bbe::Vector3::zxy() const
{
	return Vector3(z, x, y);
}

bbe::Vector3 bbe::Vector3::zxz() const
{
	return Vector3(z, x, z);
}

bbe::Vector3 bbe::Vector3::zyx() const
{
	return Vector3(z, y, x);
}

bbe::Vector3 bbe::Vector3::zyy() const
{
	return Vector3(z, y, y);
}

bbe::Vector3 bbe::Vector3::zyz() const
{
	return Vector3(z, y, z);
}

bbe::Vector3 bbe::Vector3::zzx() const
{
	return Vector3(z, z, x);
}

bbe::Vector3 bbe::Vector3::zzy() const
{
	return Vector3(z, z, y);
}

bbe::Vector3 bbe::Vector3::zzz() const
{
	return Vector3(z, z, z);
}

#include "stdafx.h"
#include "BBE/Vector2.h"
#include "BBE/Vector3.h"
#include "BBE/Vector4.h"
#include "BBE/Exceptions.h"
#include "BBE\Vector4.h"

bbe::Vector4::Vector4()
	: x(0), y(0), z(0), w(0)
{
}

bbe::Vector4::Vector4(float x, float y, float z, float w)
	: x(x), y(y), z(z), w(w)
{
}

bbe::Vector4::Vector4(float x, float y, const bbe::Vector2 &zw)
	: x(x), y(y), z(zw.x), w(zw.y)
{
}

bbe::Vector4::Vector4(const bbe::Vector2 &xy, float z, float w)
	: x(xy.x), y(xy.y), z(z), w(w)
{
}

bbe::Vector4::Vector4(const bbe::Vector2 &xy, const bbe::Vector2 &zw)
	: x(xy.x), y(xy.y), z(zw.x), w(zw.y)
{
}

bbe::Vector4::Vector4(float x, const bbe::Vector2 &yz, float w)
	: x(x), y(yz.x), z(yz.y), w(w)
{
}

bbe::Vector4::Vector4(const bbe::Vector3 &xyz, float w)
	: x(xyz.x), y(xyz.y), z(xyz.z), w(w)
{
}

bbe::Vector4::Vector4(float x, const bbe::Vector3 &yzw)
	: x(x), y(yzw.x), z(yzw.y), w(yzw.z)
{
}

bbe::Vector4 bbe::Vector4::operator+(const bbe::Vector4 & other) const
{
	return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}

bbe::Vector4 bbe::Vector4::operator-(const Vector4 & other) const
{
	return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

bbe::Vector4 bbe::Vector4::operator*(float scalar) const
{
	return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

bbe::Vector4 bbe::Vector4::operator/(float scalar) const
{
	return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
}

float& bbe::Vector4::operator[](int index)
{
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		throw IllegalIndexException();
	}
}

const float& bbe::Vector4::operator[](int index) const
{
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		throw IllegalIndexException();
	}
}

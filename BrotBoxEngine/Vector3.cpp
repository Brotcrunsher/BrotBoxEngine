#include "stdafx.h"
#include "BBE/Vector3.h"
#include "BBE/Vector2.h"
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

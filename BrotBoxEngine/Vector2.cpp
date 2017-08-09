#include "stdafx.h"
#include "BBE/Vector2.h"
#include "BBE/Math.h"

bbe::Vector2::Vector2()
	: x(0), y(0)
{
}

bbe::Vector2::Vector2(float x, float y)
	: x(x), y(y)
{
}

bbe::Vector2 bbe::Vector2::operator*(float scalar) const
{
	return Vector2(x * scalar, y * scalar);
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

bbe::Vector2 bbe::Vector2::createVector2OnUnitCircle(float radians)
{
	float x = Math::sin(radians);
	float y = Math::cos(radians);
	return Vector2(x, y);
}

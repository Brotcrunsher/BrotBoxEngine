#include "stdafx.h"
#include "BBE/Vector3.h"
#include "BBE/Vector2.h"

bbe::Vector3::Vector3()
	: x(0), y(0), z(0)
{
}

bbe::Vector3::Vector3(float x, float y, float z)
	: x(x), y(y), z(z)
{
}

bbe::Vector3::Vector3(float x, Vector2 yz)
	: x(x), y(yz.x), z(yz.y)
{
}

bbe::Vector3::Vector3(Vector2 xy, float z)
	: x(xy.x), y(xy.y), z(z)
{
}

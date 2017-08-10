#include "stdafx.h"
#include "BBE/Matrix4.h"
#include "BBE/Exceptions.h"

bbe::Matrix4::Matrix4()
{
	static_assert(sizeof(Matrix4) == sizeof(float) * 16, "The size of a Matrix4 must be sizeof(float) * 16!");


	for (int i = 0; i < 4; i++)
	{
		for (int k = 0; k < 4; k++)
		{
			if (i == k)
			{
				m_cols[i][k] = 1;
			}
			else
			{
				m_cols[i][k] = 0;
			}
		}
	}
}

float bbe::Matrix4::get(int row, int col) const
{
	if (row < 0 || row > 3 || col < 0 || col > 3)
	{
		throw IllegalIndexException();
	}

	return operator[](row + col * 4);
}

void bbe::Matrix4::set(int row, int col, float val)
{
	if (row < 0 || row > 3 || col < 0 || col > 3)
	{
		throw IllegalIndexException();
	}

	operator[](row + col * 4) = val;
}

float & bbe::Matrix4::operator[](int index)
{
	if (index < 0 || index > 15)
	{
		throw IllegalIndexException();
	}
	float* data = reinterpret_cast<float*>(this);

	return data[index];
}

const float & bbe::Matrix4::operator[](int index) const
{
	if (index < 0 || index > 15)
	{
		throw IllegalIndexException();
	}
	const float* data = reinterpret_cast<const float*>(this);

	return data[index];
}

bbe::Matrix4 bbe::Matrix4::operator*(const Matrix4 other) const
{
	Matrix4 retVal;
	retVal.set(0, 0, get(0, 0) * other.get(0, 0) + get(0, 1) * other.get(1, 0) + get(0, 2) * other.get(2, 0) + get(0, 3) * other.get(3, 0));
	retVal.set(0, 1, get(0, 0) * other.get(0, 1) + get(0, 1) * other.get(1, 1) + get(0, 2) * other.get(2, 1) + get(0, 3) * other.get(3, 1));
	retVal.set(0, 2, get(0, 0) * other.get(0, 2) + get(0, 1) * other.get(1, 2) + get(0, 2) * other.get(2, 2) + get(0, 3) * other.get(3, 2));
	retVal.set(0, 3, get(0, 0) * other.get(0, 3) + get(0, 1) * other.get(1, 3) + get(0, 2) * other.get(2, 3) + get(0, 3) * other.get(3, 3));

	retVal.set(1, 0, get(1, 0) * other.get(0, 0) + get(1, 1) * other.get(1, 0) + get(1, 2) * other.get(2, 0) + get(1, 3) * other.get(3, 0));
	retVal.set(1, 1, get(1, 0) * other.get(0, 1) + get(1, 1) * other.get(1, 1) + get(1, 2) * other.get(2, 1) + get(1, 3) * other.get(3, 1));
	retVal.set(1, 2, get(1, 0) * other.get(0, 2) + get(1, 1) * other.get(1, 2) + get(1, 2) * other.get(2, 2) + get(1, 3) * other.get(3, 2));
	retVal.set(1, 3, get(1, 0) * other.get(0, 3) + get(1, 1) * other.get(1, 3) + get(1, 2) * other.get(2, 3) + get(1, 3) * other.get(3, 3));

	retVal.set(2, 0, get(2, 0) * other.get(0, 0) + get(2, 1) * other.get(1, 0) + get(2, 2) * other.get(2, 0) + get(2, 3) * other.get(3, 0));
	retVal.set(2, 1, get(2, 0) * other.get(0, 1) + get(2, 1) * other.get(1, 1) + get(2, 2) * other.get(2, 1) + get(2, 3) * other.get(3, 1));
	retVal.set(2, 2, get(2, 0) * other.get(0, 2) + get(2, 1) * other.get(1, 2) + get(2, 2) * other.get(2, 2) + get(2, 3) * other.get(3, 2));
	retVal.set(2, 3, get(2, 0) * other.get(0, 3) + get(2, 1) * other.get(1, 3) + get(2, 2) * other.get(2, 3) + get(2, 3) * other.get(3, 3));

	retVal.set(3, 0, get(3, 0) * other.get(0, 0) + get(3, 1) * other.get(1, 0) + get(3, 2) * other.get(2, 0) + get(3, 3) * other.get(3, 0));
	retVal.set(3, 1, get(3, 0) * other.get(0, 1) + get(3, 1) * other.get(1, 1) + get(3, 2) * other.get(2, 1) + get(3, 3) * other.get(3, 1));
	retVal.set(3, 2, get(3, 0) * other.get(0, 2) + get(3, 1) * other.get(1, 2) + get(3, 2) * other.get(2, 2) + get(3, 3) * other.get(3, 2));
	retVal.set(3, 3, get(3, 0) * other.get(0, 3) + get(3, 1) * other.get(1, 3) + get(3, 2) * other.get(2, 3) + get(3, 3) * other.get(3, 3));
	return retVal;
}

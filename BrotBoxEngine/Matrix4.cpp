#include "stdafx.h"
#include "BBE/Matrix4.h"
#include "BBE/Math.h"
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

bbe::Matrix4::Matrix4(const Vector4 & col0, const Vector4 & col1, const Vector4 & col2, const Vector4 & col3)
{
	m_cols[0] = col0;
	m_cols[1] = col1;
	m_cols[2] = col2;
	m_cols[3] = col3;
}

bbe::Matrix4 bbe::Matrix4::createTranslationMatrix(const Vector3 & translation)
{
	Matrix4 retVal;
	retVal.set(0, 3, translation.x);
	retVal.set(1, 3, translation.y);
	retVal.set(2, 3, translation.z);
	return retVal;
}

bbe::Matrix4 bbe::Matrix4::createRotationMatrix(float radians, const Vector3 & rotationAxis)
{
	if (radians == 0)
	{
		return Matrix4();
	}
	Vector3 nra = rotationAxis.normalize();
	float x = nra.x;
	float y = nra.y;
	float z = nra.z;
	float cos = Math::cos(radians);
	float sin = Math::sin(radians);

	Matrix4 retVal;
	retVal.set(0, 0, cos + x * x * (1 - cos)    );   retVal.set(0, 1, x * y * (1 - cos) - z * sin);   retVal.set(0, 2, x * z * (1 - cos) + y * sin);
	retVal.set(1, 0, y * x * (1 - cos) + z * sin);   retVal.set(1, 1, cos + y * y * (1 - cos)    );   retVal.set(1, 2, y * z * (1 - cos) - x * sin);
	retVal.set(2, 0, z * x * (1 - cos) - y * sin);   retVal.set(2, 1, z * y * (1 - cos) + x * sin);   retVal.set(2, 2, cos + z * z * (1 - cos)    );

	return retVal;
}

bbe::Matrix4 bbe::Matrix4::createScaleMatrix(const Vector3 & scale)
{
	Matrix4 retVal;
	retVal.set(0, 0, scale.x);
	retVal.set(1, 1, scale.y);
	retVal.set(2, 2, scale.z);
	return retVal;
}

bbe::Matrix4 bbe::Matrix4::createPerspectiveMatrix(float fieldOfView, float aspectRatio, float nearClipPlane, float farClipPlane)
{
	float tanFoV = Math::tan(fieldOfView / 2.0f);

	Matrix4 retVal;
	retVal.set(0, 0, 1.0f / tanFoV / aspectRatio);
	retVal.set(1, 1, -1.0f / tanFoV);
	retVal.set(2, 2, -(farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane));
	retVal.set(3, 3, 0);
	
	retVal.set(3, 2, -1);
	retVal.set(2, 3, -(2 * farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane));

	return retVal;
}

bbe::Matrix4 bbe::Matrix4::createViewMatrix(const Vector3 & cameraPos, const Vector3 & lookTarget, const Vector3 & upDirection)
{
	Vector3 direction = (lookTarget - cameraPos).normalize();
	Vector3 right = direction.cross(upDirection).normalize();
	Vector3 down = right.cross(direction);

	Matrix4 retVal;

	retVal.set(0, 0, right.x);
	retVal.set(0, 1, right.y);
	retVal.set(0, 2, right.z);
	retVal.set(1, 0, down.x);
	retVal.set(1, 1, down.y);
	retVal.set(1, 2, down.z);
	retVal.set(2, 0, -direction.x);
	retVal.set(2, 1, -direction.y);
	retVal.set(2, 2, -direction.z);
	retVal.set(0, 3, -(right * cameraPos));
	retVal.set(1, 3, -(down * cameraPos));
	retVal.set(2, 3, direction * cameraPos);
	
	return retVal;
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

bbe::Vector4 bbe::Matrix4::operator*(const Vector4 & other) const
{
	return Vector4(
		get(0, 0) * other.x + get(0, 1) * other.y + get(0, 2) * other.z + get(0, 3) * other.w,
		get(1, 0) * other.x + get(1, 1) * other.y + get(1, 2) * other.z + get(1, 3) * other.w,
		get(2, 0) * other.x + get(2, 1) * other.y + get(2, 2) * other.z + get(2, 3) * other.w,
		get(3, 0) * other.x + get(3, 1) * other.y + get(3, 2) * other.z + get(3, 3) * other.w
	);
}

bbe::Vector4 bbe::Matrix4::getColumn(int colIndex) const
{
	if (colIndex < 0 || colIndex > 3)
	{
		throw IllegalIndexException();
	}
	return Vector4(m_cols[colIndex]);
}

bbe::Vector4 bbe::Matrix4::getRow(int rowIndex) const
{
	if (rowIndex < 0 || rowIndex > 3)
	{
		throw IllegalIndexException();
	}
	return Vector4(m_cols[0][rowIndex], m_cols[1][rowIndex], m_cols[2][rowIndex], m_cols[3][rowIndex]);
}

bbe::Vector3 bbe::Matrix4::extractTranslation() const
{
	return getColumn(3).xyz();
}

bbe::Vector3 bbe::Matrix4::extractScale() const
{
	return Vector3(
		getColumn(0).xyz().getLength(),
		getColumn(1).xyz().getLength(),
		getColumn(2).xyz().getLength()
	);
}

bbe::Matrix4 bbe::Matrix4::extractRotation() const
{
	Vector3 scale = extractScale();
	return Matrix4(
		Vector4((getColumn(0) / scale.x).xyz(), 0),
		Vector4((getColumn(1) / scale.y).xyz(), 0),
		Vector4((getColumn(2) / scale.z).xyz(), 0),
		Vector4(0, 0, 0, 1)
	);
}

bbe::Matrix4 bbe::Matrix4::operator*(const Matrix4 &other) const
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

bbe::Vector3 bbe::Matrix4::operator*(const Vector3 & other) const
{
	Vector4 retVal(other, 1);
	retVal = operator*(retVal);
	return Vector3(retVal.x / retVal.w, retVal.y / retVal.w, retVal.z / retVal.w);
}

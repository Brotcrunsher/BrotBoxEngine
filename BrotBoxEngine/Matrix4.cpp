#include "BBE/Matrix4.h"
#include "BBE/Math.h"
#include "BBE/Exceptions.h"

bbe::Matrix4::Matrix4()
{
	static_assert(sizeof(Matrix4) == sizeof(float) * 16, "The size of a Matrix4 must be sizeof(float) * 16!");

	m_cols[0].x = 1;
	m_cols[0].y = 0;
	m_cols[0].z = 0;
	m_cols[0].w = 0;

	m_cols[1].x = 0;
	m_cols[1].y = 1;
	m_cols[1].z = 0;
	m_cols[1].w = 0;

	m_cols[2].x = 0;
	m_cols[2].y = 0;
	m_cols[2].z = 1;
	m_cols[2].w = 0;

	m_cols[3].x = 0;
	m_cols[3].y = 0;
	m_cols[3].z = 0;
	m_cols[3].w = 1;
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
	retVal.m_cols[3].x = translation.x;
	retVal.m_cols[3].y = translation.y;
	retVal.m_cols[3].z = translation.z;
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
	float cos = static_cast<float>(Math::cos(radians));
	float sin = static_cast<float>(Math::sin(radians));

	Matrix4 retVal;
	retVal.m_cols[0].x = cos + x * x * (1 - cos);
	retVal.m_cols[1].x = x * y * (1 - cos) - z * sin;
	retVal.m_cols[2].x = x * z * (1 - cos) + y * sin;

	retVal.m_cols[0].y = y * x * (1 - cos) + z * sin;
	retVal.m_cols[1].y = cos + y * y * (1 - cos);
	retVal.m_cols[2].y = y * z * (1 - cos) - x * sin;

	retVal.m_cols[0].z = z * x * (1 - cos) - y * sin;
	retVal.m_cols[1].z = z * y * (1 - cos) + x * sin;
	retVal.m_cols[2].z = cos + z * z * (1 - cos);

	return retVal;
}

bbe::Matrix4 bbe::Matrix4::createRotationMatrix(const Vector3& from, const Vector3& to)
{
	const Vector3 a = from.normalize();
	const Vector3 b = to.normalize();
	if (a == b)
	{
		return bbe::Matrix4();
	}
	const Vector3 v = a.cross(b);
	const float c = a * b;

	bbe::Matrix4 retVal;
	retVal.m_cols[0].x +=    0;
	retVal.m_cols[0].y +=  v.z;
	retVal.m_cols[0].z += -v.y;

	retVal.m_cols[1].x += -v.z;
	retVal.m_cols[1].y +=    0;
	retVal.m_cols[1].z +=  v.x;

	retVal.m_cols[2].x +=  v.y;
	retVal.m_cols[2].y += -v.x;
	retVal.m_cols[2].z +=    0;

	if (c == -1.0f) return retVal;
	const float mult = 1.0f / (1.0f + c);

	const float x = v.x;
	const float y = v.y;
	const float z = v.z;
	const float x2 = x * x;
	const float y2 = y * y;
	const float z2 = z * z;

	retVal.m_cols[0].x += (-z2 + -y2) * mult;
	retVal.m_cols[0].y += ( -x * -y ) * mult;
	retVal.m_cols[0].z += (  x *  z ) * mult;

	retVal.m_cols[1].x += (  x *   y) * mult;
	retVal.m_cols[1].y += (-z2 + -x2) * mult;
	retVal.m_cols[1].z += ( -y *  -z) * mult;

	retVal.m_cols[2].x += ( -x *  -z) * mult;
	retVal.m_cols[2].y += (  y *   z) * mult;
	retVal.m_cols[2].z += (-y2 + -x2) * mult;

	return retVal;
}

bbe::Matrix4 bbe::Matrix4::createScaleMatrix(const Vector3 & scale)
{
	Matrix4 retVal;
	retVal.m_cols[0].x = scale.x;
	retVal.m_cols[1].y = scale.y;
	retVal.m_cols[2].z = scale.z;
	return retVal;
}

bbe::Matrix4 bbe::Matrix4::createPerspectiveMatrix(float fieldOfView, float aspectRatio, float nearClipPlane, float farClipPlane)
{
	float tanFoV = (float)Math::tan(fieldOfView / 2.0f);

	Matrix4 retVal;
	retVal.m_cols[0].x = 1.0f / tanFoV / aspectRatio;
	retVal.m_cols[1].y = -1.0f / tanFoV;
	retVal.m_cols[2].z = -(farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane);
	retVal.m_cols[3].w = 0;

	retVal.m_cols[2].w = -1;
	retVal.m_cols[3].z = -(2 * farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane);

	return retVal;
}

bbe::Matrix4 bbe::Matrix4::createViewMatrix(const Vector3 & cameraPos, const Vector3 & lookTarget, const Vector3 & upDirection)
{
	Vector3 direction = (lookTarget - cameraPos).normalize();
	Vector3 right = direction.cross(upDirection).normalize();
	Vector3 down = right.cross(direction);

	Matrix4 retVal;
	retVal.m_cols[0].x = right.x;
	retVal.m_cols[1].x = right.y;
	retVal.m_cols[2].x = right.z;

	retVal.m_cols[0].y = down.x;
	retVal.m_cols[1].y = down.y;
	retVal.m_cols[2].y = down.z;

	retVal.m_cols[0].z = -direction.x;
	retVal.m_cols[1].z = -direction.y;
	retVal.m_cols[2].z = -direction.z;

	retVal.m_cols[3].x = -(right * cameraPos);
	retVal.m_cols[3].y = -(down * cameraPos);
	retVal.m_cols[3].z = direction * cameraPos;
	
	return retVal;
}

bbe::Matrix4 bbe::Matrix4::createTransform(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	Matrix4 matTranslation = Matrix4::createTranslationMatrix(pos);
	Matrix4 matScale = Matrix4::createScaleMatrix(scale);
	Matrix4 matRotation = Matrix4::createRotationMatrix(radians, rotationVector);

	return matTranslation * matRotation * matScale;
}

float bbe::Matrix4::get(int row, int col) const
{
	if (row < 0 || row > 3 || col < 0 || col > 3)
	{
		throw IllegalIndexException();
	}

	switch (row)
	{
	case 0:
		return m_cols[col].x;
	case 1:
		return m_cols[col].y;
	case 2:
		return m_cols[col].z;
	case 3:
		return m_cols[col].w;
	}

	throw IllegalIndexException();
}

void bbe::Matrix4::set(int row, int col, float val)
{
	if (row < 0 || row > 3 || col < 0 || col > 3)
	{
		throw IllegalIndexException();
	}

	switch (row)
	{
	case 0:
		m_cols[col].x = val;
		break;
	case 1:
		m_cols[col].y = val;
		break;
	case 2:
		m_cols[col].z = val;
		break;
	case 3:
		m_cols[col].w = val;
		break;
	}
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
		m_cols[0].x * other.x + m_cols[1].x * other.y + m_cols[2].x * other.z + m_cols[3].x * other.w,
		m_cols[0].y * other.x + m_cols[1].y * other.y + m_cols[2].y * other.z + m_cols[3].y * other.w,
		m_cols[0].z * other.x + m_cols[1].z * other.y + m_cols[2].z * other.z + m_cols[3].z * other.w,
		m_cols[0].w * other.x + m_cols[1].w * other.y + m_cols[2].w * other.z + m_cols[3].w * other.w
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
	const Vector4 col = getColumn(3);
	return Vector3(col.x, col.y, col.z);
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
	retVal.m_cols[0].x = m_cols[0].x * other.m_cols[0].x + m_cols[1].x * other.m_cols[0].y + m_cols[2].x * other.m_cols[0].z + m_cols[3].x * other.m_cols[0].w;
	retVal.m_cols[1].x = m_cols[0].x * other.m_cols[1].x + m_cols[1].x * other.m_cols[1].y + m_cols[2].x * other.m_cols[1].z + m_cols[3].x * other.m_cols[1].w;
	retVal.m_cols[2].x = m_cols[0].x * other.m_cols[2].x + m_cols[1].x * other.m_cols[2].y + m_cols[2].x * other.m_cols[2].z + m_cols[3].x * other.m_cols[2].w;
	retVal.m_cols[3].x = m_cols[0].x * other.m_cols[3].x + m_cols[1].x * other.m_cols[3].y + m_cols[2].x * other.m_cols[3].z + m_cols[3].x * other.m_cols[3].w;
																					 				   				   				 				 
	retVal.m_cols[0].y = m_cols[0].y * other.m_cols[0].x + m_cols[1].y * other.m_cols[0].y + m_cols[2].y * other.m_cols[0].z + m_cols[3].y * other.m_cols[0].w;
	retVal.m_cols[1].y = m_cols[0].y * other.m_cols[1].x + m_cols[1].y * other.m_cols[1].y + m_cols[2].y * other.m_cols[1].z + m_cols[3].y * other.m_cols[1].w;
	retVal.m_cols[2].y = m_cols[0].y * other.m_cols[2].x + m_cols[1].y * other.m_cols[2].y + m_cols[2].y * other.m_cols[2].z + m_cols[3].y * other.m_cols[2].w;
	retVal.m_cols[3].y = m_cols[0].y * other.m_cols[3].x + m_cols[1].y * other.m_cols[3].y + m_cols[2].y * other.m_cols[3].z + m_cols[3].y * other.m_cols[3].w;
																					 				   				   				 				 
	retVal.m_cols[0].z = m_cols[0].z * other.m_cols[0].x + m_cols[1].z * other.m_cols[0].y + m_cols[2].z * other.m_cols[0].z + m_cols[3].z * other.m_cols[0].w;
	retVal.m_cols[1].z = m_cols[0].z * other.m_cols[1].x + m_cols[1].z * other.m_cols[1].y + m_cols[2].z * other.m_cols[1].z + m_cols[3].z * other.m_cols[1].w;
	retVal.m_cols[2].z = m_cols[0].z * other.m_cols[2].x + m_cols[1].z * other.m_cols[2].y + m_cols[2].z * other.m_cols[2].z + m_cols[3].z * other.m_cols[2].w;
	retVal.m_cols[3].z = m_cols[0].z * other.m_cols[3].x + m_cols[1].z * other.m_cols[3].y + m_cols[2].z * other.m_cols[3].z + m_cols[3].z * other.m_cols[3].w;
																					 				   				   				 				 
	retVal.m_cols[0].w = m_cols[0].w * other.m_cols[0].x + m_cols[1].w * other.m_cols[0].y + m_cols[2].w * other.m_cols[0].z + m_cols[3].w * other.m_cols[0].w;
	retVal.m_cols[1].w = m_cols[0].w * other.m_cols[1].x + m_cols[1].w * other.m_cols[1].y + m_cols[2].w * other.m_cols[1].z + m_cols[3].w * other.m_cols[1].w;
	retVal.m_cols[2].w = m_cols[0].w * other.m_cols[2].x + m_cols[1].w * other.m_cols[2].y + m_cols[2].w * other.m_cols[2].z + m_cols[3].w * other.m_cols[2].w;
	retVal.m_cols[3].w = m_cols[0].w * other.m_cols[3].x + m_cols[1].w * other.m_cols[3].y + m_cols[2].w * other.m_cols[3].z + m_cols[3].w * other.m_cols[3].w;
	return retVal;
}

bbe::Vector3 bbe::Matrix4::operator*(const Vector3 & other) const
{
	Vector4 retVal(other, 1);
	retVal = operator*(retVal);
	return Vector3(retVal.x / retVal.w, retVal.y / retVal.w, retVal.z / retVal.w);
}

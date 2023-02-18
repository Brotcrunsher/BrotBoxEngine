#include "BBE/IcoSphere.h"
#include "BBE/Math.h"
#include "BBE/List.h"
#include <string.h>

bbe::IcoSphere::IcoSphere()
{
}

bbe::IcoSphere::IcoSphere(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	set(pos, scale, rotationVector, radians);
}

bbe::IcoSphere::IcoSphere(const Matrix4 & transform)
	: m_transform(transform)
{
}

void bbe::IcoSphere::set(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	Matrix4 matTranslation = Matrix4::createTranslationMatrix(pos);
	Matrix4 matScale = Matrix4::createScaleMatrix(scale);
	Matrix4 matRotation = Matrix4::createRotationMatrix(radians, rotationVector);

	m_transform = matTranslation * matRotation * matScale;
}

bbe::Vector3 bbe::IcoSphere::getPos() const
{
	return m_transform.extractTranslation();
}

float bbe::IcoSphere::getX() const
{
	return getPos().x;
}

float bbe::IcoSphere::getY() const
{
	return getPos().y;
}

float bbe::IcoSphere::getZ() const
{
	return getPos().z;
}

bbe::Vector3 bbe::IcoSphere::getScale() const
{
	return m_transform.extractScale();
}

float bbe::IcoSphere::getWidth() const
{
	return getScale().x;
}

float bbe::IcoSphere::getHeight() const
{
	return getScale().z;
}

float bbe::IcoSphere::getDepth() const
{
	return getScale().y;
}

bbe::Matrix4 bbe::IcoSphere::getTransform() const
{
	return m_transform;
}

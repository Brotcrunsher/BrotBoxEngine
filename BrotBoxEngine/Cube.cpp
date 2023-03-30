#include "BBE/Cube.h"
#include "BBE/List.h"
#include "BBE/Math.h"
#include "string.h"

bbe::Cube::Cube()
{
	//UNTESTED
}

bbe::Cube::Cube(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	//UNTESTED
	set(pos, scale, rotationVector, radians);
}

bbe::Cube::Cube(const Vector3& pos, const Vector3& scale, const Matrix4& matRotation)
{
	set(pos, scale, matRotation);
}

bbe::Cube::Cube(const Matrix4& matTranslation, const Matrix4& matScale, const Matrix4& matRotation)
{
	set(matTranslation, matScale, matRotation);
}

bbe::Cube::Cube(const Vector3& pos, const Matrix4& matScale, const Matrix4& matRotation)
{
	set(pos, matScale, matRotation);
}

bbe::Cube::Cube(const Matrix4 & transform)
	: m_transform(transform)
{
	//UNTESTED
}


void bbe::Cube::set(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	//UNTESTED
	const Matrix4 matTranslation = Matrix4::createTranslationMatrix(pos);
	const Matrix4 matScale = Matrix4::createScaleMatrix(scale);
	const Matrix4 matRotation = Matrix4::createRotationMatrix(radians, rotationVector);
	set(matTranslation, matScale, matRotation);
}

void bbe::Cube::set(const Vector3& pos, const Vector3& scale, const Matrix4& matRotation)
{
	const Matrix4 matTranslation = Matrix4::createTranslationMatrix(pos);
	const Matrix4 matScale = Matrix4::createScaleMatrix(scale);
	set(matTranslation, matScale, matRotation);
}

void bbe::Cube::set(const Matrix4& matTranslation, const Matrix4& matScale, const Matrix4& matRotation)
{
	m_transform = matTranslation * matRotation * matScale;
}

void bbe::Cube::set(const Vector3& pos, const Matrix4& matScale, const Matrix4& matRotation)
{
	set(Matrix4::createTranslationMatrix(pos), matScale, matRotation);
}

void bbe::Cube::setRotation(const Vector3 & rotationVector, float radians)
{
	const Matrix4 matTranslation = Matrix4::createTranslationMatrix(m_transform.extractTranslation());
	const Matrix4 matScale = Matrix4::createScaleMatrix(m_transform.extractScale());
	const Matrix4 matRotation = Matrix4::createRotationMatrix(radians, rotationVector);

	m_transform = matTranslation * matRotation * matScale;
}

void bbe::Cube::setPosition(const Vector3& pos)
{
	const Matrix4 matTranslation = Matrix4::createTranslationMatrix(pos);
	const Matrix4 matScale = Matrix4::createScaleMatrix(m_transform.extractScale());
	const Matrix4 matRotation = m_transform.extractRotation();
	set(matTranslation, matScale, matRotation);
}

void bbe::Cube::translate(const bbe::Vector3& translation)
{
	const Matrix4 matScale = Matrix4::createScaleMatrix(m_transform.extractScale());
	const Matrix4 matRotation = m_transform.extractRotation();
	set(getPos() + translation, matScale, matRotation);
}

bbe::Vector3 bbe::Cube::getPos() const
{
	//UNTESTED
	return m_transform.extractTranslation();
}

float bbe::Cube::getX() const
{
	//UNTESTED
	return getPos().x;
}

float bbe::Cube::getY() const
{
	//UNTESTED
	return getPos().y;
}

float bbe::Cube::getZ() const
{
	//UNTESTED
	return getPos().z;
}

bbe::Vector3 bbe::Cube::getCenter() const
{
	return getPos();
}

bbe::Vector3 bbe::Cube::getScale() const
{
	//UNTESTED
	return m_transform.extractScale();
}

float bbe::Cube::getWidth() const
{
	//UNTESTED
	return getScale().x;
}

float bbe::Cube::getHeight() const
{
	//UNTESTED
	return getScale().z;
}

float bbe::Cube::getDepth() const
{
	//UNTESTED
	return getScale().y;
}

const bbe::Matrix4& bbe::Cube::getTransform() const
{
	//UNTESTED
	return m_transform;
}

bbe::List<bbe::Vector3> bbe::Cube::getNormals() const
{
	bbe::List<bbe::Vector3> retVal;
	retVal.resizeCapacityAndLength(6);

	const bbe::Matrix4 rotationMatrix = m_transform.extractRotation();
	retVal[0] = rotationMatrix * bbe::Vector3(1, 0, 0);
	retVal[1] = -retVal[0];
	retVal[2] = rotationMatrix * bbe::Vector3(0, 1, 0);
	retVal[3] = -retVal[2];
	retVal[4] = rotationMatrix * bbe::Vector3(0, 0, 1);
	retVal[5] = -retVal[4];

	return retVal;
}

void bbe::Cube::getVertices(bbe::List<bbe::Vector3>& outVertices) const
{
	outVertices.clear();

	outVertices.add(m_transform * bbe::Vector3(+0.5f, +0.5f, +0.5f));
	outVertices.add(m_transform * bbe::Vector3(+0.5f, +0.5f, -0.5f));
	outVertices.add(m_transform * bbe::Vector3(+0.5f, -0.5f, +0.5f));
	outVertices.add(m_transform * bbe::Vector3(+0.5f, -0.5f, -0.5f));
	outVertices.add(m_transform * bbe::Vector3(-0.5f, +0.5f, +0.5f));
	outVertices.add(m_transform * bbe::Vector3(-0.5f, +0.5f, -0.5f));
	outVertices.add(m_transform * bbe::Vector3(-0.5f, -0.5f, +0.5f));
	outVertices.add(m_transform * bbe::Vector3(-0.5f, -0.5f, -0.5f));
}

bbe::Vector2 normalPos(int32_t face, int32_t sub, bbe::Vector2& maxPos)
{
	const bbe::Vector2i basePosi = bbe::Math::squareCantor(face);
	const bbe::Vector2 basePos = bbe::Vector2(basePosi.x * 1.00f, basePosi.y * 1.00f);

	bbe::Vector2 retVal;
	switch (sub)
	{
	case(0): retVal = basePos + bbe::Vector2(1.0f, 0.0f); break;
	case(1): retVal = basePos + bbe::Vector2(1.0f, 1.0f); break;
	case(2): retVal = basePos + bbe::Vector2(0.0f, 1.0f); break;
	case(3): retVal = basePos + bbe::Vector2(0.0f, 0.0f); break;
	default: throw bbe::IllegalArgumentException();
	}

	maxPos.x = bbe::Math::max(maxPos.x, retVal.x);
	maxPos.y = bbe::Math::max(maxPos.y, retVal.y);
	
	return retVal;
}

bbe::List<bbe::PosNormalPair> bbe::Cube::getRenderVerticesDefault(FaceFlag ff)
{
	bbe::List<bbe::PosNormalPair> retVal;
	int32_t face = 0;
	bbe::Vector2 maxPos;
	if ((int)ff & (int)FaceFlag::BOTTOM)
	{
		retVal.addAll(
			PosNormalPair{ Vector3( 0.5, -0.5, -0.5), Vector3(0, 0, -1), normalPos(face, 0, maxPos) },
			PosNormalPair{ Vector3( 0.5,  0.5, -0.5), Vector3(0, 0, -1), normalPos(face, 1, maxPos) },
			PosNormalPair{ Vector3(-0.5,  0.5, -0.5), Vector3(0, 0, -1), normalPos(face, 2, maxPos) },
			PosNormalPair{ Vector3(-0.5, -0.5, -0.5), Vector3(0, 0, -1), normalPos(face, 3, maxPos) });
		face++;
	}
	if ((int)ff & (int)FaceFlag::TOP)
	{
		retVal.addAll(
			PosNormalPair{ Vector3( 0.5, -0.5,  0.5), Vector3(0, 0,  1), normalPos(face, 0, maxPos) },
			PosNormalPair{ Vector3( 0.5,  0.5,  0.5), Vector3(0, 0,  1), normalPos(face, 1, maxPos) },
			PosNormalPair{ Vector3(-0.5,  0.5,  0.5), Vector3(0, 0,  1), normalPos(face, 2, maxPos) },
			PosNormalPair{ Vector3(-0.5, -0.5,  0.5), Vector3(0, 0,  1), normalPos(face, 3, maxPos) });
		face++;
	}
	if ((int)ff & (int)FaceFlag::LEFT)
	{
		retVal.addAll(
			PosNormalPair{ Vector3( 0.5, -0.5, -0.5), Vector3(0, -1, 0), normalPos(face, 0, maxPos) },
			PosNormalPair{ Vector3( 0.5, -0.5,  0.5), Vector3(0, -1, 0), normalPos(face, 1, maxPos) },
			PosNormalPair{ Vector3(-0.5, -0.5,  0.5), Vector3(0, -1, 0), normalPos(face, 2, maxPos) },
			PosNormalPair{ Vector3(-0.5, -0.5, -0.5), Vector3(0, -1, 0), normalPos(face, 3, maxPos) });
		face++;
	}
	if ((int)ff & (int)FaceFlag::RIGHT)
	{
		retVal.addAll(
			PosNormalPair{ Vector3( 0.5,  0.5, -0.5), Vector3(0,  1, 0), normalPos(face, 0, maxPos) },
			PosNormalPair{ Vector3( 0.5,  0.5,  0.5), Vector3(0,  1, 0), normalPos(face, 1, maxPos) },
			PosNormalPair{ Vector3(-0.5,  0.5,  0.5), Vector3(0,  1, 0), normalPos(face, 2, maxPos) },
			PosNormalPair{ Vector3(-0.5,  0.5, -0.5), Vector3(0,  1, 0), normalPos(face, 3, maxPos) });
		face++;
	}
	if ((int)ff & (int)FaceFlag::FRONT)
	{
		retVal.addAll(
			PosNormalPair{ Vector3(-0.5,  0.5, -0.5), Vector3(-1, 0, 0), normalPos(face, 0, maxPos) },
			PosNormalPair{ Vector3(-0.5,  0.5,  0.5), Vector3(-1, 0, 0), normalPos(face, 1, maxPos) },
			PosNormalPair{ Vector3(-0.5, -0.5,  0.5), Vector3(-1, 0, 0), normalPos(face, 2, maxPos) },
			PosNormalPair{ Vector3(-0.5, -0.5, -0.5), Vector3(-1, 0, 0), normalPos(face, 3, maxPos) });
		face++;
	}
	if ((int)ff & (int)FaceFlag::BACK)
	{
		retVal.addAll(
			PosNormalPair{ Vector3(0.5,  0.5, -0.5), Vector3(1, 0, 0), normalPos(face, 0, maxPos) },
			PosNormalPair{ Vector3(0.5,  0.5,  0.5), Vector3(1, 0, 0), normalPos(face, 1, maxPos) },
			PosNormalPair{ Vector3(0.5, -0.5,  0.5), Vector3(1, 0, 0), normalPos(face, 2, maxPos) },
			PosNormalPair{ Vector3(0.5, -0.5, -0.5), Vector3(1, 0, 0), normalPos(face, 3, maxPos) });
		face++;
	}

	for (PosNormalPair& pnp : retVal)
	{
		pnp.uvCoord = pnp.uvCoord / maxPos;
	}

	return retVal;
}

bbe::List<bbe::PosNormalPair> bbe::Cube::getRenderVertices(FaceFlag ff) const
{
	bbe::List<bbe::PosNormalPair> retVal = getRenderVerticesDefault(ff);
	bbe::PosNormalPair::transform(retVal, m_transform);
	return retVal;

}

bbe::List<uint32_t> bbe::Cube::getRenderIndicesDefault(FaceFlag ff)
{
	bbe::List<uint32_t> retVal;
	int32_t offset = 0;
	if ((int)ff & (int)FaceFlag::BOTTOM)
	{
		retVal.addAll(
			0, 1, 3,	//Bottom
			1, 2, 3);
	} else offset += 4;
	if ((int)ff & (int)FaceFlag::TOP)
	{
		retVal.addAll(
			5 - offset, 4 - offset, 7 - offset,	//Top
			6 - offset, 5 - offset, 7 - offset);
	} else offset += 4;
	if ((int)ff & (int)FaceFlag::LEFT)
	{
		retVal.addAll(
			9 - offset, 8 - offset, 11 - offset,	//Left
			10 - offset, 9 - offset, 11 - offset);
	} else offset += 4;
	if ((int)ff & (int)FaceFlag::RIGHT)
	{
		retVal.addAll(
			12 - offset, 13 - offset, 15 - offset,	//Right
			13 - offset, 14 - offset, 15 - offset);
	} else offset += 4;
	if ((int)ff & (int)FaceFlag::FRONT)
	{
		retVal.addAll(
			16 - offset, 17 - offset, 19 - offset,	//Front
			17 - offset, 18 - offset, 19 - offset);
	} else offset += 4;
	if ((int)ff & (int)FaceFlag::BACK)
	{
		retVal.addAll(
			21 - offset, 20 - offset, 23 - offset,	//Back
			22 - offset, 21 - offset, 23 - offset);
	} else offset += 4;

	return retVal;
}

bbe::Vector3 bbe::Cube::approach(const bbe::Cube& other, const bbe::Vector3& approachVector) const
{
	//TODO This implementation is very, very naive. It can be implemented in a much faster
	// 	   and robust way.
	//     See: https://www.codeproject.com/Articles/15573/2D-Polygon-Collision-Detection

	// If we already intersect then we can't approach any further.
	if (intersects(other)) return bbe::Vector3(0, 0, 0);

	bbe::Vector3 pos = getPos();
	bbe::Matrix4 scale = bbe::Matrix4::createScaleMatrix(getScale());
	bbe::Matrix4 rotation = m_transform.extractRotation();

	// If we don't collide after the approach then we return the full approachVector.
	// TODO: This is sensitive for bullet through paper!
	if (!other.intersects(bbe::Cube(pos + approachVector, scale, rotation))) return approachVector;

	float upperBound = 1;
	float lowerBound = 0;
	// 40 iterations are probably enough...
	// In tests the 40 iterations were never reached (halfWay break happened)
	for (int i = 0; i < 40; i++)
	{
		float halfWay = (upperBound + lowerBound) * 0.5f;
		if (halfWay == upperBound || halfWay == lowerBound) break;
		if (other.intersects(bbe::Cube(pos + approachVector * halfWay, scale, rotation)))
		{
			upperBound = halfWay;
		}
		else
		{
			lowerBound = halfWay;
		}
	}

	return approachVector * lowerBound;
}

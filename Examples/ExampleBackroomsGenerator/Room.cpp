#include "Room.h"

template <typename T>
static void addToListIfNotAlreadyInIt(bbe::List<T>& list, const T& val)
{
	if (!list.contains(val))
	{
		list.add(val);
	}
}

constexpr int32_t gridSize = 32;
bbe::List<bbe::Vector2i> br::Room::getHashGridPositions() const
{
	return getHashGridPositions(boundingBox);
}

bbe::List<bbe::Vector2i> br::Room::getHashGridPositions(const bbe::Rectanglei& rect)
{
	bbe::List<bbe::Vector2i> retVal;
	for (int32_t x = rect.x; x < (rect.x + rect.width); x += gridSize)
	{
		const int32_t gridX = x / gridSize;
		for (int32_t y = rect.y; y < (rect.y + rect.height); y += gridSize)
		{
			const int32_t gridY = y / gridSize;
			addToListIfNotAlreadyInIt(retVal, { gridX, gridY });
		}
	}

	for (int32_t x = rect.x; x < (rect.x + rect.width); x += gridSize)
	{
		const int32_t gridX = x / gridSize;
		const int32_t gridY = (rect.y + rect.height - 1) / gridSize;
		addToListIfNotAlreadyInIt(retVal, { gridX, gridY });
	}
	for (int32_t y = rect.y; y < (rect.y + rect.height); y += gridSize)
	{
		const int32_t gridX = (rect.x + rect.width - 1) / gridSize;
		const int32_t gridY = y / gridSize;
		addToListIfNotAlreadyInIt(retVal, { gridX, gridY });
	}

	addToListIfNotAlreadyInIt(retVal, { (rect.x + rect.width - 1) / gridSize, (rect.y + rect.height - 1) / gridSize });
	
	return retVal;
}

bbe::Vector2i br::Room::getHashGridPosition(const bbe::Vector2i& pos)
{
	return pos / gridSize;
}

bbe::Vector3 br::Room::getBoundingCubePos() const
{
	return bbe::Vector3 {
		(float)boundingBox.x + (float)boundingBox.width * 0.5f,
		(float)boundingBox.y + (float)boundingBox.height * 0.5f,
		1.25f
	};
}

bbe::Vector3 br::Room::getBoundingCubeScale() const
{
	return bbe::Vector3 {
		(float)boundingBox.width,
		(float)boundingBox.height,
		2.5f
	};
}

// Why inner and outer Bounding Box? We need an outer Bounding box to make occlusion queries that hit rooms of which only a wall is visible. If it would
// have the same size, then it would be Z-Fighting with the bounding box. However, this slightly smaller bounding box will then suddenly be no longer visible
// for a short time when traversing a gate. For these times, the inner bounding box is necessary.
bbe::Cube br::Room::getBoundingCubeOuter() const
{
	return bbe::Cube(getBoundingCubePos(), getBoundingCubeScale() + bbe::Vector3(0.01f, 0.01f, 0.f));
}
bbe::Cube br::Room::getBoundingCubeInner() const
{
	return bbe::Cube(getBoundingCubePos(), getBoundingCubeScale());
}

bbe::Matrix4 br::Room::floorMatrix() const
{
	const bbe::Rectanglei& bounding = boundingBox;
	bbe::Matrix4 scale = bbe::Matrix4::createScaleMatrix({ (float)bounding.width, (float)bounding.height, 1 });
	return scale;
}

bbe::Matrix4 br::Room::ceilingMatrix() const
{
	const bbe::Rectanglei& bounding = boundingBox;
	bbe::Vector3 translationVec = bbe::Vector3(bounding.x + bounding.width / 2.f, bounding.y + bounding.height / 2.f, 0);
	translationVec.z = 2.5f;
	bbe::Matrix4 translation = bbe::Matrix4::createTranslationMatrix(translationVec);
	bbe::Matrix4 scale = bbe::Matrix4::createScaleMatrix({ (float)bounding.width, (float)bounding.height, 1 });
	bbe::Matrix4 rotationMat = bbe::Matrix4::createRotationMatrix(bbe::Math::PI, bbe::Vector3(1, 0, 0));
	return rotationMat * scale;
}

bbe::Matrix4 br::Room::floorTranslation() const
{
	const bbe::Rectanglei& bounding = boundingBox;
	bbe::Vector3 translationVec = bbe::Vector3(bounding.x + bounding.width / 2.f, bounding.y + bounding.height / 2.f, 0);
	bbe::Matrix4 translation = bbe::Matrix4::createTranslationMatrix(translationVec);
	return translation;
}

bbe::Matrix4 br::Room::ceilingTranslation() const
{
	const bbe::Rectanglei& bounding = boundingBox;
	bbe::Vector3 translationVec = bbe::Vector3(bounding.x + bounding.width / 2.f, bounding.y + bounding.height / 2.f, 0);
	translationVec.z = 2.5f;
	bbe::Matrix4 translation = bbe::Matrix4::createTranslationMatrix(translationVec);
	return translation;
}

bbe::Color br::Room::getColor() const
{
	return bbe::Color::HSVtoRGB(hue, saturation, value);
}

bool br::Gate::operator==(const Gate& other) const
{
	return this->ownGatePos      == other.ownGatePos 
		&& this->neighborGatePos == other.neighborGatePos;
}

br::Gate br::Gate::flipped() const
{
	Gate retVal;

	retVal.ownGatePos = neighborGatePos;
	retVal.neighborGatePos = ownGatePos;

	return retVal;
}

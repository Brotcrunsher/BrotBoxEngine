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
	for (int32_t x = rect.getX(); x < (rect.getX() + rect.getWidth()); x += gridSize)
	{
		const int32_t gridX = x / gridSize;
		for (int32_t y = rect.getY(); y < (rect.getY() + rect.getHeight()); y += gridSize)
		{
			const int32_t gridY = y / gridSize;
			addToListIfNotAlreadyInIt(retVal, { gridX, gridY });
		}
	}

	for (int32_t x = rect.getX(); x < (rect.getX() + rect.getWidth()); x += gridSize)
	{
		const int32_t gridX = x / gridSize;
		const int32_t gridY = (rect.getY() + rect.getHeight() - 1) / gridSize;
		addToListIfNotAlreadyInIt(retVal, { gridX, gridY });
	}
	for (int32_t y = rect.getY(); y < (rect.getY() + rect.getHeight()); y += gridSize)
	{
		const int32_t gridX = (rect.getX() + rect.getWidth() - 1) / gridSize;
		const int32_t gridY = y / gridSize;
		addToListIfNotAlreadyInIt(retVal, { gridX, gridY });
	}

	addToListIfNotAlreadyInIt(retVal, { (rect.getX() + rect.getWidth() - 1) / gridSize, (rect.getY() + rect.getHeight() - 1) / gridSize });
	
	return retVal;
}

bbe::Vector2i br::Room::getHashGridPosition(const bbe::Vector2i& pos)
{
	return pos / gridSize;
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

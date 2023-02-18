#include "Rooms.h"
#include "BBE/Random.h"

bbe::Rectanglei br::Rooms::shrinkBoundingBoxRec(const bbe::Rectanglei& bounding, const bbe::List<bbe::Rectanglei>& intersections, int32_t index, int32_t& currentBestArea) const
{
	if (index == intersections.getLength()) return bounding;
	
	if (!intersections[index].intersects(bounding)) return shrinkBoundingBoxRec(bounding, intersections, index + 1, currentBestArea);

	bbe::List<bbe::Rectanglei> candidates;
	if (bounding.getBottom() > intersections[index].getTop())
	{
		int32_t sub = bounding.getBottom() - intersections[index].getTop();
		if (sub < bounding.getHeight())
		{
			bbe::Rectanglei top = bounding;
			top.setHeight(top.getHeight() - sub);
			if(top.getArea() > currentBestArea)
				candidates.add(shrinkBoundingBoxRec(top, intersections, index + 1, currentBestArea));
		}
	}
	if (intersections[index].getBottom() > bounding.getTop())
	{
		int32_t sub = intersections[index].getBottom() - bounding.getTop();
		if (sub < bounding.getHeight())
		{
			bbe::Rectanglei bottom = bounding;
			bottom.setHeight(bottom.getHeight() - sub);
			bottom.setY(bottom.getY() + sub);
			if (bottom.getArea() > currentBestArea)
				candidates.add(shrinkBoundingBoxRec(bottom, intersections, index + 1, currentBestArea));
		}
	}
	if (bounding.getRight() > intersections[index].getLeft())
	{
		int32_t sub = bounding.getRight() - intersections[index].getLeft();
		if (sub < bounding.getWidth())
		{
			bbe::Rectanglei left = bounding;
			left.setWidth(left.getWidth() - sub);
			if (left.getArea() > currentBestArea)
				candidates.add(shrinkBoundingBoxRec(left, intersections, index + 1, currentBestArea));
		}
	}
	if (intersections[index].getRight() > bounding.getLeft())
	{
		int32_t sub = intersections[index].getRight() - bounding.getLeft();
		if (sub < bounding.getWidth())
		{
			bbe::Rectanglei right = bounding;
			right.setWidth(right.getWidth() - sub);
			right.setX(right.getX() + sub);
			if (right.getArea() > currentBestArea)
				candidates.add(shrinkBoundingBoxRec(right, intersections, index + 1, currentBestArea));
		}
	}

	if (candidates.getLength() == 0)
	{
		return bbe::Rectanglei(0, 0, 0, 0);
	}

	// Find the candidate with the maximum area.
	bbe::Rectanglei* retVal = &candidates[0];
	int32_t maxArea = candidates[0].getArea();
	for (size_t i = 1; i < candidates.getLength(); i++)
	{
		int32_t area = candidates[i].getArea();
		if (area > maxArea)
		{
			retVal = &candidates[i];
			maxArea = area;
		}
	}

	if (maxArea > currentBestArea)
	{
		currentBestArea = maxArea;
	}
	return *retVal;
}


br::Rooms::Rooms()
{
}

void br::Rooms::clear()
{
	rooms.clear();
	hashGrid.clear();
}

void br::Rooms::setSeed(int seed)
{
	rand.setSeed(seed);
}

bbe::Rectanglei br::Rooms::newBoundingAt(const bbe::Vector2i& position)
{
	int32_t width  = 10;
	int32_t height = 10;
	bool bigRoom = rand.randomFloat() > 0.99f;
	do
	{
		// TODO: Increase bigRooms - endless loop! Wrong hashes?
		width  += rand.randomInt(bigRoom ? 30 : 10);
		height += rand.randomInt(bigRoom ? 30 : 10);
	} while (rand.randomBool());
	int32_t x = position.x;
	int32_t y = position.y;
	return bbe::Rectanglei(x, y, width, height);
}

size_t br::Rooms::lookupRoomIndex(const bbe::Vector2i& position)
{
	int32_t index = getRoomIndexAtPoint(position);
	if (index >= 0)
	{
		return index;
	}

	bbe::Rectanglei bounding = newBoundingAt(position);
	bounding = shrinkBoundingBox(bounding);
	
	addRoom(bounding);
	return rooms.getLength() - 1;
}

bbe::Rectanglei br::Rooms::shrinkBoundingBox(const bbe::Rectanglei& bounding) const
{
	bbe::List<bbe::Rectanglei> intersections;
	bbe::List<bbe::Vector2i> hashGridPositions = Room::getHashGridPositions(bounding);
	for (const bbe::Vector2i& hgp : hashGridPositions)
	{
		const bbe::List<size_t>* candidates = hashGrid.get(hgp);
		if (candidates)
		{
			for (size_t roomi : *candidates)
			{
				if (rooms[roomi].boundingBox.intersects(bounding))
				{
					intersections.add(rooms[roomi].boundingBox);
				}
			}
		}
	}
	bbe::Rectanglei retVal = bounding;
	if (intersections.getLength() != 0)
	{
		if (intersections.getLength() < 4)
		{
			// Fast for small amount of intersections, but has O(2^N), so get's extremely bad for bigger N.
			int32_t currentBestArea = 0;
			retVal = shrinkBoundingBoxRec(bounding, intersections, 0, currentBestArea);
		}
		else
		{
			bbe::Grid<bool> intersectionGrid(bounding.getWidth(), bounding.getHeight());
			for (int32_t i = 0; i < bounding.getWidth(); i++)
			{
				for (int32_t k = 0; k < bounding.getHeight(); k++)
				{
					bbe::Vector2i pos(i + bounding.getX(), k + bounding.getY());
					bool intersects = false;
					for (const bbe::Rectanglei& rect : intersections)
					{
						if (rect.isPointInRectangle(pos, true))
						{
							intersects = true;
							break;
						}
					}
					intersectionGrid[i][k] = intersects;
				}
			}
			bbe::Rectanglei biggestRect = intersectionGrid.getBiggestRect(false);

			retVal = bbe::Rectanglei(biggestRect.getX() + bounding.getX(), biggestRect.getY() + bounding.getY(), biggestRect.getWidth(), biggestRect.getHeight());
		}
	}
	
	if (retVal.getArea() == 0)
	{
		throw bbe::IllegalStateException();
	}

	return retVal;
}

bool br::Rooms::expandRoom(size_t roomi)
{
	if (rooms[roomi].state != RoomGenerationState::outlines) return true;
	rooms[roomi].state = RoomGenerationState::expanded;

	const bbe::Rectanglei* b = &rooms[roomi].boundingBox;
	
	bbe::List<bbe::Vector2i> toFillPoints;
	toFillPoints.resizeCapacity(b->getWidth() * 2 + b->getHeight() * 2);

	for (int32_t i = 0; i < b->getWidth(); i++)
	{
		toFillPoints.add(bbe::Vector2i(b->getX() + i, b->getY()                  - 1));
		toFillPoints.add(bbe::Vector2i(b->getX() + i, b->getY() + b->getHeight()    ));
	}
	for (int32_t i = 0; i < b->getHeight(); i++)
	{
		toFillPoints.add(bbe::Vector2i(b->getX()                 - 1, b->getY() + i));
		toFillPoints.add(bbe::Vector2i(b->getX() + b->getWidth()    , b->getY() + i));
	}

	for (size_t i = 0; i < toFillPoints.getLength(); i++)
	{
		const bbe::Vector2i& point = toFillPoints[i];
		if (point.x == 9 && point.y == -10)
		{
			int a = 0;
		}
		const int32_t foundRoomIndex = getRoomIndexAtPoint(point, roomi);
		if (foundRoomIndex >= 0)
		{
			toFillPoints.removeIndex(i);
			i--;
		}
	}

	int debugIteration = 0;
	while (toFillPoints.getLength() > 0)
	{
		debugIteration++;
		if (debugIteration > 100000)
		{
			int a = 0;
			return false;
		}
		const size_t randomIndex = rand.randomInt(toFillPoints.getLength());
		const bbe::Vector2i& pos = toFillPoints[randomIndex];
		bbe::Rectanglei newBounding = newBoundingAt(pos);
		newBounding.setX(newBounding.getX() - rand.randomInt(newBounding.getWidth()));
		newBounding.setY(newBounding.getY() - rand.randomInt(newBounding.getHeight()));
		if (pos.x == b->getX() - 1) // Left Edge
		{
			newBounding.setX(pos.x - newBounding.getWidth() + 1);
		}
		else if (pos.x == b->getX() + b->getWidth()) // Right Edge
		{
			newBounding.setX(b->getX() + b->getWidth());
		}
		else if (pos.y == b->getY() - 1) // Top Edge
		{
			newBounding.setY(pos.y - newBounding.getHeight() + 1);
		}
		else if (pos.y == b->getY() + b->getHeight()) // Bottom Edge
		{
			newBounding.setY(b->getY() + b->getHeight());
		}
		else // ???
		{
			throw bbe::IllegalStateException();
		}

		try
		{
			newBounding = shrinkBoundingBox(newBounding);
		}
		catch (const bbe::IllegalStateException& e)
		{
			return false;
		}

		for (size_t i = 0; i < toFillPoints.getLength(); i++)
		{
			if (newBounding.isPointInRectangle(toFillPoints[i], true))
			{
				toFillPoints.removeIndex(i);
				i--;
			}
		}

		addRoom(newBounding);
		b = &rooms[roomi].boundingBox;
	}

	return true;
}

void br::Rooms::determineNeighbors_(size_t roomi, const bbe::Vector2i& roomiGatePos, const bbe::Vector2i& neighborGatePos)
{
	int32_t neighborIndex = getRoomIndexAtPoint(neighborGatePos);
	if (neighborIndex == roomi)
	{
		// A room is its own neighbor? Can't happen.
		throw bbe::IllegalStateException();
	}
	if (neighborIndex < 0)
	{
		// Hole in the rooms?
		throw bbe::IllegalStateException();
	}

	if (!rooms[roomi].neighbors.contains([&](const br::Neighbor& n) { return neighborIndex == n.neighborId; }))
	{
		Neighbor n;
		n.neighborId = neighborIndex;
		rooms[roomi].neighbors.add(n);
		n.neighborId = roomi;
		rooms[neighborIndex].neighbors.add(n);
	}

	Gate gate;
	gate.ownGatePos = roomiGatePos;
	gate.neighborGatePos = neighborGatePos;

	rooms[roomi]        .neighbors.find([&](const br::Neighbor& n) { return neighborIndex == n.neighborId; })->gates.addUnique(gate);
	rooms[neighborIndex].neighbors.find([&](const br::Neighbor& n) { return roomi         == n.neighborId; })->gates.addUnique(gate.flipped());
}

void br::Rooms::determineNeighbors(size_t roomi)
{
	if (rooms[roomi].state == RoomGenerationState::outlines) expandRoom(roomi);
	if (rooms[roomi].state != RoomGenerationState::expanded) return;
	rooms[roomi].state = RoomGenerationState::neighborsDetermined;

	const Room& room = rooms[roomi];
	const bbe::Rectanglei& rect = room.boundingBox;

	for (int32_t i = rect.getLeft(); i < rect.getRight(); i++)
	{
		bbe::Vector2i roomGatePos(i, rect.getTop());
		bbe::Vector2i neighborGatePos(i, rect.getTop() - 1);
		determineNeighbors_(roomi, roomGatePos, neighborGatePos);
	}
	for (int32_t i = rect.getLeft(); i < rect.getRight(); i++)
	{
		bbe::Vector2i roomGatePos(i, rect.getTop() + rect.getHeight() - 1);
		bbe::Vector2i neighborGatePos(i, rect.getTop() + rect.getHeight());
		determineNeighbors_(roomi, roomGatePos, neighborGatePos);
	}

	for (int32_t i = rect.getTop(); i < rect.getBottom(); i++)
	{
		bbe::Vector2i roomGatePos(rect.getLeft(), i);
		bbe::Vector2i neighborGatePos(rect.getLeft() - 1, i);
		determineNeighbors_(roomi, roomGatePos, neighborGatePos);
	}
	for (int32_t i = rect.getTop(); i < rect.getBottom(); i++)
	{
		bbe::Vector2i roomGatePos(rect.getLeft() + rect.getWidth() - 1, i);
		bbe::Vector2i neighborGatePos(rect.getLeft() + rect.getWidth(), i);
		determineNeighbors_(roomi, roomGatePos, neighborGatePos);
	}
}

void br::Rooms::collapseGates(size_t roomi)
{
	if (rooms[roomi].state < RoomGenerationState::neighborsDetermined) determineNeighbors(roomi);
	if (rooms[roomi].state != RoomGenerationState::neighborsDetermined) return;
	rooms[roomi].state = RoomGenerationState::gatesCollapsed;

	// First make all neighbors find their neighbors so that we can be sure that their gate list isn't updated anymore.
	for (Neighbor& n : rooms[roomi].neighbors)
	{
		determineNeighbors(n.neighborId);
	}

	for (Neighbor& n : rooms[roomi].neighbors)
	{
		uint32_t gateToKeep = rand.randomInt(n.gates.getLength());
		Gate keeper = n.gates[gateToKeep];
		n.gates.clear();
		n.gates.add(keeper);
		
		Neighbor* myself = rooms[n.neighborId].neighbors.find([&](const Neighbor& n) { return n.neighborId == roomi; });
		myself->gates.clear();
		myself->gates.add(keeper.flipped());
	}
}

void br::Rooms::connectGates(size_t roomi)
{
	if (rooms[roomi].state < RoomGenerationState::gatesCollapsed) collapseGates(roomi);
	if (rooms[roomi].state != RoomGenerationState::gatesCollapsed) return;
	rooms[roomi].state = RoomGenerationState::gatesConnected;
	Room& r = rooms[roomi];

	if (r.neighbors.getLength() < 2)
	{
		// To connect rooms, we need at least two neighbors.
		throw bbe::IllegalStateException();
	}

	
	bbe::Grid<int32_t> pathIdGrid(r.boundingBox.getDim());
	pathIdGrid.setAll(-1);

	bbe::List<int32_t> gatesConnected;
	gatesConnected.resizeCapacityAndLength(r.neighbors.getLength());
	for (size_t i = 0; i < gatesConnected.getLength(); i++) gatesConnected[i] = 1;

	bbe::List<bbe::Vector2i> walkableTiles;

	bool skipLoop = false;
	for (size_t i = 0; i < r.neighbors.getLength(); i++)
	{
		if (r.neighbors[i].gates.getLength() != 1)
		{
			// Gates should have collapsed by now.
			throw bbe::IllegalStateException();
		}
		bbe::Vector2i gateWorldPos = r.neighbors[i].gates[0].ownGatePos;
		bbe::Vector2i gateLocalPos = gateWorldPos - r.boundingBox.getPos();
		if (pathIdGrid[gateLocalPos] != -1)
		{
			gatesConnected[pathIdGrid[gateLocalPos]]++;
			if (gatesConnected[pathIdGrid[gateLocalPos]] == r.neighbors.getLength())
			{
				skipLoop = true;
			}
		}
		else
		{
			pathIdGrid[gateLocalPos] = i;
			walkableTiles.add(gateLocalPos);
		}
	}

	while (!skipLoop)
	{
		const bbe::Vector2i p1 = walkableTiles[rand.randomInt(walkableTiles.getLength())];
		const bbe::Vector2i p2 = walkableTiles[rand.randomInt(walkableTiles.getLength())];
		if (pathIdGrid[p1] != pathIdGrid[p2])
		{
			bbe::Vector2i movingP1 = p1;
			bbe::Vector2i dir = p2 - p1;
			bbe::List<bbe::Vector2i> newPath;
			bool validPath = true;
			while (movingP1 != p2)
			{
				int32_t xOrY = rand.randomInt(bbe::Math::abs(dir.x) + bbe::Math::abs(dir.y));
				bool isX = xOrY < bbe::Math::abs(dir.x);
				if (isX)
				{
					if (dir.x < 0)
					{
						movingP1.x--;
						dir.x++;
					}
					else
					{
						movingP1.x++;
						dir.x--;
					}
				}
				else
				{
					if (dir.y < 0)
					{
						movingP1.y--;
						dir.y++;
					}
					else
					{
						movingP1.y++;
						dir.y--;
					}
				}
				if (pathIdGrid[p1] != pathIdGrid[movingP1] && pathIdGrid[movingP1] != -1)
				{
					validPath = true;
					break;
				}
				if (pathIdGrid[p1] == pathIdGrid[movingP1])
				{
					validPath = false;
					break;
				}
				newPath.add(movingP1);
			}

			if (validPath)
			{
				gatesConnected[pathIdGrid[p1]] += gatesConnected[pathIdGrid[movingP1]];
				pathIdGrid.floodFill(movingP1, pathIdGrid[p1], false);
				for (const bbe::Vector2i& step : newPath)
				{
					walkableTiles.add(step);
					pathIdGrid[step] = pathIdGrid[p1];
				}
				if (gatesConnected[pathIdGrid[p1]] >= r.neighbors.getLength())
				{
					break;
				}
			}
		}
	}

	r.walkable = bbe::Grid<bool>(r.boundingBox.getDim());
	for (int32_t x = 0; x < r.boundingBox.getWidth(); x++)
	{
		for (int32_t y = 0; y < r.boundingBox.getHeight(); y++)
		{
			r.walkable[x][y] = pathIdGrid[x][y] != -1;
		}
	}
}

void br::Rooms::generateAtPoint(const bbe::Vector2i& position)
{
	size_t roomi = lookupRoomIndex(position);
	expandRoom(roomi);
}

int32_t br::Rooms::getRoomIndexAtPoint(const bbe::Vector2i& position, int32_t ignore_room) const
{
	const bbe::Vector2i gridPos = Room::getHashGridPosition(position);
	const bbe::List<size_t>* indizes = hashGrid.get(gridPos);
	if (!indizes)
	{
		return -1;
	}

	for (size_t i = 0; i < indizes->getLength(); i++)
	{
		size_t val = (*indizes)[i];
		if (val != ignore_room && rooms[val].boundingBox.isPointInRectangle(position, true))
		{
			return val;
		}
	}

	return -1;
}

void br::Rooms::addRoom(const bbe::Rectanglei& bounding)
{
	Room room;
	room.boundingBox = bounding;
	room.hue = rand.randomFloat() * 360;
	room.value = rand.randomFloat() / 2.0f + 0.5f;
	room.saturation = rand.randomFloat() / 2.0f + 0.5f;
	room.id = rooms.getLength();

	rooms.add(room);

	bbe::List<bbe::Vector2i> gridPos = room.getHashGridPositions();
	
	const size_t roomId = rooms.getLength() - 1;

	for (size_t i = 0; i < gridPos.getLength(); i++)
	{
		if (hashGrid.contains(gridPos[i]))
		{
			hashGrid.get(gridPos[i])->add(roomId);
		}
		else
		{
			bbe::List<size_t> addVal;
			addVal.add(roomId);
			hashGrid.add(gridPos[i], addVal);
		}
	}
}

#include "Rooms.h"
#include "BBE/Random.h"
#include "BBE/MeshBuilder.h"

bbe::Rectanglei br::Rooms::shrinkBoundingBoxRec(const bbe::Rectanglei& bounding, const bbe::List<bbe::Rectanglei>& intersections, int32_t index, int32_t& currentBestArea) const
{
	if (index == intersections.getLength()) return bounding;
	
	if (!intersections[index].intersects(bounding)) return shrinkBoundingBoxRec(bounding, intersections, index + 1, currentBestArea);

	bbe::List<bbe::Rectanglei> candidates;
	if (bounding.getBottom() > intersections[index].getTop())
	{
		int32_t sub = bounding.getBottom() - intersections[index].getTop();
		if (sub < bounding.height)
		{
			bbe::Rectanglei top = bounding;
			top.height = top.height - sub;
			if(top.getArea() > currentBestArea)
				candidates.add(shrinkBoundingBoxRec(top, intersections, index + 1, currentBestArea));
		}
	}
	if (intersections[index].getBottom() > bounding.getTop())
	{
		int32_t sub = intersections[index].getBottom() - bounding.getTop();
		if (sub < bounding.height)
		{
			bbe::Rectanglei bottom = bounding;
			bottom.height = bottom.height - sub;
			bottom.y = bottom.y + sub;
			if (bottom.getArea() > currentBestArea)
				candidates.add(shrinkBoundingBoxRec(bottom, intersections, index + 1, currentBestArea));
		}
	}
	if (bounding.getRight() > intersections[index].getLeft())
	{
		int32_t sub = bounding.getRight() - intersections[index].getLeft();
		if (sub < bounding.width)
		{
			bbe::Rectanglei left = bounding;
			left.width = left.width - sub;
			if (left.getArea() > currentBestArea)
				candidates.add(shrinkBoundingBoxRec(left, intersections, index + 1, currentBestArea));
		}
	}
	if (intersections[index].getRight() > bounding.getLeft())
	{
		int32_t sub = intersections[index].getRight() - bounding.getLeft();
		if (sub < bounding.width)
		{
			bbe::Rectanglei right = bounding;
			right.width = right.width - sub;
			right.x = right.x + sub;
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

void br::Rooms::update(float timeSinceLastFrame, const bbe::Vector3& camPos, const bbe::SoundDataSource& lightBuzz)
{
	for (size_t i = 0; i < bakedRoomIds.getLength(); i++)
	{
		size_t roomi = bakedRoomIds[i];
		Room& r = rooms[roomi];
		r.timeSinceLastTouch += timeSinceLastFrame;
		if (r.timeSinceLastTouch > 180.0f)
		{
			unbakeLights(roomi);
			i--;
		}
	}

	{
		constexpr uint32_t maxSoundDistance = 10;
		constexpr size_t maxSoundSources = 20;
		bbe::List<BuzzingLight*> allDrawnLights;
		getLights(allDrawnLights, bbe::Vector2i{ (int32_t)camPos.x, (int32_t)camPos.y }, maxSoundDistance);
		if (allDrawnLights.getLength() > maxSoundSources)
		{
			allDrawnLights.sort([&](BuzzingLight* const& a, BuzzingLight* const& b)
				{
					float aDist = a->light.pos.getDistanceTo(camPos);
					float bDist = b->light.pos.getDistanceTo(camPos);
					return aDist < bDist;
				});
		}

		for (size_t i = 0; i < allDrawnLights.getLength() && i < maxSoundSources; i++)
		{
			if (allDrawnLights[i]->buzz.isPlaying() == false)
			{
				allDrawnLights[i]->buzz = lightBuzz.play(allDrawnLights[i]->light.pos + bbe::Vector3(0, 0, 1));
				buzzingLightSounds.add(BuzzingLightSound{ allDrawnLights[i]->buzz, allDrawnLights[i]->light.pos });
			}
		}

		for (size_t i = 0; i < buzzingLightSounds.getLength(); i++)
		{
			float dist = camPos.getDistanceTo(buzzingLightSounds[i].pos);
			if (dist > maxSoundDistance + 5) // + 5 to make sure we don't flip a sound off and on all the time.
			{
				buzzingLightSounds[i].instance.stop();
				buzzingLightSounds.removeIndex(i);
				i--;
			}
		}
	}
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
			bbe::Grid<bool> intersectionGrid(bounding.width, bounding.height);
			for (int32_t i = 0; i < bounding.width; i++)
			{
				for (int32_t k = 0; k < bounding.height; k++)
				{
					bbe::Vector2i pos(i + bounding.x, k + bounding.y);
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

			retVal = bbe::Rectanglei(biggestRect.x + bounding.x, biggestRect.y + bounding.y, biggestRect.width, biggestRect.height);
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
	toFillPoints.resizeCapacity(b->width * 2 + b->height * 2);

	for (int32_t i = 0; i < b->width; i++)
	{
		toFillPoints.add(bbe::Vector2i(b->x + i, b->y             - 1));
		toFillPoints.add(bbe::Vector2i(b->x + i, b->y + b->height    ));
	}
	for (int32_t i = 0; i < b->height; i++)
	{
		toFillPoints.add(bbe::Vector2i(b->x            - 1, b->y + i));
		toFillPoints.add(bbe::Vector2i(b->x + b->width    , b->y + i));
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
		newBounding.x = newBounding.x - rand.randomInt(newBounding.width);
		newBounding.y = newBounding.y - rand.randomInt(newBounding.height);
		if (pos.x == b->x - 1) // Left Edge
		{
			newBounding.x = pos.x - newBounding.width + 1;
		}
		else if (pos.x == b->x + b->width) // Right Edge
		{
			newBounding.x = b->x + b->width;
		}
		else if (pos.y == b->y - 1) // Top Edge
		{
			newBounding.y = pos.y - newBounding.height + 1;
		}
		else if (pos.y == b->y + b->height) // Bottom Edge
		{
			newBounding.y = b->y + b->height;
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

void br::Rooms::generateAtPointMulti_(size_t roomi, bbe::List<size_t>& list, size_t depth)
{
	if (!list.contains(roomi))
	{
		connectGates(roomi);
		list.add(roomi);
	}
	if (depth == (size_t)-1) return;

	for (size_t i = 0; i < rooms[roomi].neighbors.getLength(); i++)
	{
		generateAtPointMulti_(rooms[roomi].neighbors[i].neighborId, list, depth - 1);
	}
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
		bbe::Vector2i roomGatePos(i, rect.getTop() + rect.height - 1);
		bbe::Vector2i neighborGatePos(i, rect.getTop() + rect.height);
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
		bbe::Vector2i roomGatePos(rect.getLeft() + rect.width - 1, i);
		bbe::Vector2i neighborGatePos(rect.getLeft() + rect.width, i);
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

	constexpr uint32_t wallSpaceScale = 4;
	r.walkable = bbe::Grid<bool>(r.boundingBox.getDim() * wallSpaceScale);
	for (int32_t x = 0; x < r.walkable.getWidth(); x++)
	{
		for (int32_t y = 0; y < r.walkable.getHeight(); y++)
		{
			r.walkable[x][y] = x != 0 && y != 0 && x != r.walkable.getWidth() - 1 && y != r.walkable.getHeight() - 1;
		}
	}
	for (const Neighbor& n : r.neighbors)
	{
		for (const Gate& g : n.gates)
		{
			for (int32_t i = 0; i < wallSpaceScale; i++)
			{
				for (int32_t k = 0; k < wallSpaceScale; k++)
				{
					r.walkable[(g.ownGatePos - r.boundingBox.getPos()) * wallSpaceScale + bbe::Vector2i(i, k)] = true;
				}
			}
		}
	}

	enum class InnerRoomType
	{
		EMPTY,
		RANDOM,
		COLUMNS,
		REPEATING,
	};

	bbe::List<bbe::Random::SampleBallsInBagPair<InnerRoomType>> innerRoomTypes
	{
		{InnerRoomType::EMPTY  ,   1},
		{InnerRoomType::RANDOM ,  10},
		{InnerRoomType::COLUMNS,   1},
	};
	if (r.boundingBox.width >= 10 && r.boundingBox.height >= 10)
	{
		innerRoomTypes.add({InnerRoomType::REPEATING, 5});
	}

	const InnerRoomType irt = (InnerRoomType)rand.sampleContainerWithBag(innerRoomTypes);
	if (irt == InnerRoomType::EMPTY)
	{
		// Do nothing
	}
	else if (irt == InnerRoomType::RANDOM)
	{
		const float innerWallProbability = rand.randomFloat() * rand.randomFloat() * rand.randomFloat() * rand.randomFloat() * rand.randomFloat() * rand.randomFloat();
		for (int32_t i = 1; i < r.walkable.getWidth() - 1; i++)
		{
			for (int32_t k = 1; k < r.walkable.getHeight() - 1; k++)
			{
				if (rand.randomFloat() < innerWallProbability)
				{
					const int32_t width = rand.randomInt(10);
					const int32_t height = rand.randomInt(10);
					for (int32_t x = i; x < i + width && x < r.walkable.getWidth(); x++)
					{
						for (int32_t y = k; y < k + height && y < r.walkable.getHeight(); y++)
						{
							r.walkable[x][y] = false;
						}
					}
				}
			}
		}
	}
	else if (irt == InnerRoomType::COLUMNS)
	{
		uint32_t columnSize = rand.randomInt(3) + 1;
		uint32_t columnSpread = columnSize + 5 + rand.randomInt(10);
		uint32_t columnOffset = rand.randomInt(columnSpread) / 2;
		for (int32_t i = 2 + columnOffset; i < r.walkable.getWidth() - 2; i += columnSpread)
		{
			for (int32_t k = 2 + columnOffset; k < r.walkable.getHeight() - 2; k += columnSpread)
			{
				for (int32_t x = 0; x < columnSize && i + x < r.walkable.getWidth(); x++)
				{
					for (int32_t y = 0; y < columnSize && k + y < r.walkable.getHeight(); y++)
					{
						r.walkable[i + x][k + y] = false;
					}
				}
			}
		}
	}
	else if (irt == InnerRoomType::REPEATING)
	{
		const uint32_t patternSize = rand.randomInt(25) + 25;
		bbe::Grid<bool> wallsPattern(patternSize, patternSize);
		uint32_t amountOfWalls = rand.randomInt(5) + 3;
		for (uint32_t i = 0; i < amountOfWalls; i++)
		{
			const int32_t width = rand.randomInt(10);
			const int32_t height = rand.randomInt(10);
			const int32_t x = rand.randomInt(patternSize - width);
			const int32_t y = rand.randomInt(patternSize - height);
			for (int32_t xx = 0; xx < width; xx++)
			{
				for (int32_t yy = 0; yy < height; yy++)
				{
					wallsPattern[xx + x][yy + y] = true;
				}
			}
		}


		for (int32_t i = 2; i < r.walkable.getWidth() - 2; i++)
		{
			for (int32_t k = 2; k < r.walkable.getHeight() - 2; k++)
			{
				r.walkable[i][k] = !wallsPattern[i % wallsPattern.getWidth()][k % wallsPattern.getHeight()];
			}
		}
	}
	else
	{
		throw bbe::IllegalStateException();
	}

	{
		// Fill spaces that can't be reached with walls. This way we reduce the total amount of meshes.
		constexpr int8_t BLOCKED = 0;
		constexpr int8_t UNBLOCKED = 1;
		constexpr int8_t REACHABLE = 2;

		bbe::Grid<int8_t> floodFillGrid(r.walkable.getWidth(), r.walkable.getHeight());
		for (size_t i = 0; i < floodFillGrid.getWidth(); i++)
		{
			for (size_t k = 0; k < floodFillGrid.getHeight(); k++)
			{
				floodFillGrid[i][k] = r.walkable[i][k] ? UNBLOCKED : BLOCKED;
			}
		}

		for (const Neighbor& n : r.neighbors)
		{
			for (const Gate& g : n.gates)
			{
				bbe::Vector2i pos = (g.ownGatePos - r.boundingBox.getPos()) * wallSpaceScale;
				floodFillGrid.floodFill(pos, REACHABLE, false);
			}
		}

		for (size_t i = 0; i < floodFillGrid.getWidth(); i++)
		{
			for (size_t k = 0; k < floodFillGrid.getHeight(); k++)
			{
				r.walkable[i][k] = floodFillGrid[i][k] == REACHABLE;
			}
		}
	}


	{
		bbe::MeshBuilder mb;
		mb.addRectangle(r.floorMatrix());
		r.floorModel = mb.getModel();
	}
	{
		bbe::MeshBuilder mb;
		mb.addRectangle(r.ceilingMatrix());
		r.ceilingModel = mb.getModel();
	}

	bbe::List<bbe::Rectanglei> rects = r.walkable.getAllBiggestRects(false); // TODO: This could be slightly improved. X formations currently generate 3 walls, where they could produce 2.
	bbe::MeshBuilder wallsMb;
	bbe::MeshBuilder skirtingBoardMb;
	for (const bbe::Rectanglei& rect : rects)
	{
		bbe::Vector3 coord = bbe::Vector3(rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f, 1.25f);
		coord.x /= float(wallSpaceScale);
		coord.y /= float(wallSpaceScale);
		wallsMb.addCube(bbe::Cube(coord, bbe::Vector3(rect.width / float(wallSpaceScale), rect.height / float(wallSpaceScale), 2.5f)), bbe::FaceFlag::CIRCUMFERENCE);
		coord.z = 0.075f;
		skirtingBoardMb.addCube(bbe::Cube(coord, bbe::Vector3(rect.width / float(wallSpaceScale) + 0.03f, rect.height / float(wallSpaceScale) + 0.03f, 0.15f)), bbe::FaceFlag::BOTTOMLESS);
	}
	bbe::Vector3 meshPos = bbe::Vector3(r.boundingBox.x, r.boundingBox.y, 0);
	r.wallsModel = Room::ModelOffsetPair{ meshPos, wallsMb.getModel()};
	r.skirtingBoardModel = Room::ModelOffsetPair{ meshPos, skirtingBoardMb.getModel() };

	const float roomLightProbability = 0.0001f + rand.randomFloat() * 0.2f;
	for (int32_t i = 2; i < r.boundingBox.width - 2; i++)
	{
		for (int32_t k = 2; k < r.boundingBox.height - 2; k++)
		{
			if (rand.randomFloat() >= roomLightProbability) continue;
			bool wallBlockingLight = false;

			for (int32_t x = 0; x < wallSpaceScale; x++)
			{
				for (int32_t y = 0; y < wallSpaceScale; y++)
				{
					if (!r.walkable[i * wallSpaceScale + x][k * wallSpaceScale + y])
					{
						wallBlockingLight = true;
						goto outer;
					}
				}
			}
		outer:

			if (!wallBlockingLight)
			{
				bbe::PointLight pl;
				pl.pos = bbe::Vector3(i + r.boundingBox.x + 0.5f, k + r.boundingBox.y + 0.5f, 2.f);
				pl.lightStrength = 1.5f;
				BuzzingLight bl;
				bl.light = pl;
				r.lights.add(bl);
			}
		}
	}

	bbe::MeshBuilder lightMb;
	for (const BuzzingLight& light : r.lights)
	{
		lightMb.addCube(bbe::Cube(light.light.pos + bbe::Vector3(0.05f, 0.05f, 0.5f), bbe::Vector3(0.9f, 0.9f, 0.01f)), bbe::FaceFlag::TOPLESS);
	}
	r.lightsModel = Room::ModelOffsetPair{ bbe::Vector3(), lightMb.getModel()}; // TODO: The 0 offset while backing the vertex poses in world coords is a bad idea for accuracy.
}

bool br::Rooms::bakeLights(size_t roomi, bbe::Game* game, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard)
{
	rooms[roomi].timeSinceLastTouch = 0.f;
	if (rooms[roomi].state < RoomGenerationState::gatesConnected) connectGates(roomi);
	if (rooms[roomi].state != RoomGenerationState::gatesConnected && rooms[roomi].state != RoomGenerationState::baking) return false;
	rooms[roomi].state = RoomGenerationState::baking;

	if (!bakedRoomIds.contains(roomi)) bakedRoomIds.add(roomi);

	// Making sure all neighbors lights have been created.
	bbe::List<size_t> roomLightSources = generateMulti(roomi, 2);
	Room& r = rooms[roomi];

	bbe::List<bbe::PointLight> lights;
	const int32_t lightmapSize = 128;
	for (size_t i = 0; i < roomLightSources.getLength(); i++)
	{
		for (size_t k = 0; k < rooms[roomLightSources[i]].lights.getLength(); k++)
		{
			lights.add(rooms[roomLightSources[i]].lights[k].light);
		}
	}
	if (r.bakedCeiling.isLoadedCpu() == false && r.bakedCeiling.isLoadedGpu() == false)
	{
		r.bakedCeiling = game->bakeLights(r.ceilingTranslation(), r.ceilingModel, nullptr, shaderCeiling, { lightmapSize, lightmapSize }, lights);
	}
	else if (r.bakedFloor.isLoadedCpu() == false && r.bakedFloor.isLoadedGpu() == false)
	{
		r.bakedFloor = game->bakeLights(r.floorTranslation(), r.floorModel, nullptr, shaderFloor, { lightmapSize, lightmapSize }, lights);
	}
	else if (rooms[roomi].bakedWalls.getLength() != 1)
	{
		r.bakedWalls.add(game->bakeLights(bbe::Matrix4::createTranslationMatrix(r.wallsModel.offset), r.wallsModel.model, nullptr, shaderWall, { lightmapSize, lightmapSize }, lights));
	}
	else if (rooms[roomi].bakedSkirtingBoard.getLength() != 1)
	{
		r.bakedSkirtingBoard.add(game->bakeLights(bbe::Matrix4::createTranslationMatrix(r.skirtingBoardModel.offset), r.skirtingBoardModel.model, nullptr, shaderSkirtingBoard, { lightmapSize, lightmapSize }, lights));
	}
	else
	{
		// Must happen at the start rather than the end because we could have 0 walls.
		rooms[roomi].state = RoomGenerationState::lightsBaked;
		return false;
	}
	return true;
}

void br::Rooms::unbakeLights(size_t roomi)
{
	if (rooms[roomi].state < RoomGenerationState::baking)
	{
		throw bbe::IllegalStateException();
	}
	if (!bakedRoomIds.removeSingle(roomi))
	{
		throw bbe::IllegalStateException();
	}
	Room& r = rooms[roomi];
	r.bakedCeiling = bbe::Image();
	r.bakedFloor = bbe::Image();
	r.bakedWalls.clear();
	r.bakedSkirtingBoard.clear();
	r.state = RoomGenerationState::gatesConnected;
}

void br::Rooms::bakeLightsOfNeighborsBasedOnPriorityList(const bbe::List<size_t>& roomis, bbe::Game* game, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard)
{
	for (size_t roomi : roomis)
	{
		bakeLights(roomi, game, shaderFloor, shaderWall, shaderCeiling, shaderSkirtingBoard);
		if (rooms[roomi].state < RoomGenerationState::lightsBaked)
		{
			return;
		}
	}

	for (size_t roomi : roomis)
	{
		for (size_t i = 0; i < rooms[roomi].neighbors.getLength(); i++)
		{
			size_t neighborId = rooms[roomi].neighbors[i].neighborId;
			bakeLights(neighborId, game, shaderFloor, shaderWall, shaderCeiling, shaderSkirtingBoard);
			if (rooms[neighborId].state < RoomGenerationState::lightsBaked)
			{
				return;
			}
		}
	}
}

size_t br::Rooms::generateAtPoint(const bbe::Vector2i& position)
{
	size_t roomi = lookupRoomIndex(position);
	connectGates(roomi);
	return roomi;
}

bbe::List<size_t> br::Rooms::generateAtPointMulti(const bbe::Vector2i& position, size_t depth)
{
	size_t roomi = lookupRoomIndex(position);
	return generateMulti(roomi, depth);
}

bbe::List<size_t> br::Rooms::generateMulti(size_t roomi, size_t depth)
{
	bbe::List<size_t> retVal;
	connectGates(roomi);
	generateAtPointMulti_(roomi, retVal, depth);
	return retVal;
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

bool br::Rooms::isRoomVisible(size_t roomi)
{
	return rooms[roomi].visible;
}

void br::Rooms::updateOcclusionQueries(size_t roomi, bbe::PrimitiveBrush3D& brush)
{
	Room& r = rooms[roomi];
	r.occlusionQueries.add(
		{
			brush.isCubeVisible(r.getBoundingCubeInner()),
			brush.isCubeVisible(r.getBoundingCubeOuter())
		}
	);
	while (!r.occlusionQueries.isEmpty() && r.occlusionQueries.peek().inner.isValueReady() && r.occlusionQueries.peek().outer.isValueReady())
	{
		Room::OcclusionQueryPair oqp = r.occlusionQueries.pop();
		r.visible = oqp.inner.getValue() || oqp.outer.getValue();
	}
}

void br::Rooms::drawAt(const bbe::Vector3 pos, bbe::PrimitiveBrush3D& brush, bbe::Game* game, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard, bool drawFloor, bool drawWalls, bool drawCeiling, bool drawLights)
{
	bbe::Vector2i lookupPos((int32_t)pos.x, (int32_t)pos.y);
	if (pos.x < 0) lookupPos.x--;
	if (pos.y < 0) lookupPos.y--;
	size_t roomi = lookupRoomIndex(lookupPos);
	bbe::List<size_t> alreadyDrawn;
	bbe::List<size_t> neighborList;
	bool bakedRoom = false;
	drawRoomsRecursively(alreadyDrawn, neighborList, bakedRoom, roomi, brush, game, shaderFloor, shaderWall, shaderCeiling, shaderSkirtingBoard, drawFloor, drawWalls, drawCeiling, drawLights);
	for (size_t roomi : neighborList)
	{
		updateOcclusionQueries(roomi, brush);
	}
	if (!bakedRoom)
	{
		bakeLightsOfNeighborsBasedOnPriorityList(alreadyDrawn, game, shaderFloor, shaderWall, shaderCeiling, shaderSkirtingBoard);
	}
	RoomIterator ri(this, roomi);
	for(size_t i = 0; i<32 && !bakedRoom; i++)
	{
		bakedRoom = bakeLights(ri.next(), game, shaderFloor, shaderWall, shaderCeiling, shaderSkirtingBoard);
	}

	// Also always draw available neighbors of the original Room. This prevents popups in case the user is spinning several times. Also prevents hiding a room for a frame on gate traversal.
	for (size_t i = 0; i < rooms[roomi].neighbors.getLength(); i++)
	{
		size_t neighborId = rooms[roomi].neighbors[i].neighborId;
		if (rooms[neighborId].state >= RoomGenerationState::lightsBaked) // Only draw baked rooms to prevent unnecessary lag spikes on room traversal
		{
			drawRoom(neighborId, brush, game, shaderFloor, shaderWall, shaderCeiling, shaderSkirtingBoard, drawFloor, drawWalls, drawCeiling, drawLights);
		}
	}
}

void br::Rooms::drawRoom(size_t roomi, bbe::PrimitiveBrush3D& brush, bbe::Game* game, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard, bool drawFloor, bool drawWalls, bool drawCeiling, bool drawLights)
{
	if (rooms[roomi].state < RoomGenerationState::lightsBaked)
	{
		// Draw the bounding box to stop other rooms to become worngly visible.
		brush.fillCube(rooms[roomi].getBoundingCubeInner());
		return;
	}

	const Room& r = rooms[roomi];
	brush.setColor(1, 1, 1, 1);
	if (drawLights)
	{
		brush.fillModel(bbe::Matrix4::createTranslationMatrix(r.lightsModel.offset), r.lightsModel.model, nullptr, nullptr, &bbe::Image::white());
	}
	brush.setColorHSV(r.hue, r.saturation, r.value);
	if (r.bakedWalls.getLength() != 1) throw bbe::IllegalStateException();
	if(drawFloor) brush.fillModel(r.floorTranslation(), r.floorModel, nullptr, nullptr, &r.bakedFloor, shaderFloor);
	if(drawCeiling) brush.fillModel(r.ceilingTranslation(), r.ceilingModel, nullptr, nullptr, &r.bakedCeiling, shaderCeiling);
	if (drawWalls)
	{
		brush.fillModel(bbe::Matrix4::createTranslationMatrix(r.wallsModel.offset), r.wallsModel.model, nullptr, nullptr, &r.bakedWalls[0], shaderWall);
		brush.setColor(1, 1, 1, 1);
		brush.fillModel(bbe::Matrix4::createTranslationMatrix(r.skirtingBoardModel.offset), r.skirtingBoardModel.model, nullptr, nullptr, &r.bakedSkirtingBoard[0], shaderSkirtingBoard);
	}
}

void br::Rooms::drawRoomsRecursively(bbe::List<size_t>& alreadyDrawn, bbe::List<size_t>& neighborList, bool& bakedRoom, size_t roomi, bbe::PrimitiveBrush3D& brush, bbe::Game* game, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard, bool drawFloor, bool drawWalls, bool drawCeiling, bool drawLights)
{
	if (alreadyDrawn.contains(roomi)) return;
	alreadyDrawn.add(roomi);
	if (rooms[roomi].state < RoomGenerationState::lightsBaked)
	{
		if (!bakedRoom)
		{
			bakeLights(roomi, game, shaderFloor, shaderWall, shaderCeiling, shaderSkirtingBoard);
			bakedRoom = true;
		}
	}
	drawRoom(roomi, brush, game, shaderFloor, shaderWall, shaderCeiling, shaderSkirtingBoard, drawFloor, drawWalls, drawCeiling, drawLights);
	if (rooms[roomi].state < RoomGenerationState::lightsBaked)
	{
		return;
	}
	for (size_t i = 0; i < rooms[roomi].neighbors.getLength(); i++)
	{
		size_t neighborId = rooms[roomi].neighbors[i].neighborId;
		if (!neighborList.contains(neighborId)) neighborList.add(neighborId);
		if (isRoomVisible(neighborId))
		{
			drawRoomsRecursively(alreadyDrawn, neighborList, bakedRoom, neighborId, brush, game, shaderFloor, shaderWall, shaderCeiling, shaderSkirtingBoard, drawFloor, drawWalls, drawCeiling, drawLights);
		}
	}
}

void br::Rooms::getLights(bbe::List<BuzzingLight*>& allDrawnLights, const bbe::Vector2i& position, int32_t maxDist)
{
	bbe::List<size_t> roomis;
	getRooms(roomis, position, maxDist);

	for (size_t roomi : roomis)
	{
		// Doing this in a separate loop to make 100% sure we don't change the rooms array anymore afterwards.
		connectGates(roomi);
	}

	for (size_t roomi : roomis)
	{
		Room& r = rooms[roomi];
		for (size_t i = 0; i < r.lights.getLength(); i++)
		{
			BuzzingLight& bz = r.lights[i];
			bbe::Vector2i lightPos = bbe::Vector2i((int32_t)bz.light.pos.x, (int32_t)bz.light.pos.y);
			const int32_t distance = lightPos.getDistanceTo(position);
			if (distance <= maxDist)
			{
				allDrawnLights.add(&bz);
			}
		}
	}
}

void br::Rooms::getRooms(bbe::List<size_t>& roomis, const bbe::Vector2i& position, int32_t maxDist)
{
	roomis.clear();
	size_t roomi = getRoomIndexAtPoint(position);
	getRooms(roomis, roomi, position, maxDist);
}

void br::Rooms::getRooms(bbe::List<size_t>& roomis, size_t roomi, const bbe::Vector2i& position, int32_t maxDist)
{
	if (roomis.contains(roomi)) return;
	determineNeighbors(roomi);
	const int32_t distance = rooms[roomi].boundingBox.getDistanceTo(position);
	if (distance > maxDist) return;
	roomis.add(roomi);
	for (size_t i = 0; i < rooms[roomi].neighbors.getLength(); i++)
	{
		getRooms(roomis, rooms[roomi].neighbors[i].neighborId, position, maxDist);
	}
}

br::RoomIterator::RoomIterator(Rooms* rooms, size_t startIndex) :
	rooms(rooms)
{
	currentWave.add(startIndex);
}

size_t br::RoomIterator::next()
{
	if (currentWave.isEmpty())
	{
		currentWave = nextWave;
		nextWave.clear();
	}
	const size_t retVal = currentWave.popBack();
	const Room& r = rooms->rooms[retVal];

	for (const Neighbor& n : r.neighbors)
	{
		if (!visitedRooms.contains(n.neighborId)
			&& !currentWave.contains(n.neighborId)
			&& !nextWave.contains(n.neighborId))
		{
			nextWave.add(n.neighborId);
		}
	}

	return retVal;
}

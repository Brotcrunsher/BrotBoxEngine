#pragma once
#include "BBE/BrotBoxEngine.h"
#include "Room.h"

namespace br
{
	struct Rooms
	{
	private:
		bbe::Rectanglei shrinkBoundingBoxRec(const bbe::Rectanglei& bounding, const bbe::List<bbe::Rectanglei>& intersections, int32_t index, int32_t &currentBestArea) const;
		void determineNeighbors_(size_t roomi, const bbe::Vector2i& roomiGatePos, const bbe::Vector2i& neighborGatePos);
		void generateAtPointMulti_(size_t roomi, bbe::List<size_t> &list, size_t depth);

	public:
		Rooms();


		bbe::List<Room> rooms;
		bbe::Random rand;
		bbe::HashMap<bbe::Vector2i, bbe::List<size_t>> hashGrid;
		
		void clear();

		// Debug
		void setSeed(int seed);

		bbe::Rectanglei newBoundingAt(const bbe::Vector2i& position);
		size_t lookupRoomIndex(const bbe::Vector2i& position);
		bbe::Rectanglei shrinkBoundingBox(const bbe::Rectanglei &bounding) const;
		bool expandRoom(size_t roomi);
		void determineNeighbors(size_t roomi);
		void collapseGates(size_t roomi);
		void connectGates(size_t roomi);
		bool bakeLights(size_t roomi, bbe::Game* game, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling);

		size_t generateAtPoint(const bbe::Vector2i& position);
		bbe::List<size_t> generateAtPointMulti(const bbe::Vector2i& position, size_t depth);
		int32_t getRoomIndexAtPoint(const bbe::Vector2i& position, int32_t ignore_room = -1) const;
		size_t bakeAtPoint(const bbe::Vector2i& position, bbe::Game* game, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling);
		void propagateSingleBakeAtPoint(const bbe::Vector2i& position, bbe::Game* game, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling);

		void addRoom(const bbe::Rectanglei& bounding);
	};
}

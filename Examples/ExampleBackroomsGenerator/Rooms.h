#pragma once
#include <optional>
#include "BBE/BrotBoxEngine.h"
#include "Room.h"

namespace br
{
	struct Rooms;
	struct RoomIterator
	{
	private:
		Rooms* rooms = nullptr;
		bbe::List<size_t> visitedRooms;
		bbe::List<size_t> currentWave;
		bbe::List<size_t> nextWave;

	public:
		RoomIterator(Rooms* rooms, size_t startIndex);

		size_t next();
	};

	struct Rooms
	{
	private:
		bbe::Rectanglei shrinkBoundingBoxRec(const bbe::Rectanglei& bounding, const bbe::List<bbe::Rectanglei>& intersections, int32_t index, int32_t &currentBestArea) const;
		void determineNeighbors_(size_t roomi, const bbe::Vector2i& roomiGatePos, const bbe::Vector2i& neighborGatePos);
		void generateAtPointMulti_(size_t roomi, bbe::List<size_t> &list, size_t depth);
		void getRooms(bbe::List<size_t>& roomis, size_t roomi, const bbe::Vector2i& position, int32_t maxDist); // TODO: Change roomis to set

		struct BuzzingLightSound
		{
			bbe::SoundInstance instance;
			bbe::Vector3 pos;
		};
		bbe::List<BuzzingLightSound> buzzingLightSounds;

	public:
		Rooms();


		bbe::List<Room> rooms;
		bbe::List<size_t> bakedRoomIds;
		bbe::Random rand;
		bbe::HashMap<bbe::Vector2i, bbe::List<size_t>> hashGrid;
		
		void clear();
		void update(float timeSinceLastFrame, const bbe::Vector3& camPos, const bbe::SoundDataSource& lightBuzz);

		// Debug
		void setSeed(int seed);

		bbe::Rectanglei newBoundingAt(const bbe::Vector2i& position);
		size_t lookupRoomIndex(const bbe::Vector2i& position);
		std::optional<bbe::Rectanglei> shrinkBoundingBox(const bbe::Rectanglei &bounding) const;
		bool expandRoom(size_t roomi);
		void determineNeighbors(size_t roomi);
		void collapseGates(size_t roomi);
		void connectGates(size_t roomi);
		bool bakeLightsStep(size_t roomi, bbe::PrimitiveBrush3D& brush, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard);
		bool bakeLights(size_t roomi, uint32_t& bakingBudget, bbe::PrimitiveBrush3D& brush, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard);
		void unbakeLights(size_t roomi);
		void bakeLightsOfNeighborsBasedOnPriorityList(const bbe::List<size_t>& roomis, uint32_t& bakingBudget, bbe::PrimitiveBrush3D& brush, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard);

		size_t generateAtPoint(const bbe::Vector2i& position);
		bbe::List<size_t> generateAtPointMulti(const bbe::Vector2i& position, size_t depth);
		bbe::List<size_t> generateMulti(size_t roomi, size_t depth);
		int32_t getRoomIndexAtPoint(const bbe::Vector2i& position, int32_t ignore_room = -1) const;
		
		void addRoom(const bbe::Rectanglei& bounding);

		bool isRoomVisible(size_t roomi);
		void updateOcclusionQueries(size_t roomi, bbe::PrimitiveBrush3D& brush);
		void drawAt(const bbe::Vector3 pos, bbe::PrimitiveBrush3D& brush, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard, bool drawFloor, bool drawWalls, bool drawSkirtingBoard, bool drawCeiling, bool drawLights);
		void drawRoom(size_t roomi, bbe::PrimitiveBrush3D& brush, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard, bool drawFloor, bool drawWalls, bool drawSkirtingBoard, bool drawCeiling, bool drawLights);
		void drawRoomsRecursively(bbe::List<size_t>& alreadyDrawn, bbe::List<size_t>& neighborList, uint32_t& bakingBudget, size_t roomi, bbe::PrimitiveBrush3D& brush, bbe::FragmentShader* shaderFloor, bbe::FragmentShader* shaderWall, bbe::FragmentShader* shaderCeiling, bbe::FragmentShader* shaderSkirtingBoard, bool drawFloor, bool drawWalls, bool drawSkirtingBoard, bool drawCeiling, bool drawLights);

		void getLights(bbe::List<BuzzingLight*>& allDrawnLights, const bbe::Vector2i& position, int32_t maxDist);
		void getRooms(bbe::List<size_t>& roomis, const bbe::Vector2i& position, int32_t maxDist);

		bool isPositionInWall(const bbe::Vector2i& pos);
		bool isPositionInWall(const bbe::Vector3& pos);
		bool isLineInWall(const bbe::Vector2i& start, const bbe::Vector2i& end);
		bool doesPointSeeRoomInterior(const bbe::Vector3& pos, size_t roomi);

		float getDistanceToRoom(size_t roomi, const bbe::Vector3 pos);
	};
}

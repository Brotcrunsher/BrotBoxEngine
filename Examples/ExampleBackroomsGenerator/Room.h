#pragma once
#include "BBE/BrotBoxEngine.h"

namespace br
{
	enum class RoomGenerationState
	{
		outlines,
		expanded,
		neighborsDetermined,
		gatesCollapsed,
		gatesConnected,
	};

	struct Gate
	{
		bbe::Vector2i ownGatePos;
		bbe::Vector2i neighborGatePos;

		bool operator==(const Gate& other) const;
		Gate flipped() const;
	};

	struct Neighbor
	{
		size_t neighborId = 0;
		bbe::List<Gate> gates;
	};

	struct Room
	{
		bbe::Rectanglei boundingBox;
		float hue = 0.f;
		float value = 0.f;
		float saturation = 0.f;
		size_t id = 0;
		RoomGenerationState state = RoomGenerationState::outlines;
		bbe::List<Neighbor> neighbors;
		bbe::Grid<bool> walkable;
		bbe::Model wallsModel;
		bbe::List<bbe::PointLight> lights;

		bbe::List<bbe::Vector2i> getHashGridPositions() const;
		static bbe::List<bbe::Vector2i> getHashGridPositions(const bbe::Rectanglei& rect);
		static bbe::Vector2i getHashGridPosition(const bbe::Vector2i& pos);
	};
}

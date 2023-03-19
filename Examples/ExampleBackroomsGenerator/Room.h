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
		baking,
		lightsBaked,
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
		bbe::Model ceilingModel;
		bbe::Model floorModel;
		struct ModelOffsetPair
		{
			bbe::Vector3 offset;
			bbe::Model model;
		};
		bbe::List<ModelOffsetPair> wallsModels;
		bbe::List<bbe::PointLight> lights;
		bbe::Image bakedCeiling;
		bbe::Image bakedFloor;
		bbe::List<bbe::Image> bakedLights;
		struct OcclusionQueryPair
		{
			bbe::Future<bool> inner;
			bbe::Future<bool> outer;
		};
		bbe::List<OcclusionQueryPair> occlusionQueries;
		bool visible = false;

		bbe::List<bbe::Vector2i> getHashGridPositions() const;
		static bbe::List<bbe::Vector2i> getHashGridPositions(const bbe::Rectanglei& rect);
		static bbe::Vector2i getHashGridPosition(const bbe::Vector2i& pos);

		bbe::Vector3 getBoundingCubePos() const;
		bbe::Vector3 getBoundingCubeScale() const;
		bbe::Cube getBoundingCubeOuter() const;
		bbe::Cube getBoundingCubeInner() const;
		bbe::Matrix4 floorMatrix() const;
		bbe::Matrix4 ceilingMatrix() const;
		bbe::Matrix4 floorTranslation() const;
		bbe::Matrix4 ceilingTranslation() const;
		bbe::Color getColor() const;
	};
}

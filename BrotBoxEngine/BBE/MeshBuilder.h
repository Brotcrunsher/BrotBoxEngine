#pragma once

#include "../BBE/Model.h"
#include "../BBE/Cube.h"
#include "../BBE/Matrix4.h"
#include "../BBE/List.h"

namespace bbe
{
	class MeshBuilder
	{
	private:
		bbe::List<bbe::Matrix4> quads;

	public:
		MeshBuilder();

		void addCube(const bbe::Cube& cube, FaceFlag ff = FaceFlag::ALL);
		void addCubes(const bbe::List<Cube>& cubes);
		void addRectangle(const bbe::Matrix4& transform);

		struct ModelUvDimensionsPair
		{
			bbe::Model model;
			bbe::Vector2i uvDimensions;
		};
		ModelUvDimensionsPair getModel(uint32_t pixelsPerUnit);
	};
}

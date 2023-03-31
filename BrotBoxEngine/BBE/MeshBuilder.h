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

		bbe::Model getModel();
	};
}

#pragma once

#include "../BBE/Model.h"
#include "../BBE/Cube.h"
#include "../BBE/Matrix4.h"

namespace bbe
{
	class MeshBuilder
	{
	private:
		bbe::Model m_model;
		uint32_t meshes = 0;

		void addElement(bbe::List<PosNormalPair>& vertices, bbe::List<uint32_t>& indices);

	public:
		MeshBuilder();

		void addCube(const bbe::Cube& cube, FaceFlag ff = FaceFlag::ALL);
		void addCubes(const bbe::List<Cube>& cubes);
		void addRectangle(const bbe::Matrix4& transform);

		bbe::Model getModel();
	};
}

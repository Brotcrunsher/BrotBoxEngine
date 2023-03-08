#pragma once

#include "../BBE/Model.h"
#include "../BBE/Cube.h"

namespace bbe
{
	class MeshBuilder
	{
	private:
		bbe::Model m_model;
		uint32_t meshes = 0;

	public:
		MeshBuilder();

		void addCube(const bbe::Cube& cube);
		void addCubes(const bbe::List<Cube>& cubes);

		bbe::Model getModel();
	};
}

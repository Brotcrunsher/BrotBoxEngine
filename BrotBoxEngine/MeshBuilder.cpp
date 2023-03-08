#include "BBE/MeshBuilder.h"

bbe::MeshBuilder::MeshBuilder()
{
	// Do nothing.
}

void bbe::MeshBuilder::addCube(const bbe::Cube& cube)
{
	bbe::List<PosNormalPair> vertices = cube.getRenderVertices();
	bbe::List<uint32_t> indices = Cube::getRenderIndicesDefault();
	for (uint32_t& i : indices)
	{
		i += (uint32_t)m_model.getAmountOfVertices();
	}
	const bbe::Vector2 uvOffset = bbe::Math::squareCantor(meshes).as<float>();
	for (PosNormalPair& vertex : vertices)
	{
		vertex.uvCoord += uvOffset;
	}
	m_model.add(vertices, indices);
	meshes++;
}

void bbe::MeshBuilder::addCubes(const bbe::List<Cube>& cubes)
{
	for (const Cube& c : cubes)
	{
		addCube(c);
	}
}

bbe::Model bbe::MeshBuilder::getModel()
{
	return m_model.finalize();
}

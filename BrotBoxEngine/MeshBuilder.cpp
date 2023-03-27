#include "BBE/MeshBuilder.h"

void bbe::MeshBuilder::addElement(bbe::List<PosNormalPair>& vertices, bbe::List<uint32_t>& indices)
{
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

bbe::MeshBuilder::MeshBuilder()
{
	// Do nothing.
}

void bbe::MeshBuilder::addCube(const bbe::Cube& cube, FaceFlag ff)
{
	bbe::List<PosNormalPair> vertices = cube.getRenderVertices(ff);
	bbe::List<uint32_t> indices = Cube::getRenderIndicesDefault(ff);
	addElement(vertices, indices);
}

void bbe::MeshBuilder::addCubes(const bbe::List<Cube>& cubes)
{
	for (const Cube& c : cubes)
	{
		addCube(c);
	}
}

void bbe::MeshBuilder::addRectangle(const bbe::Matrix4& transform)
{
	bbe::List<PosNormalPair> vertices =
		{
			bbe::PosNormalPair{bbe::Vector3(-0.5, -0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(0, 0)},
			bbe::PosNormalPair{bbe::Vector3(-0.5,  0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(0, 1)},
			bbe::PosNormalPair{bbe::Vector3( 0.5, -0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(1, 0)},
			bbe::PosNormalPair{bbe::Vector3( 0.5,  0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(1, 1)},
		};
	bbe::PosNormalPair::transform(vertices, transform);
	bbe::List<uint32_t> indices = { 0, 1, 2, 2, 1, 3 };
	addElement(vertices, indices);
}

bbe::Model bbe::MeshBuilder::getModel()
{
	return m_model.finalize();
}

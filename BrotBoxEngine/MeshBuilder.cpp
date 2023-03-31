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
	const bbe::Matrix4 t = bbe::Matrix4::createTranslationMatrix(bbe::Vector3(0, 0, 0.5));
	if ((int)ff & (int)FaceFlag::BOTTOM)
	{
		addRectangle(cube.getTransform() * bbe::Matrix4::createRotationMatrix(bbe::Math::PI, bbe::Vector3(1, 0, 0)) * t);
	}
	if ((int)ff & (int)FaceFlag::TOP)
	{
		addRectangle(cube.getTransform() * t);
	}
	if ((int)ff & (int)FaceFlag::LEFT)
	{
		addRectangle(cube.getTransform() * bbe::Matrix4::createRotationMatrix(bbe::Math::PI * 0.5f, bbe::Vector3(1, 0, 0)) * t);
	}
	if ((int)ff & (int)FaceFlag::RIGHT)
	{
		addRectangle(cube.getTransform() * bbe::Matrix4::createRotationMatrix(-bbe::Math::PI * 0.5f, bbe::Vector3(1, 0, 0)) * t);
	}
	if ((int)ff & (int)FaceFlag::FRONT)
	{
		addRectangle(cube.getTransform() * bbe::Matrix4::createRotationMatrix(bbe::Math::PI * 0.5f, bbe::Vector3(0, 1, 0)) * t);
	}
	if ((int)ff & (int)FaceFlag::BACK)
	{
		addRectangle(cube.getTransform() * bbe::Matrix4::createRotationMatrix(-bbe::Math::PI * 0.5f, bbe::Vector3(0, 1, 0)) * t);
	}
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

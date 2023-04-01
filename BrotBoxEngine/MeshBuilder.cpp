#include "BBE/MeshBuilder.h"
#include "BBE/Rectangle.h"

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
	quads.add(transform);
}

bbe::MeshBuilder::ModelUvDimensionsPair bbe::MeshBuilder::getModel(uint32_t pixelsPerUnit)
{
	ModelUvDimensionsPair retVal;
	bbe::List<bbe::Rectanglei> rects;
	for (size_t i = 0; i < quads.getLength(); i++)
	{
		const bbe::Vector3 scale = quads[i].extractScale();
		rects.add(bbe::Rectanglei(
			0, 0,
			bbe::Math::max(1.0f, scale.x * pixelsPerUnit),
			bbe::Math::max(1.0f, scale.y * pixelsPerUnit)
		));
	}
	if (rects.getLength() == 0)
	{
		// Create a dummy dimension so that the caller can blindly use it without getting issues that a texture was impossible to create etc.
		retVal.uvDimensions = bbe::Vector2i(1, 1);
	}
	else
	{
		retVal.uvDimensions = bbe::Rectanglei::pack(rects);
	}

	for (size_t i = 0; i < quads.getLength(); i++)
	{
		const bbe::Matrix4& quad = quads[i];
		const bbe::Rectanglei& uvRect = rects[i];

		bbe::List<PosNormalPair> vertices =
		{
			bbe::PosNormalPair{bbe::Vector3(-0.5, -0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(uvRect.getLeft()  + 0.5f, uvRect.getTop()    + 0.5f)},
			bbe::PosNormalPair{bbe::Vector3(-0.5,  0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(uvRect.getLeft()  + 0.5f, uvRect.getBottom() - 0.5f)},
			bbe::PosNormalPair{bbe::Vector3( 0.5, -0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(uvRect.getRight() - 0.5f, uvRect.getTop()    + 0.5f)},
			bbe::PosNormalPair{bbe::Vector3( 0.5,  0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(uvRect.getRight() - 0.5f, uvRect.getBottom() - 0.5f)},
		};
		retVal.model.m_bakingUvs.add(bbe::Vector2(uvRect.getLeft() , uvRect.getTop()   ));
		retVal.model.m_bakingUvs.add(bbe::Vector2(uvRect.getLeft() , uvRect.getBottom()));
		retVal.model.m_bakingUvs.add(bbe::Vector2(uvRect.getRight(), uvRect.getTop()   ));
		retVal.model.m_bakingUvs.add(bbe::Vector2(uvRect.getRight(), uvRect.getBottom()));
		bbe::PosNormalPair::transform(vertices, quad);
		const bbe::List<uint32_t> indices = { 
			0 + 4 * (uint32_t)i,
			1 + 4 * (uint32_t)i,
			2 + 4 * (uint32_t)i,
			2 + 4 * (uint32_t)i,
			1 + 4 * (uint32_t)i,
			3 + 4 * (uint32_t)i };

		retVal.model.add(vertices, indices);
	}
	retVal.model = retVal.model.finalize(retVal.uvDimensions);
	return retVal;
}

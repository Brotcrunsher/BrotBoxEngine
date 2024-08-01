#include "BBE/OpenGL/OpenGLSphere.h"
#include "BBE/OpenGL/OpenGLManager.h"
#include "BBE/List.h"
#include "BBE/Vector3.h"
#include "BBE/Math.h"

static GLuint vbo = 0;
static GLuint ibo = 0;

// TODO This is basically a copy of the Vulkan implementation. Generalize it somehow
//      and remove the redundant code. Same applies to the OpenGLCube implementation.

static bbe::List<uint32_t> indices = {
	5,  11, 0,
	1,  5,  0,
	7,  1,  0,
	10, 7,  0,
	11, 10, 0,

	9, 5,  1,
	4, 11, 5,
	2, 10, 11,
	6, 7,  10,
	8, 1,  7,

	4, 9, 3,
	2, 4, 3,
	6, 2, 3,
	8, 6, 3,
	9, 8, 3,

	5,  9, 4,
	11, 4, 2,
	10, 2, 6,
	7,  6, 8,
	1,  8, 9,
};

static uint32_t getHalfPointIndex(bbe::List<bbe::Vector3>& vertices, const bbe::Vector3& a, const bbe::Vector3& b)
{
	bbe::Vector3 halfPoint = (a + b).normalize() / 2;
	for (uint32_t i = 0; i < vertices.getLength(); i++)
	{
		if (halfPoint == vertices[i])
		{
			return i;
		}
	}

	vertices.add(halfPoint);
	return static_cast<uint32_t>(vertices.getLength() - 1);
}

void bbe::INTERNAL::openGl::OpenGLSphere::init()
{
	float x = (1 + bbe::Math::sqrt(5)) / 4;
	bbe::List<bbe::Vector3> positions = {
		bbe::Vector3(-0.5,  x,  0).normalize() / 2,
		bbe::Vector3(0.5,  x,  0).normalize() / 2,
		bbe::Vector3(-0.5, -x,  0).normalize() / 2,
		bbe::Vector3(0.5, -x,  0).normalize() / 2,

		bbe::Vector3(0, -0.5,  x).normalize() / 2,
		bbe::Vector3(0,  0.5,  x).normalize() / 2,
		bbe::Vector3(0, -0.5, -x).normalize() / 2,
		bbe::Vector3(0,  0.5, -x).normalize() / 2,

		bbe::Vector3(x,  0, -0.5).normalize() / 2,
		bbe::Vector3(x,  0,  0.5).normalize() / 2,
		bbe::Vector3(-x, 0, -0.5).normalize() / 2,
		bbe::Vector3(-x, 0,  0.5).normalize() / 2,
	};

	constexpr int iterations = 2;
	for (int i = 0; i < iterations; i++)
	{
		bbe::List<uint32_t> newIndices;

		for (size_t k = 0; k < indices.getLength(); k += 3)
		{
			uint32_t a = getHalfPointIndex(positions, positions[indices[k + 0]], positions[indices[k + 1]]);
			uint32_t b = getHalfPointIndex(positions, positions[indices[k + 1]], positions[indices[k + 2]]);
			uint32_t c = getHalfPointIndex(positions, positions[indices[k + 2]], positions[indices[k + 0]]);

			newIndices.addAll(
				c, a, indices[k + 0],
				a, b, indices[k + 1],
				b, c, indices[k + 2],
				c, b, a
			);
		}

		indices = std::move(newIndices);
	}

	bbe::List<PosNormalPair> vertexData;
	for (const bbe::Vector3& pos : positions)
	{
		vertexData.add(PosNormalPair{pos, pos});
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PosNormalPair) * vertexData.getLength(), vertexData.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.getLength(), indices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void bbe::INTERNAL::openGl::OpenGLSphere::destroy()
{
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
}

GLuint bbe::INTERNAL::openGl::OpenGLSphere::getVbo()
{
	return vbo;
}

GLuint bbe::INTERNAL::openGl::OpenGLSphere::getIbo()
{
	return ibo;
}

size_t bbe::INTERNAL::openGl::OpenGLSphere::getAmountOfIndices()
{
	return indices.getLength();
}

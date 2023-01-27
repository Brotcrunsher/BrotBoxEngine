#include "BBE/OpenGL/OpenGLCube.h"
#include "BBE/OpenGL/OpenGLManager.h"
#include "BBE/List.h"
#include "BBE/Vector3.h"

static GLuint vbo = 0;
static GLuint ibo = 0;

static const bbe::List<uint32_t> indices = {
	 0,  1,  3,	//Bottom
	 1,  2,  3,
	 5,  4,  7,	//Top
	 6,  5,  7,
	 9,  8, 11,	//Left
	10,  9, 11,
	12, 13, 15,	//Right
	13, 14, 15,
	16, 17, 19,	//Front
	17, 18, 19,
	21, 20, 23,	//Back
	22, 21, 23,
};

void bbe::INTERNAL::openGl::OpenGLCube::init()
{
	const bbe::List<PosNormalPair> vertices = {
		PosNormalPair{Vector3( 0.5, -0.5, -0.5), Vector3(0, 0, -1)},
		PosNormalPair{Vector3( 0.5,  0.5, -0.5), Vector3(0, 0, -1)},
		PosNormalPair{Vector3(-0.5,  0.5, -0.5), Vector3(0, 0, -1)},
		PosNormalPair{Vector3(-0.5, -0.5, -0.5), Vector3(0, 0, -1)},

		PosNormalPair{Vector3( 0.5, -0.5,  0.5), Vector3(0, 0,  1)},
		PosNormalPair{Vector3( 0.5,  0.5,  0.5), Vector3(0, 0,  1)},
		PosNormalPair{Vector3(-0.5,  0.5,  0.5), Vector3(0, 0,  1)},
		PosNormalPair{Vector3(-0.5, -0.5,  0.5), Vector3(0, 0,  1)},

		PosNormalPair{Vector3( 0.5, -0.5, -0.5), Vector3(0, -1, 0)},
		PosNormalPair{Vector3( 0.5, -0.5,  0.5), Vector3(0, -1, 0)},
		PosNormalPair{Vector3(-0.5, -0.5,  0.5), Vector3(0, -1, 0)},
		PosNormalPair{Vector3(-0.5, -0.5, -0.5), Vector3(0, -1, 0)},

		PosNormalPair{Vector3( 0.5,  0.5, -0.5), Vector3(0,  1, 0)},
		PosNormalPair{Vector3( 0.5,  0.5,  0.5), Vector3(0,  1, 0)},
		PosNormalPair{Vector3(-0.5,  0.5,  0.5), Vector3(0,  1, 0)},
		PosNormalPair{Vector3(-0.5,  0.5, -0.5), Vector3(0,  1, 0)},

		PosNormalPair{Vector3(-0.5,  0.5, -0.5), Vector3(-1, 0, 0)},
		PosNormalPair{Vector3(-0.5,  0.5,  0.5), Vector3(-1, 0, 0)},
		PosNormalPair{Vector3(-0.5, -0.5,  0.5), Vector3(-1, 0, 0)},
		PosNormalPair{Vector3(-0.5, -0.5, -0.5), Vector3(-1, 0, 0)},

		PosNormalPair{Vector3( 0.5,  0.5, -0.5), Vector3( 1, 0, 0)},
		PosNormalPair{Vector3( 0.5,  0.5,  0.5), Vector3( 1, 0, 0)},
		PosNormalPair{Vector3( 0.5, -0.5,  0.5), Vector3( 1, 0, 0)},
		PosNormalPair{Vector3( 0.5, -0.5, -0.5), Vector3( 1, 0, 0)},
	};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PosNormalPair) * vertices.getLength(), vertices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.getLength(), indices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void bbe::INTERNAL::openGl::OpenGLCube::destroy()
{
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
}

GLuint bbe::INTERNAL::openGl::OpenGLCube::getVbo()
{
	return vbo;
}

GLuint bbe::INTERNAL::openGl::OpenGLCube::getIbo()
{
	return ibo;
}

size_t bbe::INTERNAL::openGl::OpenGLCube::getAmountOfIndices()
{
	return indices.getLength();
}

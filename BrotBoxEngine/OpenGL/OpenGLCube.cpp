#include "BBE/OpenGL/OpenGLCube.h"
#include "BBE/OpenGL/OpenGLManager.h"
#include "BBE/List.h"
#include "BBE/Vector3.h"

static GLuint vbo = 0;
static GLuint ibo = 0;
static size_t amountOfIndices = 0;

void bbe::INTERNAL::openGl::OpenGLCube::init()
{
	const bbe::List<PosNormalPair> vertices = bbe::Cube::getRenderVerticesDefault();
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PosNormalPair) * vertices.getLength(), vertices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	const bbe::List<uint32_t> indices = bbe::Cube::getRenderIndicesDefault();
	amountOfIndices = indices.getLength();
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
	return Cube::getRenderIndicesDefault().getLength();
}

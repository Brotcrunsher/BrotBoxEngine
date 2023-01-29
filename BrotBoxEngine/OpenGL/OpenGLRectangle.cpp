#include "BBE/OpenGL/OpenGLRectangle.h"
#include "BBE/OpenGL/OpenGLManager.h"
#include "BBE/List.h"
#include "BBE/Vector3.h"

static GLuint vbo = 0;
static GLuint ibo = 0;

static const bbe::List<uint32_t> indices = { 0, 1, 3, 2 };

void bbe::INTERNAL::openGl::OpenGLRectangle::init()
{
	const bbe::List<Vector2> vertices = {
		{0, 0},
		{0, 1},
		{1, 1},
		{1, 0},
	};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * vertices.getLength(), vertices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.getLength(), indices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void bbe::INTERNAL::openGl::OpenGLRectangle::destroy()
{
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
}

GLuint bbe::INTERNAL::openGl::OpenGLRectangle::getVbo()
{
	return vbo;
}

GLuint bbe::INTERNAL::openGl::OpenGLRectangle::getIbo()
{
	return ibo;
}

size_t bbe::INTERNAL::openGl::OpenGLRectangle::getAmountOfIndices()
{
	return indices.getLength();
}

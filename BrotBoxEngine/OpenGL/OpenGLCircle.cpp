#include "BBE/OpenGL/OpenGLCircle.h"
#include "BBE/OpenGL/OpenGLManager.h"
#include "BBE/List.h"
#include "BBE/Vector3.h"
#include "BBE/Circle.h"

static GLuint vbo = 0;
static GLuint ibo = 0;

static bbe::List<uint32_t> indices;

void bbe::INTERNAL::openGl::OpenGLCircle::init()
{
	bbe::Circle circle(0, 0, 1, 1);
	bbe::List<bbe::Vector2> vertices;
	circle.getVertices(vertices);

	for (uint32_t i = 0; i < (uint32_t)vertices.getLength(); i++)
	{
		indices.add(i);
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * vertices.getLength(), vertices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.getLength(), indices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void bbe::INTERNAL::openGl::OpenGLCircle::destroy()
{
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
}

GLuint bbe::INTERNAL::openGl::OpenGLCircle::getVbo()
{
	return vbo;
}

GLuint bbe::INTERNAL::openGl::OpenGLCircle::getIbo()
{
	return ibo;
}

size_t bbe::INTERNAL::openGl::OpenGLCircle::getAmountOfIndices()
{
	return indices.getLength();
}

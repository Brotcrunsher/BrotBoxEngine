#include "BBE/OpenGL/OpenGLModel.h"

bbe::INTERNAL::openGl::OpenGLModel::OpenGLModel(const bbe::Model& model)
{
	model.m_prendererData = this;

	const bbe::List<PosNormalPair>& vertices = model.m_vertices;
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PosNormalPair) * vertices.getLength(), vertices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	const bbe::List<uint32_t>& indices = model.m_indices;
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.getLength(), indices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_amountOfIndices = indices.getLength();
}

bbe::INTERNAL::openGl::OpenGLModel::~OpenGLModel()
{
	glDeleteBuffers(1, &m_ibo);
	glDeleteBuffers(1, &m_vbo);
}

GLuint bbe::INTERNAL::openGl::OpenGLModel::getVbo()
{
	return m_vbo;
}

GLuint bbe::INTERNAL::openGl::OpenGLModel::getIbo()
{
	return m_ibo;
}

size_t bbe::INTERNAL::openGl::OpenGLModel::getAmountOfIndices()
{
	return m_amountOfIndices;
}

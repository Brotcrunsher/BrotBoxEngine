#include "stdafx.h"
#include "BBE/Cube.h"


bbe::INTERNAL::vulkan::VulkanBuffer bbe::Cube::s_indexBuffer;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::Cube::s_vertexBuffer;

void bbe::Cube::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	s_initVertexBuffer(device, physicalDevice, commandPool, queue);
	s_initIndexBuffer(device, physicalDevice, commandPool, queue);
}

void bbe::Cube::s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	uint32_t indices[] = {
		0, 1, 3,	//Bottom
		1, 2, 3,
		2, 1, 5,	//Back
		2, 5, 6,
		3, 2, 6,	//Left
		3, 6, 7,
		3, 4, 0,	//Front
		3, 7, 4,
		0, 5, 1,	//Right
		0, 4, 5,
		7, 6, 5,	//Up
		7, 5, 4
	};

	s_indexBuffer.create(device, physicalDevice, sizeof(uint32_t) * 12 * 3, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void* dataBuf = s_indexBuffer.map();
	memcpy(dataBuf, indices, sizeof(uint32_t) * 12 * 3);
	s_indexBuffer.unmap();

	s_indexBuffer.upload(commandPool, queue);
}

void bbe::Cube::s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	Vector3 vertices[] = {
		Vector3(0.5 , -0.5, 0),
		Vector3(0.5 , 0.5 , 0),
		Vector3(-0.5, 0.5 , 0),
		Vector3(-0.5, -0.5, 0),

		Vector3(0.5 , -0.5, 1),
		Vector3(0.5 , 0.5 , 1),
		Vector3(-0.5, 0.5 , 1),
		Vector3(-0.5, -0.5, 1)
	};

	s_vertexBuffer.create(device, physicalDevice, sizeof(Vector3) * 8, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	void* dataBuf = s_vertexBuffer.map();
	memcpy(dataBuf, vertices, sizeof(Vector3) * 8);
	s_vertexBuffer.unmap();

	s_vertexBuffer.upload(commandPool, queue);
}

void bbe::Cube::s_destroy()
{
	s_indexBuffer.destroy();
	s_vertexBuffer.destroy();
}

bbe::Cube::Cube()
{
}

bbe::Cube::Cube(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	set(pos, scale, rotationVector, radians);
}

bbe::Cube::Cube(const Matrix4 & transform)
{
	m_transform = transform;
}


void bbe::Cube::set(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	m_bufferDirty = true;
	Matrix4 matTranslation = Matrix4::createTranslationMatrix(pos);
	Matrix4 matScale = Matrix4::createScaleMatrix(scale);
	Matrix4 matRotation = Matrix4::createRotationMatrix(radians, rotationVector);

	m_transform = matTranslation * matRotation * matScale;
}

bbe::Vector3 bbe::Cube::getPos() const
{
	return m_transform.extractTranslation();
}

float bbe::Cube::getX() const
{
	return getPos().x;
}

float bbe::Cube::getY() const
{
	return getPos().y;
}

float bbe::Cube::getZ() const
{
	return getPos().z;
}

bbe::Vector3 bbe::Cube::getScale() const
{
	return m_transform.extractScale();
}

float bbe::Cube::getWidth() const
{
	return getScale().x;
}

float bbe::Cube::getHeight() const
{
	return getScale().z;
}

float bbe::Cube::getDepth() const
{
	return getScale().y;
}

bbe::Matrix4 bbe::Cube::getTransform() const
{
	return m_transform;
}

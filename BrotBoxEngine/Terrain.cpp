#include "stdafx.h"
#include "BBE/Terrain.h"
#include "BBE/VertexWithNormal.h"
#include "BBE/Random.h"
#include "BBE/ValueNoise2D.h"


bbe::INTERNAL::vulkan::VulkanBuffer bbe::Terrain::s_indexBuffer;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::Terrain::s_vertexBuffer;
int bbe::Terrain::s_numberOfVertices = 0;

static const int WIDTH = 3000;
static const int HEIGHT = 3000;

void bbe::Terrain::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	s_initVertexBuffer(device, physicalDevice, commandPool, queue);
	s_initIndexBuffer(device, physicalDevice, commandPool, queue);
}

void bbe::Terrain::s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	List<uint32_t> indices;

	for (int i = 0; i < WIDTH - 1; i++)
	{
		for (int k = 0; k < HEIGHT - 1; k++)
		{
			indices.add(k * WIDTH + i);
			indices.add(k * WIDTH + i + 1);
			indices.add(k * WIDTH + i + WIDTH + 1);

			indices.add(k * WIDTH + i);
			indices.add(k * WIDTH + i + WIDTH + 1);
			indices.add(k * WIDTH + i + WIDTH);
		}
	}

	s_indexBuffer.create(device, physicalDevice, sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void *dataBuf = s_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
	s_indexBuffer.unmap();

	s_indexBuffer.upload(commandPool, queue);

	s_numberOfVertices = indices.getLength();
}

void bbe::Terrain::s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	List<VertexWithNormal> vertices;
	Random rand;
	ValueNoise2D valueNoise;
	valueNoise.create(WIDTH, HEIGHT);

	for (int i = 0; i < HEIGHT; i++)
	{
		for (int k = 0; k < WIDTH; k++)
		{
			float height = valueNoise.get(i, k);
			vertices.add(VertexWithNormal(Vector3(i / 2.0f, k / 2.0f, height * 100.0f), Vector3(0, 0, 1)));
		}
	}

	s_vertexBuffer.create(device, physicalDevice, sizeof(VertexWithNormal) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	void* dataBuf = s_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(VertexWithNormal) * vertices.getLength());
	s_vertexBuffer.unmap();

	s_vertexBuffer.upload(commandPool, queue);
}

void bbe::Terrain::s_destroy()
{
	s_indexBuffer.destroy();
	s_vertexBuffer.destroy();
}

bbe::Terrain::Terrain()
{
}

bbe::Matrix4 bbe::Terrain::getTransform() const
{
	return m_transform;
}

void bbe::Terrain::setTransform(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	m_bufferDirty = true;
	Matrix4 matTranslation = Matrix4::createTranslationMatrix(pos);
	Matrix4 matScale = Matrix4::createScaleMatrix(scale);
	Matrix4 matRotation = Matrix4::createRotationMatrix(radians, rotationVector);

	m_transform = matTranslation * matRotation * matScale;
}

void bbe::Terrain::setTransform(const Matrix4 & transform)
{
	m_transform = transform;
}

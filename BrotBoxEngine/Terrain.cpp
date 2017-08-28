#include "stdafx.h"
#include "BBE/Terrain.h"
#include "BBE/VertexWithNormal.h"
#include "BBE/Random.h"
#include "BBE/ValueNoise2D.h"


VkDevice         bbe::Terrain::s_device         = VK_NULL_HANDLE;
VkPhysicalDevice bbe::Terrain::s_physicalDevice = VK_NULL_HANDLE;
VkQueue          bbe::Terrain::s_queue          = VK_NULL_HANDLE;
bbe::INTERNAL::vulkan::VulkanCommandPool *bbe::Terrain::s_pcommandPool = nullptr;

void bbe::Terrain::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	s_device = device;
	s_physicalDevice = physicalDevice;
	s_queue = queue;
	s_pcommandPool = &commandPool;
}

void bbe::Terrain::init() const
{
	if (m_created)
	{
		return;
	}

	initIndexBuffer();
	initVertexBuffer();

	m_created = true;
}

void bbe::Terrain::initIndexBuffer() const
{
	List<uint32_t> indices;

	for (int i = 0; i < m_width - 1; i++)
	{
		for (int k = 0; k < m_height; k++)
		{
			indices.add(k * m_width + i);
			indices.add(k * m_width + i + 1);
		}
		indices.add(0xFFFFFFFF);
	}

	m_indexBuffer.create(s_device, s_physicalDevice, sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void *dataBuf = m_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
	m_indexBuffer.unmap();

	m_indexBuffer.upload(*s_pcommandPool, s_queue);

	m_numberOfVertices = indices.getLength();
}

void bbe::Terrain::initVertexBuffer() const
{
	List<VertexWithNormal> vertices;
	Random rand;
	ValueNoise2D valueNoise;
	valueNoise.create(m_width, m_height);

	for (int i = 0; i < m_height; i++)
	{
		for (int k = 0; k < m_width; k++)
		{
			float height = valueNoise.get(i, k);
			//float height = 0;
			vertices.add(VertexWithNormal(Vector3(i / 2.0f, k / 2.0f, height * 100.0f), Vector3(0, 0, 1)));
		}
	}

	m_vertexBuffer.create(s_device, s_physicalDevice, sizeof(VertexWithNormal) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	void* dataBuf = m_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(VertexWithNormal) * vertices.getLength());
	m_vertexBuffer.unmap();

	m_vertexBuffer.upload(*s_pcommandPool, s_queue);
}

void bbe::Terrain::destroy()
{
	m_indexBuffer.destroy();
	m_vertexBuffer.destroy();
}

bbe::Terrain::Terrain(int width, int height)
	: m_width(width), m_height(height)
{
}

bbe::Terrain::~Terrain()
{
	destroy();
}

bbe::Matrix4 bbe::Terrain::getTransform() const
{
	return m_transform;
}

void bbe::Terrain::setTransform(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	Matrix4 matTranslation = Matrix4::createTranslationMatrix(pos);
	Matrix4 matScale = Matrix4::createScaleMatrix(scale);
	Matrix4 matRotation = Matrix4::createRotationMatrix(radians, rotationVector);

	m_transform = matTranslation * matRotation * matScale;
}

void bbe::Terrain::setTransform(const Matrix4 & transform)
{
	m_transform = transform;
}

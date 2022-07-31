#include "BBE/Vulkan/VulkanCube.h"
#include "BBE/List.h"
#include "BBE/VertexWithNormal.h"
#include "BBE/Math.h"

bbe::INTERNAL::vulkan::VulkanBuffer bbe::INTERNAL::vulkan::VulkanCube::s_indexBuffer;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::INTERNAL::vulkan::VulkanCube::s_vertexBuffer;
uint32_t bbe::INTERNAL::vulkan::VulkanCube::amountOfIndices = 0;

void bbe::INTERNAL::vulkan::VulkanCube::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue)
{
	s_initVertexBuffer(device, physicalDevice, commandPool, queue);
	s_initIndexBuffer(device, physicalDevice, commandPool, queue);
}

void bbe::INTERNAL::vulkan::VulkanCube::s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue)
{
	bbe::List<uint32_t> indices = {
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

	amountOfIndices = indices.getLength();

	s_indexBuffer.create(device, physicalDevice, sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void* dataBuf = s_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
	s_indexBuffer.unmap();

	s_indexBuffer.upload(commandPool, queue);
}

void bbe::INTERNAL::vulkan::VulkanCube::s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue)
{
	bbe::List<INTERNAL::VertexWithNormal> vertices = {
		INTERNAL::VertexWithNormal(Vector3(0.5, -0.5, -0.5), Vector3(0, 0, -1)),
		INTERNAL::VertexWithNormal(Vector3(0.5,  0.5, -0.5), Vector3(0, 0, -1)),
		INTERNAL::VertexWithNormal(Vector3(-0.5,  0.5, -0.5), Vector3(0, 0, -1)),
		INTERNAL::VertexWithNormal(Vector3(-0.5, -0.5, -0.5), Vector3(0, 0, -1)),

		INTERNAL::VertexWithNormal(Vector3(0.5, -0.5,  0.5), Vector3(0, 0,  1)),
		INTERNAL::VertexWithNormal(Vector3(0.5,  0.5,  0.5), Vector3(0, 0,  1)),
		INTERNAL::VertexWithNormal(Vector3(-0.5,  0.5,  0.5), Vector3(0, 0,  1)),
		INTERNAL::VertexWithNormal(Vector3(-0.5, -0.5,  0.5), Vector3(0, 0,  1)),

		INTERNAL::VertexWithNormal(Vector3(0.5, -0.5, -0.5), Vector3(0, -1, 0)),
		INTERNAL::VertexWithNormal(Vector3(0.5, -0.5,  0.5), Vector3(0, -1, 0)),
		INTERNAL::VertexWithNormal(Vector3(-0.5, -0.5,  0.5), Vector3(0, -1, 0)),
		INTERNAL::VertexWithNormal(Vector3(-0.5, -0.5, -0.5), Vector3(0, -1, 0)),

		INTERNAL::VertexWithNormal(Vector3(0.5,  0.5, -0.5), Vector3(0,  1, 0)),
		INTERNAL::VertexWithNormal(Vector3(0.5,  0.5,  0.5), Vector3(0,  1, 0)),
		INTERNAL::VertexWithNormal(Vector3(-0.5,  0.5,  0.5), Vector3(0,  1, 0)),
		INTERNAL::VertexWithNormal(Vector3(-0.5,  0.5, -0.5), Vector3(0,  1, 0)),

		INTERNAL::VertexWithNormal(Vector3(-0.5,  0.5, -0.5), Vector3(-1, 0, 0)),
		INTERNAL::VertexWithNormal(Vector3(-0.5,  0.5,  0.5), Vector3(-1, 0, 0)),
		INTERNAL::VertexWithNormal(Vector3(-0.5, -0.5,  0.5), Vector3(-1, 0, 0)),
		INTERNAL::VertexWithNormal(Vector3(-0.5, -0.5, -0.5), Vector3(-1, 0, 0)),

		INTERNAL::VertexWithNormal(Vector3(0.5,  0.5, -0.5), Vector3(1, 0, 0)),
		INTERNAL::VertexWithNormal(Vector3(0.5,  0.5,  0.5), Vector3(1, 0, 0)),
		INTERNAL::VertexWithNormal(Vector3(0.5, -0.5,  0.5), Vector3(1, 0, 0)),
		INTERNAL::VertexWithNormal(Vector3(0.5, -0.5, -0.5), Vector3(1, 0, 0)),
	};

	for (size_t i = 0; i < vertices.getLength(); i++)
	{
		vertices[i].m_normal = bbe::Math::interpolateLinear(vertices[i].m_pos, vertices[i].m_normal, 0.5f);

		vertices[i].m_normal = vertices[i].m_normal.normalize();
	}

	s_vertexBuffer.create(device, physicalDevice, sizeof(INTERNAL::VertexWithNormal) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	void* dataBuf = s_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(INTERNAL::VertexWithNormal) * vertices.getLength());
	s_vertexBuffer.unmap();

	s_vertexBuffer.upload(commandPool, queue);
}

void bbe::INTERNAL::vulkan::VulkanCube::s_destroy()
{
	s_indexBuffer.destroy();
	s_vertexBuffer.destroy();
}

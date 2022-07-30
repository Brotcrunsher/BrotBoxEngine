#include "../BBE/Vulkan/VulkanCircle.h"
#include "../BBE/Vector2.h"

const uint32_t bbe::INTERNAL::vulkan::VulkanCircle::AMOUNTOFVERTICES = 32;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::INTERNAL::vulkan::VulkanCircle::s_indexBuffer;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::INTERNAL::vulkan::VulkanCircle::s_vertexBuffer;

void bbe::INTERNAL::vulkan::VulkanCircle::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue)
{
	s_initVertexBuffer(device, physicalDevice, commandPool, queue);
	s_initIndexBuffer(device, physicalDevice, commandPool, queue);
}

void bbe::INTERNAL::vulkan::VulkanCircle::s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue)
{
	List<uint32_t> indices;
	for (uint32_t i = 1; i < AMOUNTOFVERTICES - 1; i++)
	{
		indices.add(0);
		indices.add(i);
		indices.add(i + 1);
	}

	s_indexBuffer.create(device, physicalDevice, sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void* dataBuf = s_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
	s_indexBuffer.unmap();

	s_indexBuffer.upload(commandPool, queue);
}

void bbe::INTERNAL::vulkan::VulkanCircle::s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue)
{
	List<Vector2> vertices;
	for (std::size_t i = 0; i < AMOUNTOFVERTICES; i++)
	{
		vertices.add(Vector2::createVector2OnUnitCircle((float)i / (float)AMOUNTOFVERTICES * 2 * Math::PI) / 2 + Vector2(0.5f, 0.5f));
	}

	s_vertexBuffer.create(device, physicalDevice, sizeof(Vector2) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	void* dataBuf = s_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(Vector2) * vertices.getLength());
	s_vertexBuffer.unmap();

	s_vertexBuffer.upload(commandPool, queue);
}

void bbe::INTERNAL::vulkan::VulkanCircle::s_destroy()
{
	s_indexBuffer.destroy();
	s_vertexBuffer.destroy();
}


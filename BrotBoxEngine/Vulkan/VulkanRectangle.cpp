#include "../BBE/Vulkan/VulkanRectangle.h"

bbe::INTERNAL::vulkan::VulkanBuffer bbe::INTERNAL::vulkan::VulkanRectangle::s_indexBuffer;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::INTERNAL::vulkan::VulkanRectangle::s_vertexBuffer;

void bbe::INTERNAL::vulkan::VulkanRectangle::s_init(VkDevice device, VkPhysicalDevice physicalDevice, bbe::INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue)
{
	s_initVertexBuffer(device, physicalDevice, commandPool, queue);
	s_initIndexBuffer(device, physicalDevice, commandPool, queue);
}

void bbe::INTERNAL::vulkan::VulkanRectangle::s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue)
{
	s_indexBuffer.create(device, physicalDevice, sizeof(uint32_t) * 6, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	uint32_t data[] = {
		0, 1, 2, 0, 2, 3
	};
	void* dataBuf = s_indexBuffer.map();
	memcpy(dataBuf, data, sizeof(uint32_t) * 6);
	s_indexBuffer.unmap();

	s_indexBuffer.upload(commandPool, queue);
}

void bbe::INTERNAL::vulkan::VulkanRectangle::s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue)
{
	s_vertexBuffer.create(device, physicalDevice, sizeof(float) * 8, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	float data[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
	};

	void* dataBuf = s_vertexBuffer.map();
	memcpy(dataBuf, data, sizeof(float) * 8);
	s_vertexBuffer.unmap();

	s_vertexBuffer.upload(commandPool, queue);
}

void bbe::INTERNAL::vulkan::VulkanRectangle::s_destroy()
{
	s_indexBuffer.destroy();
	s_vertexBuffer.destroy();
}

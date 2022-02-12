#include "BBE/VulkanHelper.h"
#include "BBE/List.h"
#include "BBE/VulkanRenderPass.h"
#include "BBE/VulkanCommandPool.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanSwapchain.h"
#include "BBE/VulkanPipeline.h"

bbe::INTERNAL::vulkan::VulkanCommandPool::VulkanCommandPool()
{
}

void bbe::INTERNAL::vulkan::VulkanCommandPool::init(const VulkanDevice & device)
{
	m_device = device.getDevice();

	VkCommandPoolCreateInfo cpci = {};
	cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpci.pNext = nullptr;
	cpci.flags = 0;
	cpci.queueFamilyIndex = device.getQueueFamilyIndex();

	VkResult result = vkCreateCommandPool(m_device, &cpci, nullptr, &m_commandPool);
	ASSERT_VULKAN(result);
}

void bbe::INTERNAL::vulkan::VulkanCommandPool::destroy()
{
	if (m_commandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		m_commandPool = VK_NULL_HANDLE;
	}
}

VkCommandPool bbe::INTERNAL::vulkan::VulkanCommandPool::getCommandPool() const
{
	return m_commandPool;
}

VkCommandBuffer bbe::INTERNAL::vulkan::VulkanCommandPool::getCommandBuffer()
{
	VkCommandBufferAllocateInfo cbai;
	cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbai.pNext = nullptr;
	cbai.commandPool = m_commandPool;
	cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbai.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VkResult result = vkAllocateCommandBuffers(m_device, &cbai, &commandBuffer);
	ASSERT_VULKAN(result);

	return commandBuffer;
}

void bbe::INTERNAL::vulkan::VulkanCommandPool::freeCommandBuffer(VkCommandBuffer commandBuffer)
{
	vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

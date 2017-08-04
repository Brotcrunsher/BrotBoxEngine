#include "stdafx.h"
#include "VulkanFence.h"
#include "VulkanDevice.h"
#include "VulkanHelper.h"
#include "Exceptions.h"

bbe::INTERNAL::vulkan::VulkanFence::VulkanFence()
{
	//DO NOTHING
}

void bbe::INTERNAL::vulkan::VulkanFence::init(const VulkanDevice & vulkanDevice)
{
	m_device = vulkanDevice.getDevice();

	VkFenceCreateInfo fci = {};
	fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fci.pNext = nullptr;
	fci.flags = 0;
	VkResult result = vkCreateFence(m_device, &fci, nullptr, &m_fence);
	ASSERT_VULKAN(result);
}

void bbe::INTERNAL::vulkan::VulkanFence::destroy()
{
	if (m_fence != VK_NULL_HANDLE)
	{
		vkDestroyFence(m_device, m_fence, nullptr);
		m_device = VK_NULL_HANDLE;
		m_fence = VK_NULL_HANDLE;
	}
}

void bbe::INTERNAL::vulkan::VulkanFence::waitForFence(uint64_t timeout)
{
	if (m_fence == VK_NULL_HANDLE)
	{
		throw NotInitializedException();
	}

	VkResult result = vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, timeout);
	ASSERT_VULKAN(result);
	
	result = vkResetFences(m_device, 1, &m_fence);
	ASSERT_VULKAN(result);
}

VkFence bbe::INTERNAL::vulkan::VulkanFence::getFence()
{
	if (m_fence == VK_NULL_HANDLE)
	{
		throw NotInitializedException();
	}

	return m_fence;
}

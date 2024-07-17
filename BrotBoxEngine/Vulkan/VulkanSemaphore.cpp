#include "BBE/Vulkan/VulkanSemaphore.h"
#include "BBE/Vulkan/VulkanDevice.h"
#include "BBE/Vulkan/VulkanHelper.h"
#include "BBE/Error.h"

bbe::INTERNAL::vulkan::VulkanSemaphore::VulkanSemaphore()
{
	//DO NOTHING
}

void bbe::INTERNAL::vulkan::VulkanSemaphore::init(const VulkanDevice & device)
{
	if (m_semaphore != VK_NULL_HANDLE)
	{
		bbe::Crash(bbe::Error::AlreadyCreated);
	}

	m_device = device.getDevice();

	VkSemaphoreCreateInfo sci = {};
	sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	sci.pNext = nullptr;
	sci.flags = 0;

	VkResult result = vkCreateSemaphore(m_device, &sci, nullptr, &m_semaphore);
	ASSERT_VULKAN(result);
}

void bbe::INTERNAL::vulkan::VulkanSemaphore::destroy()
{
	if (m_semaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(m_device, m_semaphore, nullptr);
		m_device = VK_NULL_HANDLE;
		m_semaphore = VK_NULL_HANDLE;
	}
}

VkSemaphore bbe::INTERNAL::vulkan::VulkanSemaphore::getSemaphore()
{
	if (m_semaphore == VK_NULL_HANDLE)
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}

	return m_semaphore;
}

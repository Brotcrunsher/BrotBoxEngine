#include "stdafx.h"
#include "VulkanSurface.h"
#include "VulkanHelper.h"
#include "VulkanInstance.h"

bbe::INTERNAL::vulkan::VulkanSurface::VulkanSurface()
{
}

void bbe::INTERNAL::vulkan::VulkanSurface::destroy()
{
	if (m_surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(m_instance->getInstance(), m_surface, nullptr);
		m_surface = VK_NULL_HANDLE;
	}

}

void bbe::INTERNAL::vulkan::VulkanSurface::init(VulkanInstance & instance, GLFWwindow * window)
{
	m_instance = &instance;
	VkResult result = glfwCreateWindowSurface(instance.getInstance(), window, nullptr, &m_surface);
	ASSERT_VULKAN(result);
}

VkSurfaceKHR bbe::INTERNAL::vulkan::VulkanSurface::getSurface() const
{
	return m_surface;
}
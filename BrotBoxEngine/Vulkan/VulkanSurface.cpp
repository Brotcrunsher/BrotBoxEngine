#include "BBE/Vulkan/VulkanSurface.h"
#include "BBE/Vulkan/VulkanHelper.h"
#include "BBE/Vulkan/VulkanInstance.h"

bbe::INTERNAL::vulkan::VulkanSurface::VulkanSurface()
{
}

void bbe::INTERNAL::vulkan::VulkanSurface::destroy()
{
	if (m_surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(m_pinstance->getInstance(), m_surface, nullptr);
		m_surface = VK_NULL_HANDLE;
	}

}

void bbe::INTERNAL::vulkan::VulkanSurface::init(VulkanInstance & instance, GLFWwindow * window)
{
	m_pinstance = &instance;
	VkResult result = glfwCreateWindowSurface(instance.getInstance(), window, nullptr, &m_surface);
	ASSERT_VULKAN(result);
}

VkSurfaceKHR bbe::INTERNAL::vulkan::VulkanSurface::getSurface() const
{
	return m_surface;
}
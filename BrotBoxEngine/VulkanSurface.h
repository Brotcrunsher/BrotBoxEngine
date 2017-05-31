#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "VulkanHelper.h"
#include "VulkanInstance.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanSurface
			{
			private:
				VkSurfaceKHR m_surface = VK_NULL_HANDLE;
				Instance* m_instance = nullptr;

			public:
				VulkanSurface()
				{
					//DO NOTHING
				}

				~VulkanSurface()
				{
					vkDestroySurfaceKHR(m_instance->getInstance(), m_surface, nullptr);
				}

				void init(Instance &instance, GLFWwindow *window) {
					m_instance = &instance;
					VkResult result = glfwCreateWindowSurface(instance.getInstance(), window, nullptr, &m_surface);
					ASSERT_VULKAN(result);
				}

				VulkanSurface(const VulkanSurface& other) = delete;
				VulkanSurface(VulkanSurface&& other) = delete;
				VulkanSurface& operator=(const VulkanSurface& other) = delete;
				VulkanSurface& operator=(VulkanSurface&& other) = delete;

				VkSurfaceKHR getSurface() const
				{
					return m_surface;
				}
			};
		}
	}
}
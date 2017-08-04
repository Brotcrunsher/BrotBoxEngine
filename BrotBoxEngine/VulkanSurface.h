#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanInstance;

			class VulkanSurface
			{
			private:
				VkSurfaceKHR m_surface = VK_NULL_HANDLE;
				VulkanInstance* m_instance = nullptr;

			public:
				VulkanSurface();


				void destroy();

				void init(VulkanInstance &instance, GLFWwindow *window);

				VulkanSurface(const VulkanSurface& other) = delete;
				VulkanSurface(VulkanSurface&& other) = delete;
				VulkanSurface& operator=(const VulkanSurface& other) = delete;
				VulkanSurface& operator=(VulkanSurface&& other) = delete;

				VkSurfaceKHR getSurface() const;
			};
		}
	}
}
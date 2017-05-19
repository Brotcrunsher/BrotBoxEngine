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
			class Surface
			{
			private:
				VkSurfaceKHR m_surface = VK_NULL_HANDLE;

			public:
				Surface()
				{
					//DO NOTHING
				}

				void init(const Instance &instance, GLFWwindow *window) {
					VkResult result = glfwCreateWindowSurface(instance.getInstance(), window, nullptr, &m_surface);
					ASSERT_VULKAN(result);
				}

				Surface(const Surface& other) = delete;
				Surface(Surface&& other) = delete;
				Surface& operator=(const Surface& other) = delete;
				Surface& operator=(Surface&& other) = delete;

				VkSurfaceKHR getSurface() const
				{
					return m_surface;
				}
			};
		}
	}
}
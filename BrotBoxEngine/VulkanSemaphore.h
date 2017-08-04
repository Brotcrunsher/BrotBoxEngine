#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;

			class VulkanSemaphore
			{
			private:
				VkSemaphore m_semaphore = VK_NULL_HANDLE;
				VkDevice m_device = VK_NULL_HANDLE;

			public:
				VulkanSemaphore();

				void init(const VulkanDevice &device);

				void destroy();

				VkSemaphore getSemaphore();
			};
		}
	}
}
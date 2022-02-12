#pragma once

#include "GLFW/glfw3.h"

#include <stdint.h>
#include <limits>


namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;

			class VulkanFence
			{
			private:
				VkFence  m_fence  = VK_NULL_HANDLE;
				VkDevice m_device = VK_NULL_HANDLE;

			public:
				VulkanFence();

				void init(const VulkanDevice &vulkanDevice);
				void destroy();

				void waitForFence(uint64_t timeout = std::numeric_limits<uint64_t>::max());
				void reset();

				VkFence getFence();
			};
		}
	}
}

#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include <stdint.h>
#include "../BBE/List.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanSwapchain;
			class VulkanPipeline;
			class VulkanRenderPass;
			class VulkanSwapchain;

			class VulkanCommandPool
			{
			private:
				VkDevice m_device           = VK_NULL_HANDLE;
				VkCommandPool m_commandPool = VK_NULL_HANDLE;

			public:
				VulkanCommandPool();

				void init(const VulkanDevice &device);

				void destroy();

				VkCommandPool getCommandPool() const;

				VkCommandBuffer getCommandBuffer();
				void freeCommandBuffer(VkCommandBuffer commandBuffer);
			};
		}
	}
}

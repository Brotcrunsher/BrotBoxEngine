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

			class VulkanRenderPass
			{
			private:
				VkRenderPass m_renderPass = VK_NULL_HANDLE;
				VkDevice m_device = VK_NULL_HANDLE;

			public:
				VulkanRenderPass();

				void destroy();

				void init(const VulkanDevice &device);

				VkRenderPass getRenderPass() const;
			};
		}
	}
}

#pragma once

#include "../BBE/Vulkan/VulkanBuffer.h"
#include "../BBE/Vulkan/VulkanCommandPool.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanRectangle
			{
			public:
				static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue);
				static void s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue);
				static void s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue);
				static void s_destroy();
				static bbe::INTERNAL::vulkan::VulkanBuffer s_indexBuffer;
				static bbe::INTERNAL::vulkan::VulkanBuffer s_vertexBuffer;
			};
		}
	}
}

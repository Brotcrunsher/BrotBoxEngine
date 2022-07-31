#pragma once

#include "../BBE/Vulkan/VulkanBuffer.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanSphere
			{
			public:
				static uint32_t amountOfVertices;
				static uint32_t amountOfIndices;

				static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue);
				static void s_destroy();
				static bbe::INTERNAL::vulkan::VulkanBuffer s_indexBuffer;
				static bbe::INTERNAL::vulkan::VulkanBuffer s_vertexBuffer;
			};
		}
	}
}

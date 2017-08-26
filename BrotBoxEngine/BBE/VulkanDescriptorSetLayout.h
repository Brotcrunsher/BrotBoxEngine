#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/List.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanDescriptorPool;

			class VulkanDescriptorSetLayout
			{
				friend class VulkanDescriptorPool;
			private:
				VkDevice              m_device              = VK_NULL_HANDLE;
				VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

				List<VkDescriptorSetLayoutBinding> m_bindings;
			public:
				void addBinding(const VkDescriptorSetLayoutBinding &dslb);
				void addBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers = nullptr);

				void create(const VulkanDevice &device);
				void destroy();

				VkDescriptorSetLayout getDescriptorSetLayout() const;
			};
		}
	}
}
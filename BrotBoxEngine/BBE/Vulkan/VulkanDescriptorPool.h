#pragma once

#include "GLFW/glfw3.h"
#include "../BBE/List.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanDescriptorSetLayout;

			class VulkanDescriptorPoolSetLayoutContainer
			{
			public:
				VulkanDescriptorPoolSetLayoutContainer(const VulkanDescriptorSetLayout* vulkanDescriptorSetLayout, uint32_t amountOfSets);

				const VulkanDescriptorSetLayout* m_pvulkanDescriptorSetLayout;
				uint32_t m_amountOfSets;
			};

			class VulkanDescriptorPool
			{
			private:
				VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
				VkDevice         m_device         = VK_NULL_HANDLE;

				List<VulkanDescriptorPoolSetLayoutContainer> m_setLayouts;
			public:

				void addVulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &dsl, uint32_t amountOfSets);

				void create(const VulkanDevice &device);
				void destroy();

				VkDescriptorPool getDescriptorPool() const;
			};
		}
	}
}

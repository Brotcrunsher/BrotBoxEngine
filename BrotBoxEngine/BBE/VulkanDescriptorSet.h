#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "../BBE/List.h"

namespace bbe
{
	class Image;
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDescriptorPool;
			class VulkanDescriptorSetLayout;
			class VulkanDevice;
			class VulkanBuffer;

			class AdvancedDescriptorBufferInfo
			{
			public:
				AdvancedDescriptorBufferInfo(const VkDescriptorBufferInfo &descriptorBufferInfo, uint32_t binding);
				VkDescriptorBufferInfo m_descriptorBufferInfo;
				uint32_t m_binding;
			};

			class AdvancedDescriptorImageInfo
			{
			public:
				AdvancedDescriptorImageInfo(const VkDescriptorImageInfo &descriptorImageInfo, uint32_t binding);
				VkDescriptorImageInfo m_descriptorImageInfo;
				uint32_t m_binding;
			};

			class VulkanDescriptorSet
			{
			private:
				VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

				List<AdvancedDescriptorBufferInfo> m_descriptorBufferInfos;
				List<AdvancedDescriptorImageInfo> m_descriptorImageInfos;
			public:
				void addUniformBuffer(const VulkanBuffer &buffer, VkDeviceSize offset, uint32_t binding);
				void addCombinedImageSampler(const Image& image, uint32_t binding);
				void create(const VulkanDevice &device, const VulkanDescriptorPool &descriptorPool, const VulkanDescriptorSetLayout &setLayout);
				void update(const VulkanDevice &device);

				VkDescriptorSet getDescriptorSet() const;
				const VkDescriptorSet* getPDescriptorSet() const;
			};
		}
	}
}

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
			class VulkanBuffer;

			class VulkanDescriptorPool
			{
			private:
				class AdvancedBufferInfo
				{
				public:
					AdvancedBufferInfo(VkDescriptorBufferInfo dbi, uint32_t binding);

					VkDescriptorBufferInfo m_descriptorBufferInfo;
					uint32_t m_binding;
				};

				List<VkDescriptorPoolSize> m_descriptorPoolSizes;
				List<VkDescriptorSetLayoutBinding> m_layoutBindings;
				List<AdvancedBufferInfo> m_descriptorBufferInfo;

				VkDescriptorPool      m_descriptorPool      = VK_NULL_HANDLE;
				VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
				VkDescriptorSet       m_descriptorSet       = VK_NULL_HANDLE;
				VkDevice              m_device              = VK_NULL_HANDLE;

			public:

				void addPoolSize(VkDescriptorPoolSize dps);
				void addPoolSize(VkDescriptorType type, uint32_t descriptorCount);

				void addLayoutBinding(VkDescriptorSetLayoutBinding dslb);
				void addLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers = nullptr);

				void addDescriptorBufferInfo(VkDescriptorBufferInfo dbi, uint32_t binding);
				void addDescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding);
				void addDescriptorBufferInfo(const VulkanBuffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding);

				void create(VkDevice device);
				void destroy();

				VkDescriptorSetLayout getLayout() const;
			};
		}
	}
}
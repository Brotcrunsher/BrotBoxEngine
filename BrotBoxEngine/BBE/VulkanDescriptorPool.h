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
					AdvancedBufferInfo(VkDescriptorType type, VkShaderStageFlags shaderStage, const VulkanBuffer &buffer, VkDeviceSize offset, uint32_t binding, uint32_t setIndex);

					VkDescriptorType m_type;
					VkShaderStageFlags m_shaderStage;
					VkBuffer m_buffer;
					VkDeviceSize m_offset;
					VkDeviceSize m_bufferSize;
					uint32_t m_binding;
					uint32_t m_setIndex;
				};

				List<AdvancedBufferInfo> m_descriptorBufferInfos;

				VkDescriptorPool      m_descriptorPool      = VK_NULL_HANDLE;
				VkDevice              m_device              = VK_NULL_HANDLE;
				List<VkDescriptorSetLayout> m_descriptorSetLayout;
				List<VkDescriptorSet>       m_descriptorSets;

			public:

				void addDescriptor(VkDescriptorType type, VkShaderStageFlags shaderStage, const VulkanBuffer &buffer, VkDeviceSize offset, uint32_t binding, uint32_t setIndex);

				void create(VkDevice device);
				void destroy();

				VkDescriptorSetLayout getLayout(size_t setNumber) const;
				VkDescriptorSet* getPSet(size_t setNumber);
			};
		}
	}
}
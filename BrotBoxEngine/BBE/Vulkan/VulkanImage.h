#pragma once

#include "../BBE/ManuallyRefCountable.h"
#include "GLFW/glfw3.h"
#include "../BBE/Vulkan/VulkanDescriptorSet.h"
#include "../BBE/Vulkan/VulkanHelper.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanCommandPool;
			class VulkanManager;
			class VulkanDescriptorSet;

			struct VulkanImage : public ManuallyRefCountable
			{
				int32_t        m_refCount = 0;
				VkImage        m_image = VK_NULL_HANDLE;
				VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
				VkImageView    m_imageView = VK_NULL_HANDLE;
				std::unique_ptr<VkImageLayout[]> m_imageLayout = nullptr;
				VkDevice       m_device = VK_NULL_HANDLE;
				VkSampler      m_sampler = VK_NULL_HANDLE;
				VkFormat       m_format = VK_FORMAT_UNDEFINED;
				int32_t        m_width = 0;
				int32_t        m_height = 0;
				INTERNAL::vulkan::VulkanDescriptorSet m_descriptorSet;
				INTERNAL::vulkan::VulkanDescriptorSet* m_pCorrectDescriptorSet;

				VulkanImage(const bbe::Image& image, const INTERNAL::vulkan::VulkanDevice& device, const INTERNAL::vulkan::VulkanCommandPool& commandPool, const INTERNAL::vulkan::VulkanDescriptorPool& descriptorPool, const INTERNAL::vulkan::VulkanDescriptorSetLayout& setLayout, const Image* parentImage = nullptr);
				~VulkanImage();

				VulkanImage(const VulkanImage&) = delete;
				VulkanImage(VulkanImage&&) = delete;
				VulkanImage& operator =(const VulkanImage&) = delete;
				VulkanImage&& operator ==(const VulkanImage&&) = delete;

				void changeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImageLayout layout, uint32_t baseMipLevel = 0, uint32_t levelCount = 1) const;
				void writeBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer) const;

				VkSampler getSampler() const;
				VkImageView getImageView() const;
				VkImageLayout getImageLayout() const;

				INTERNAL::vulkan::VulkanDescriptorSet& getDescriptorSet() const;
			};
		}
	}
}

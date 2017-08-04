#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "VulkanHelper.h"
#include "VulkanCommandPool.h"
#include "List.h"
#include "Exceptions.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VWDepthImage
			{
			private:
				VkImage m_image = VK_NULL_HANDLE;
				VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
				VkImageView m_imageView = VK_NULL_HANDLE;
				VkDevice m_device = VK_NULL_HANDLE;
				bool m_created = false;

			public:
				VWDepthImage();


				VWDepthImage(const VWDepthImage&) = delete;
				VWDepthImage(VWDepthImage&&) = delete;
				VWDepthImage& operator=(const VWDepthImage &) = delete;
				VWDepthImage& operator=(VWDepthImage&&) = delete;

				void create(const VulkanDevice &device, const VulkanCommandPool &commandPool, uint32_t width, uint32_t height);

				void destroy();

				VkImageView getImageView() const;

				static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

				static VkAttachmentDescription getDepthAttachment(VkPhysicalDevice physicalDevice);

				static VkPipelineDepthStencilStateCreateInfo getDepthStencilStateCreateInfoOpaque();
			};
		}
	}
}
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
			class VWDepthImage;
			class VulkanSurface;
			class VulkanDevice;
			class VulkanRenderPass;

			class VulkanSwapchain
			{
			private:
				VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
				VkDevice m_device          = VK_NULL_HANDLE;
				uint32_t m_amountOfImages  = 0;

				VkImage *m_pswapchainImages = nullptr;
				VkImageView *m_pimageViews  = nullptr;
				bbe::List<VkFramebuffer> frameBuffers;

				uint32_t m_width = 0;
				uint32_t m_height = 0;

			public:
				VulkanSwapchain();

				void destroy();

				VulkanSwapchain(const VulkanSwapchain& other)            = delete;
				VulkanSwapchain(VulkanSwapchain&& other)                 = delete;
				VulkanSwapchain& operator=(const VulkanSwapchain& other) = delete;
				VulkanSwapchain& operator=(VulkanSwapchain&& other)      = delete;

				void init(const VulkanSurface &surface, const VulkanDevice &device, uint32_t width, uint32_t height, VulkanSwapchain *oldSwapchain = nullptr);

				void createFramebuffers(const VWDepthImage &depthImage, const VulkanRenderPass &renderPass);

				uint32_t getAmountOfImages() const;

				VkFramebuffer getFrameBuffer(size_t index) const;

				VkSwapchainKHR getSwapchain() const;

			private:
				void createImageViews(VkFormat deviceFormat);

				
			};
		}
	}
}

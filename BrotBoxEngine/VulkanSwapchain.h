#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "VulkanSurface.h"
#include "VulkanDevice.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanSwapchain
			{
			private:
				VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
				VkDevice m_device = VK_NULL_HANDLE;
				uint32_t amountOfImagesInSwapchain = 0;

				VkImage *m_pswapchainImages = nullptr;
				VkImageView *m_pimageViews = nullptr;

			public:
				VulkanSwapchain()
				{
					//DO NOTHING
				}

				~VulkanSwapchain()
				{
					for (uint32_t i = 0; i < amountOfImagesInSwapchain; i++) {
						vkDestroyImageView(m_device, m_pimageViews[i], nullptr);
					}

					if (m_pimageViews != nullptr) {
						delete[] m_pimageViews;
						m_pimageViews = nullptr;
					}
					if (m_pswapchainImages != nullptr) {
						delete[] m_pswapchainImages;
						m_pswapchainImages = nullptr;
					}
					vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
				}

				VulkanSwapchain(const VulkanSwapchain& other)            = delete;
				VulkanSwapchain(VulkanSwapchain&& other)                 = delete;
				VulkanSwapchain& operator=(const VulkanSwapchain& other) = delete;
				VulkanSwapchain& operator=(VulkanSwapchain&& other)      = delete;

				void init(const VulkanSurface &surface, const VulkanDevice &device, uint32_t width, uint32_t height, VulkanSwapchain *oldSwapchain = nullptr)
				{
					m_device = device.getDevice();

					VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
					swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
					swapchainCreateInfo.pNext = nullptr;
					swapchainCreateInfo.flags = 0;
					swapchainCreateInfo.surface = surface.getSurface();
					swapchainCreateInfo.minImageCount = 3;
					swapchainCreateInfo.imageFormat = device.getFormat();
					swapchainCreateInfo.imageColorSpace = device.getColorSpace();
					swapchainCreateInfo.imageExtent = VkExtent2D{ width, height };
					swapchainCreateInfo.imageArrayLayers = 1;
					swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
					VulkanSharingBehaviour vsb = device.getSharingBehaviour();
					swapchainCreateInfo.imageSharingMode = vsb.m_sharingMode;
					swapchainCreateInfo.queueFamilyIndexCount = (uint32_t)vsb.m_queueFamilyIndices.getLength();
					swapchainCreateInfo.pQueueFamilyIndices = vsb.m_queueFamilyIndices.getRaw();
					swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
					swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
					swapchainCreateInfo.presentMode = device.getPresentMode();
					swapchainCreateInfo.clipped = VK_TRUE;
					swapchainCreateInfo.oldSwapchain = (oldSwapchain == nullptr) ? VK_NULL_HANDLE : oldSwapchain->m_swapchain;

					VkResult result = vkCreateSwapchainKHR(device.getDevice(), &swapchainCreateInfo, nullptr, &m_swapchain);
					ASSERT_VULKAN(result);

					createImageViews(device.getFormat());
				}

			private:
				void createImageViews(VkFormat deviceFormat) {
					vkGetSwapchainImagesKHR(m_device, m_swapchain, &amountOfImagesInSwapchain, nullptr);
					m_pswapchainImages = new VkImage[amountOfImagesInSwapchain];
					VkResult result = vkGetSwapchainImagesKHR(m_device, m_swapchain, &amountOfImagesInSwapchain, m_pswapchainImages);
					ASSERT_VULKAN(result);

					m_pimageViews = new VkImageView[amountOfImagesInSwapchain];
					for (uint32_t i = 0; i < amountOfImagesInSwapchain; i++) {
						VkImageViewCreateInfo imageViewCreateInfo = {};
						imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
						imageViewCreateInfo.pNext = nullptr;
						imageViewCreateInfo.flags = 0;
						imageViewCreateInfo.image = m_pswapchainImages[i];
						imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
						imageViewCreateInfo.format = deviceFormat;
						imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
						imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
						imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
						imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
						imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
						imageViewCreateInfo.subresourceRange.levelCount = 1;
						imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
						imageViewCreateInfo.subresourceRange.layerCount = 1;

						result = vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_pimageViews[i]);
						ASSERT_VULKAN(result);
					}
				}
			};
		}
	}
}
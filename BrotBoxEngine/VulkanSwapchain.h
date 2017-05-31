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

			public:
				VulkanSwapchain()
				{
					//DO NOTHING
				}

				~VulkanSwapchain()
				{
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
					swapchainCreateInfo.queueFamilyIndexCount = vsb.m_queueFamilyIndices.getLength();
					swapchainCreateInfo.pQueueFamilyIndices = vsb.m_queueFamilyIndices.getRaw();
					swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
					swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
					swapchainCreateInfo.presentMode = device.getPresentMode();
					swapchainCreateInfo.clipped = VK_TRUE;
					swapchainCreateInfo.oldSwapchain = (oldSwapchain == nullptr) ? VK_NULL_HANDLE : oldSwapchain->m_swapchain;

					VkResult result = vkCreateSwapchainKHR(device.getDevice(), &swapchainCreateInfo, nullptr, &m_swapchain);
					ASSERT_VULKAN(result);
				}
			};
		}
	}
}
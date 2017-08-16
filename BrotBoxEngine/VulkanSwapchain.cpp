#include "stdafx.h"
#include "BBE/VulkanSwapchain.h"
#include "BBE/VulkanSurface.h"
#include "BBE/VulkanRenderPass.h"
#include "BBE/VWDepthImage.h"
#include "BBE/VulkanDevice.h"

bbe::INTERNAL::vulkan::VulkanSwapchain::VulkanSwapchain()
{
}

void bbe::INTERNAL::vulkan::VulkanSwapchain::destroy()
{
	if (m_swapchain != VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < m_amountOfImages; i++)
		{
			vkDestroyFramebuffer(m_device, frameBuffers[i], nullptr);
		}

		for (uint32_t i = 0; i < m_amountOfImages; i++) {
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
		m_swapchain = VK_NULL_HANDLE;
	}
	
}

void bbe::INTERNAL::vulkan::VulkanSwapchain::init(const VulkanSurface & surface, const VulkanDevice & device, uint32_t width, uint32_t height, VulkanSwapchain * oldSwapchain)
{
	m_device = device.getDevice();
	m_width = width;
	m_height = height;

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

void bbe::INTERNAL::vulkan::VulkanSwapchain::createFramebuffers(const VWDepthImage & depthImage, const VulkanRenderPass & renderPass)
{
	for (uint32_t i = 0; i < m_amountOfImages; i++)
	{
		bbe::List<VkImageView> attachmentViews = { 
			m_pimageViews[i],
			depthImage.getImageView()
		};

		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = renderPass.getRenderPass();
		framebufferCreateInfo.attachmentCount = attachmentViews.getLength();
		framebufferCreateInfo.pAttachments = attachmentViews.getRaw();
		framebufferCreateInfo.width = m_width;
		framebufferCreateInfo.height = m_height;
		framebufferCreateInfo.layers = 1;

		VkFramebuffer frameBuffer;

		VkResult result = vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &frameBuffer);
		ASSERT_VULKAN(result);

		frameBuffers.add(frameBuffer);
	}
}

uint32_t bbe::INTERNAL::vulkan::VulkanSwapchain::getAmountOfImages() const
{
	return m_amountOfImages;
}

VkFramebuffer bbe::INTERNAL::vulkan::VulkanSwapchain::getFrameBuffer(size_t index) const
{
	return frameBuffers[index];
}

VkSwapchainKHR bbe::INTERNAL::vulkan::VulkanSwapchain::getSwapchain() const
{
	if (m_swapchain == VK_NULL_HANDLE)
	{
		throw NotInitializedException();
	}
	return m_swapchain;
}

void bbe::INTERNAL::vulkan::VulkanSwapchain::createImageViews(VkFormat deviceFormat)
{
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_amountOfImages, nullptr);
	m_pswapchainImages = new VkImage[m_amountOfImages];
	VkResult result = vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_amountOfImages, m_pswapchainImages);
	ASSERT_VULKAN(result);

	m_pimageViews = new VkImageView[m_amountOfImages];
	for (uint32_t i = 0; i < m_amountOfImages; i++) {
		createImageView(m_device, m_pswapchainImages[i], deviceFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_pimageViews[i]);
	}
}
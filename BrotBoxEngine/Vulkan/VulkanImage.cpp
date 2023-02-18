#include "BBE/Vulkan/VulkanImage.h"
#include "BBE/Vulkan/VulkanDevice.h"
#include "BBE/Vulkan/VulkanCommandPool.h"
#include "BBE/Vulkan/VulkanDescriptorPool.h"
#include "BBE/Vulkan/VulkanDescriptorSetLayout.h"
#include "BBE/Vulkan/VulkanBuffer.h"
#include "BBE/Image.h"

bbe::INTERNAL::vulkan::VulkanImage::VulkanImage(const bbe::Image& image, const INTERNAL::vulkan::VulkanDevice& device, const INTERNAL::vulkan::VulkanCommandPool& commandPool, const INTERNAL::vulkan::VulkanDescriptorPool& descriptorPool, const INTERNAL::vulkan::VulkanDescriptorSetLayout& setLayout, const Image* parentImage)
{
	if (image.m_pdata == nullptr)
	{
		throw NotInitializedException();
	}

	if (image.m_prendererData != nullptr)
	{
		throw IllegalStateException();
	}

	image.m_prendererData = this;

	m_format = (VkFormat)image.m_format;
	m_width = image.getWidth();
	m_height = image.getHeight();

	if (image.m_parentImage == nullptr)
	{
		m_pCorrectDescriptorSet = &m_descriptorSet;
	}
	else
	{
		m_pCorrectDescriptorSet = &(((VulkanImage*)image.m_parentImage->m_prendererData.get())->m_descriptorSet);
	}

	m_device = device.getDevice();
	const int amountOfMips = Math::max(1, Math::log2Floor(Math::min(image.getWidth(), image.getHeight())));
	m_imageLayout = std::make_unique<VkImageLayout[]>(amountOfMips); //TODO use allocator
	for (int i = 0; i < amountOfMips; i++)
	{
		m_imageLayout[i] = VK_IMAGE_LAYOUT_PREINITIALIZED;
	}

	VkDeviceSize imageSize = image.getSizeInBytes();

	INTERNAL::vulkan::VulkanBuffer stagingBuffer;
	stagingBuffer.create(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	void* data = stagingBuffer.map();
	memcpy(data, image.m_pdata, imageSize);
	stagingBuffer.unmap();


	INTERNAL::vulkan::createImage(
		m_device,
		device.getPhysicalDevice(),
		image.getWidth(), image.getHeight(),
		(VkFormat)image.m_format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_image, m_imageMemory,
		amountOfMips);

	changeLayout(m_device, commandPool.getCommandPool(), device.getQueue(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	writeBufferToImage(m_device, commandPool.getCommandPool(), device.getQueue(), stagingBuffer.getBuffer());
	changeLayout(m_device, commandPool.getCommandPool(), device.getQueue(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	for (int i = 1; i < amountOfMips; i++)
	{
		VkImageBlit ib = {};
		ib.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ib.srcSubresource.mipLevel = i - 1;
		ib.srcSubresource.baseArrayLayer = 0;
		ib.srcSubresource.layerCount = 1;
		ib.srcOffsets[1].x = image.getWidth() >> (i - 1);
		ib.srcOffsets[1].y = image.getHeight() >> (i - 1);
		ib.srcOffsets[1].z = 1;

		ib.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ib.dstSubresource.mipLevel = i;
		ib.dstSubresource.baseArrayLayer = 0;
		ib.dstSubresource.layerCount = 1;
		ib.dstOffsets[1].x = image.getWidth() >> i;
		ib.dstOffsets[1].y = image.getHeight() >> i;
		ib.dstOffsets[1].z = 1;

		changeLayout(m_device, commandPool.getCommandPool(), device.getQueue(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, i);

		VkCommandBuffer commandBuffer = INTERNAL::vulkan::startSingleTimeCommandBuffer(device.getDevice(), commandPool.getCommandPool());
		vkCmdBlitImage(commandBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ib, VK_FILTER_LINEAR);
		INTERNAL::vulkan::endSingleTimeCommandBuffer(device.getDevice(), device.getQueue(), commandPool.getCommandPool(), commandBuffer);

		changeLayout(m_device, commandPool.getCommandPool(), device.getQueue(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, i);
	}

	changeLayout(m_device, commandPool.getCommandPool(), device.getQueue(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, amountOfMips);

	stagingBuffer.destroy();

	VkComponentSwizzle swizR = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentSwizzle swizG = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentSwizzle swizB = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentSwizzle swizA = VK_COMPONENT_SWIZZLE_IDENTITY;
	if (image.m_format == ImageFormat::R8)
	{
		swizR = VK_COMPONENT_SWIZZLE_R;
		swizG = VK_COMPONENT_SWIZZLE_R;
		swizB = VK_COMPONENT_SWIZZLE_R;
		swizA = VK_COMPONENT_SWIZZLE_R;
	}

	INTERNAL::vulkan::createImageView(m_device, m_image, (VkFormat)image.m_format, VK_IMAGE_ASPECT_COLOR_BIT, m_imageView, amountOfMips, swizR, swizG, swizB, swizA);

	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = (VkFilter)image.m_filterMode;
	samplerCreateInfo.minFilter = (VkFilter)image.m_filterMode;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = (VkSamplerAddressMode)image.m_repeatMode;
	samplerCreateInfo.addressModeV = (VkSamplerAddressMode)image.m_repeatMode;
	samplerCreateInfo.addressModeW = (VkSamplerAddressMode)image.m_repeatMode;
	samplerCreateInfo.mipLodBias = 0.0f;
	//samplerCreateInfo.anisotropyEnable = VK_TRUE;
	//samplerCreateInfo.maxAnisotropy = 16;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = static_cast<float>(amountOfMips);
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	VkResult result = vkCreateSampler(m_device, &samplerCreateInfo, nullptr, &m_sampler);
	ASSERT_VULKAN(result);

	image.m_parentImage = parentImage;

	getDescriptorSet().addCombinedImageSampler(image, 0);
	if (image.m_parentImage == nullptr)
	{
		getDescriptorSet().create(device, descriptorPool, setLayout);
	}
}

bbe::INTERNAL::vulkan::VulkanImage::~VulkanImage()
{
	if (m_sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(m_device, m_sampler, nullptr);
		vkDestroyImageView(m_device, m_imageView, nullptr);

		vkDestroyImage(m_device, m_image, nullptr);
		vkFreeMemory(m_device, m_imageMemory, nullptr);

		m_descriptorSet.destroy();

		m_image = VK_NULL_HANDLE;
		m_imageMemory = VK_NULL_HANDLE;
		m_imageView = VK_NULL_HANDLE;
		m_imageLayout = nullptr;
		m_device = VK_NULL_HANDLE;
		m_sampler = VK_NULL_HANDLE;
	}
}

void bbe::INTERNAL::vulkan::VulkanImage::changeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImageLayout layout, uint32_t baseMipLevel, uint32_t levelCount) const
{
	INTERNAL::vulkan::changeImageLayout(device, commandPool, queue, m_image, m_format, m_imageLayout[baseMipLevel], layout, baseMipLevel, levelCount);

	m_imageLayout[baseMipLevel] = layout;
}

void bbe::INTERNAL::vulkan::VulkanImage::writeBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer) const
{
	VkCommandBuffer commandBuffer = INTERNAL::vulkan::startSingleTimeCommandBuffer(device, commandPool);

	VkBufferImageCopy bufferImageCopy = {};
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = 0;
	bufferImageCopy.bufferImageHeight = 0;
	bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopy.imageSubresource.mipLevel = 0;
	bufferImageCopy.imageSubresource.baseArrayLayer = 0;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageOffset = { 0, 0, 0 };
	bufferImageCopy.imageExtent = { (uint32_t)m_width, (uint32_t)m_height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

	INTERNAL::vulkan::endSingleTimeCommandBuffer(device, queue, commandPool, commandBuffer);
}

VkSampler bbe::INTERNAL::vulkan::VulkanImage::getSampler() const
{
	return m_sampler;
}

VkImageView bbe::INTERNAL::vulkan::VulkanImage::getImageView() const
{
	return m_imageView;
}

VkImageLayout bbe::INTERNAL::vulkan::VulkanImage::getImageLayout() const
{
	return m_imageLayout[0];
}

bbe::INTERNAL::vulkan::VulkanDescriptorSet& bbe::INTERNAL::vulkan::VulkanImage::getDescriptorSet() const
{
	return *m_pCorrectDescriptorSet;
}

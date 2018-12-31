#include "stdafx.h"
#include "BBE/Image.h"
#include "BBE/Exceptions.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanCommandPool.h"
#include "BBE/VulkanDescriptorPool.h"
#include "BBE/VulkanDescriptorSetLayout.h"
#include "BBE/VulkanBuffer.h"
#include "BBE/Math.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void bbe::Image::createAndUpload(const INTERNAL::vulkan::VulkanDevice & device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, const INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool, const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayout, const Image* parentImage) const
{
	if (wasUploadedToVulkan)
	{
		return;
	}

	if (m_pdata == nullptr)
	{
		throw NotInitializedException();
	}

	m_device = device.getDevice();
	int amountOfMips = Math::log2Floor(Math::max(getWidth(), getHeight())) + 1;
	m_imageLayout = std::make_unique<VkImageLayout[]>(amountOfMips); //TODO use allocator
	for (int i = 0; i < amountOfMips; i++)
	{
		m_imageLayout[i] = VK_IMAGE_LAYOUT_PREINITIALIZED;
	}

	VkDeviceSize imageSize = getSizeInBytes();

	INTERNAL::vulkan::VulkanBuffer stagingBuffer;
	stagingBuffer.create(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	
	void *data = stagingBuffer.map();
	memcpy(data, m_pdata, imageSize);
	stagingBuffer.unmap();


	INTERNAL::vulkan::createImage(
		m_device, 
		device.getPhysicalDevice(), 
		getWidth(), getHeight(), 
		(VkFormat)m_format, 
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
		ib.srcOffsets[1].x = getWidth() >> (i - 1);
		ib.srcOffsets[1].y = getHeight() >> (i - 1);
		ib.srcOffsets[1].z = 1;

		ib.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ib.dstSubresource.mipLevel = i;
		ib.dstSubresource.baseArrayLayer = 0;
		ib.dstSubresource.layerCount = 1;
		ib.dstOffsets[1].x = getWidth() >> i;
		ib.dstOffsets[1].y = getHeight() >> i;
		ib.dstOffsets[1].z = 1;

		changeLayout(m_device, commandPool.getCommandPool(), device.getQueue(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, i);

		VkCommandBuffer commandBuffer = INTERNAL::vulkan::startSingleTimeCommandBuffer(device.getDevice(), commandPool.getCommandPool());
		vkCmdBlitImage(commandBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ib, VK_FILTER_LINEAR);
		INTERNAL::vulkan::endSingleTimeCommandBuffer(device.getDevice(), device.getQueue(), commandPool.getCommandPool(), commandBuffer);

		changeLayout(m_device, commandPool.getCommandPool(), device.getQueue(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, i);
	}
	
	changeLayout(m_device, commandPool.getCommandPool(), device.getQueue(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, amountOfMips);

	stagingBuffer.destroy();

	INTERNAL::vulkan::createImageView(m_device, m_image, (VkFormat)m_format, VK_IMAGE_ASPECT_COLOR_BIT, m_imageView, amountOfMips);

	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = (VkFilter)m_filterMode;
	samplerCreateInfo.minFilter = (VkFilter)m_filterMode;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = (VkSamplerAddressMode)m_repeatMode;
	samplerCreateInfo.addressModeV = (VkSamplerAddressMode)m_repeatMode;
	samplerCreateInfo.addressModeW = (VkSamplerAddressMode)m_repeatMode;
	samplerCreateInfo.mipLodBias = 0.0f;
	//samplerCreateInfo.anisotropyEnable = VK_TRUE;
	//samplerCreateInfo.maxAnisotropy = 16;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = amountOfMips;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	VkResult result = vkCreateSampler(m_device, &samplerCreateInfo, nullptr, &m_sampler);
	ASSERT_VULKAN(result);

	m_parentImage = parentImage;

	getDescriptorSet().addCombinedImageSampler(*this, 0);
	if (m_parentImage == nullptr)
	{
		getDescriptorSet().create(device, descriptorPool, setLayout);
	}

	wasUploadedToVulkan = true;
}

void bbe::Image::changeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImageLayout layout, uint32_t baseMipLevel, uint32_t levelCount) const
{
	INTERNAL::vulkan::changeImageLayout(device, commandPool, queue, m_image, (VkFormat)m_format, this->m_imageLayout[baseMipLevel], layout, baseMipLevel, levelCount);

	this->m_imageLayout[baseMipLevel] = layout;
}

void bbe::Image::writeBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer) const
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
	bufferImageCopy.imageExtent = { (uint32_t)getWidth(), (uint32_t)getHeight(), 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

	INTERNAL::vulkan::endSingleTimeCommandBuffer(device, queue, commandPool, commandBuffer);
}

VkSampler bbe::Image::getSampler() const
{
	return m_sampler;
}

VkImageView bbe::Image::getImageView() const
{
	return m_imageView;
}

VkImageLayout bbe::Image::getImageLayout() const
{
	return m_imageLayout[0];
}

bbe::INTERNAL::vulkan::VulkanDescriptorSet & bbe::Image::getDescriptorSet() const
{
	if (m_parentImage == nullptr)
	{
		return m_descriptorrSet;
	}
	else
	{
		return m_parentImage->m_descriptorrSet;
	}
}

bbe::Image::Image()
{
}

bbe::Image::Image(const char * path)
{
	load(path);
}

bbe::Image::Image(int width, int height)
{
	load(width, height);
}

bbe::Image::Image(int width, int height, const Color & c)
{
	load(width, height, c);
}

bbe::Image::Image(int width, int height, const float * data, ImageFormat format)
{
	load(width, height, data, format);
}

bbe::Image::~Image()
{
	destroy();
}

void bbe::Image::load(const char * path)
{
	if (m_pdata != nullptr)
	{
		throw AlreadyCreatedException();
	}

	int texChannels = 0;
	stbi_uc *pixels = stbi_load(path, &m_width, &m_height, &texChannels, STBI_rgb_alpha);
	m_format = ImageFormat::R8G8B8A8;

	if (pixels == nullptr)
	{
		throw LoadException();
	}

	m_pdata = new byte[getSizeInBytes()]; //TODO use allocator
	memcpy(m_pdata, pixels, getSizeInBytes());

	stbi_image_free(pixels);
}

void bbe::Image::load(int width, int height)
{
	load(width, height, Color());
}

void bbe::Image::load(int width, int height, const Color & c)
{
	m_width = width;
	m_height = height;
	m_format = ImageFormat::R8G8B8A8;

	m_pdata = new byte[getSizeInBytes()]; //TODO use allocator
	for (int i = 0; i < getSizeInBytes(); i+=4)
	{
		m_pdata[i + 0] = (byte)(c.r * 255);
		m_pdata[i + 1] = (byte)(c.g * 255);
		m_pdata[i + 2] = (byte)(c.b * 255);
		m_pdata[i + 3] = (byte)(c.a * 255);
	}
}

void bbe::Image::load(int width, int height, const float * data, ImageFormat format)
{
	m_width = width;
	m_height = height;
	m_format = format;

	m_pdata = new byte[getSizeInBytes()];
	if (format == ImageFormat::R8)
	{
		for (int i = 0; i < getSizeInBytes(); i++)
		{
			m_pdata[i] = (byte)(data[i] * 255);
		}
	}
	else if (format == ImageFormat::R32FLOAT)
	{
		for (int i = 0; i < getSizeInBytes(); i += 4)
		{
			byte *ptr = (byte*)(data + i / 4);
			m_pdata[i + 0] = ptr[0];
			m_pdata[i + 1] = ptr[1];
			m_pdata[i + 2] = ptr[2];
			m_pdata[i + 3] = ptr[3];
		}
	}
	else
	{
		throw FormatNotSupportedException();
	}
	
}

void bbe::Image::destroy()
{
	if (m_pdata != nullptr)
	{
		delete[] m_pdata;
		m_pdata = nullptr;
		m_width = 0;
		m_height = 0;
		wasUploadedToVulkan = false;
	}

	if(m_sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(m_device, m_sampler, nullptr);
		vkDestroyImageView(m_device, m_imageView, nullptr);

		vkDestroyImage(m_device, m_image, nullptr);
		vkFreeMemory(m_device, m_imageMemory, nullptr);

		m_image       = VK_NULL_HANDLE;
		m_imageMemory = VK_NULL_HANDLE;
		m_imageView   = VK_NULL_HANDLE;
		m_imageLayout = nullptr;
		m_device      = VK_NULL_HANDLE;
		m_sampler     = VK_NULL_HANDLE;
	}
}

int bbe::Image::getWidth() const
{
	return m_width;
}

int bbe::Image::getHeight() const
{
	return m_height;
}

int bbe::Image::getSizeInBytes() const
{
	return getWidth() * getHeight() * getAmountOfChannels() * getBytesPerChannel();
}

int bbe::Image::getAmountOfChannels() const
{
	switch (m_format)
	{
	case ImageFormat::R8:
		return 1;
	case ImageFormat::R8G8B8A8:
		return 4;
	case ImageFormat::R32FLOAT:
		return 1;
	default:
		throw FormatNotSupportedException();
	}
}

int bbe::Image::getBytesPerChannel() const
{
	switch (m_format)
	{
	case ImageFormat::R8:
		return 1;
	case ImageFormat::R8G8B8A8:
		return 1;
	case ImageFormat::R32FLOAT:
		return 4;
	default:
		throw FormatNotSupportedException();
	}
}

bbe::Color bbe::Image::getPixel(int x, int y) const
{
	if (m_pdata == nullptr)
	{
		throw NotInitializedException();
	}

	int index = (y * m_width + x) * getAmountOfChannels();
	switch(m_format)
	{
	case ImageFormat::R8:
		return Color(m_pdata[index] / 255.f, m_pdata[index] / 255.f, m_pdata[index] / 255.f, 1.0f);
	case ImageFormat::R8G8B8A8:
		return Color(m_pdata[index] / 255.f, m_pdata[index + 1] / 255.f, m_pdata[index + 2] / 255.f, m_pdata[index + 3] / 255.f);
	default:
		throw FormatNotSupportedException();
	}
	
}

bbe::ImageRepeatMode bbe::Image::getRepeatMode() const
{
	return m_repeatMode;
}

void bbe::Image::setRepeatMode(ImageRepeatMode irm)
{
	if (wasUploadedToVulkan)
	{
		throw AlreadyUploadedException();
	}

	m_repeatMode = irm;
}

bbe::ImageFilterMode bbe::Image::getFilterMode() const
{
	return m_filterMode;
}

void bbe::Image::setFilterMode(ImageFilterMode ifm)
{
	if (wasUploadedToVulkan)
	{
		throw AlreadyUploadedException();
	}

	m_filterMode = ifm;
}

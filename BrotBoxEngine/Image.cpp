#include "stdafx.h"
#include "BBE\Image.h"
#include "BBE/Exceptions.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanCommandPool.h"
#include "BBE/VulkanBuffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void bbe::Image::createAndUpload(const INTERNAL::vulkan::VulkanDevice & device, const INTERNAL::vulkan::VulkanCommandPool & commandPool)
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


	VkDeviceSize imageSize = getSizeInBytes();

	INTERNAL::vulkan::VulkanBuffer stagingBuffer;
	stagingBuffer.create(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	
	void *data = stagingBuffer.map();
	memcpy(data, m_pdata, imageSize);
	stagingBuffer.unmap();

	INTERNAL::vulkan::createImage(m_device, device.getPhysicalDevice(), getWidth(), getHeight(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);

	changeLayout(m_device, commandPool.getCommandPool(), device.getQueue(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	writeBufferToImage(m_device, commandPool.getCommandPool(), device.getQueue(), stagingBuffer.getBuffer());
	changeLayout(m_device, commandPool.getCommandPool(), device.getQueue(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	stagingBuffer.destroy();

	INTERNAL::vulkan::createImageView(m_device, m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_imageView);

	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = 16;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	VkResult result = vkCreateSampler(m_device, &samplerCreateInfo, nullptr, &m_sampler);
	ASSERT_VULKAN(result);

	wasUploadedToVulkan = true;
}

void bbe::Image::changeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImageLayout layout)
{
	INTERNAL::vulkan::changeImageLayout(device, commandPool, queue, m_image, VK_FORMAT_R8G8B8A8_UNORM, this->m_imageLayout, layout);

	this->m_imageLayout = layout;
}

void bbe::Image::writeBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer)
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
	return m_imageLayout;
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

	if (pixels == nullptr)
	{
		throw LoadException();
	}

	m_pdata = new ColorByte[m_width * m_height]; //TODO use allocator
	for (int i = 0; i < m_width; i++)
	{
		for (int k = 0; k < m_height; k++)
		{
			int index = k * m_width + i;
			m_pdata[index].r = pixels[index * 4 + 0];
			m_pdata[index].g = pixels[index * 4 + 1];
			m_pdata[index].b = pixels[index * 4 + 2];
			m_pdata[index].a = pixels[index * 4 + 3];
		}
	}

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

	m_pdata = new ColorByte[m_width * m_height]; //TODO use allocator
	ColorByte cb((byte)(c.r * 255), (byte)(c.g * 255), (byte)(c.b * 255), (byte)(c.a * 255));
	for (int i = 0; i < m_width * m_height; i++)
	{
		m_pdata[i] = cb;
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

	if(m_sampler == VK_NULL_HANDLE)
	{
		vkDestroySampler(m_device, m_sampler, nullptr);
		vkDestroyImageView(m_device, m_imageView, nullptr);

		vkDestroyImage(m_device, m_image, nullptr);
		vkFreeMemory(m_device, m_imageMemory, nullptr);

		m_image       = VK_NULL_HANDLE;
		m_imageMemory = VK_NULL_HANDLE;
		m_imageView   = VK_NULL_HANDLE;
		m_imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
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
	return getWidth() * getHeight() * 4;
}

bbe::Color bbe::Image::getPixel(int x, int y) const
{
	if (m_pdata == nullptr)
	{
		throw NotInitializedException();
	}

	int index = y * m_width + x;
	return Color(m_pdata[index].r / 255.f, m_pdata[index].g / 255.f, m_pdata[index].b / 255.f, m_pdata[index].a / 255.f);
}

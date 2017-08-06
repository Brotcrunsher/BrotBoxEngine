#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/List.h"
#include "../BBE/Exceptions.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			#define ASSERT_VULKAN(val)\
			if(val != VK_SUCCESS){\
				__debugbreak();\
			}

			uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

			bool isFormatSupported(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

			VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const bbe::List<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

			bool isStencilFormat(VkFormat format);

			void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize deviceSize, VkBufferUsageFlags bufferUsageFlags, VkBuffer &buffer, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory &deviceMemory, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* p_queueFamilyIndices);

			VkCommandBuffer startSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool);

			void endSingleTimeCommandBuffer(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);

			void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkImage &image, VkDeviceMemory &imageMemory);

			void createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView &imageView);

			void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer src, VkBuffer dest, VkDeviceSize size);

			template <typename T>
			void createAndUploadBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, bbe::List<T> data, VkBufferUsageFlags usage, VkBuffer &buffer, VkDeviceMemory &deviceMemory, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* p_queueFamilyIndices)
			{
				VkDeviceSize bufferSize = sizeof(T) * data.getLength();

				VkBuffer stagingBuffer;
				VkDeviceMemory stagingBufferMemory;
				createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr);

				void *rawData;
				vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &rawData);
				memcpy(rawData, data.getRaw(), bufferSize);
				vkUnmapMemory(device, stagingBufferMemory);

				createBuffer(device, physicalDevice, bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, deviceMemory, sharingMode, queueFamilyIndexCount, p_queueFamilyIndices);

				copyBuffer(device, commandPool, queue, stagingBuffer, buffer, bufferSize);

				vkDestroyBuffer(device, stagingBuffer, nullptr);
				vkFreeMemory(device, stagingBufferMemory, nullptr);
			}

			void changeImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		}
	}
}

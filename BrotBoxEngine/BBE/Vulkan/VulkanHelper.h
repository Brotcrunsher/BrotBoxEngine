#pragma once

#include <utility>
#include <type_traits>
#include "GLFW/glfw3.h"
#include "../BBE/List.h"
#include "../BBE/Error.h"
#include "../BBE/UtilDebug.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			#define ASSERT_VULKAN(val)\
			if(val != VK_SUCCESS){\
				bbe::debugBreak();\
			}

			// I am pretty sure that RetVal can be somehow deducted from Func, but I am too potato to do that.
			// RetVal is NOT the return value from Func (which is VkResult in most cases) but instead the last
			// argument to Func without being a pointer.
			template <typename RetVal, typename Func, typename... Args>
			bbe::List<RetVal> get_from_function(Func f, Args&&... args)
			{
				uint32_t count = 0;
				if constexpr (std::is_same<decltype(f(std::forward<Args>(args)..., &count, nullptr)), VkResult>::value)
				{
					VkResult result = f(std::forward<Args>(args)..., &count, nullptr);
					ASSERT_VULKAN(result);
				}
				else
				{
					f(std::forward<Args>(args)..., &count, nullptr);
				}
				bbe::List<RetVal> list(count);
				list.resizeCapacityAndLength(count);
				if constexpr (std::is_same<decltype(f(std::forward<Args>(args)..., &count, list.getRaw())), VkResult>::value)
				{
					VkResult result = f(std::forward<Args>(args)..., &count, list.getRaw());
					ASSERT_VULKAN(result);
				}
				else
				{
					f(std::forward<Args>(args)..., &count, list.getRaw());
				}
				return list;
			}

			uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

			bool isFormatSupported(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

			VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const bbe::List<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

			bool isStencilFormat(VkFormat format);

			void preCreateBuffer(VkDevice device,
				VkDeviceSize deviceSize,
				VkBufferUsageFlags bufferUsageFlags,
				VkBuffer & buffer,
				VkSharingMode sharingMode,
				uint32_t queueFamilyIndexCount,
				const uint32_t* p_queueFamilyIndices);
			void postCreateBuffer(
				VkDevice device,
				VkPhysicalDevice physicalDevice,
				VkBuffer & buffer,
				VkMemoryPropertyFlags memoryPropertyFlags,
				VkDeviceMemory & deviceMemory,
				VkDeviceMemory parentDeviceMemory,
				VkDeviceSize offset);
			void createBuffer(VkDevice device, 
				VkPhysicalDevice physicalDevice, 
				VkDeviceSize deviceSize, 
				VkBufferUsageFlags bufferUsageFlags, 
				VkBuffer &buffer, 
				VkMemoryPropertyFlags memoryPropertyFlags, 
				VkDeviceMemory &deviceMemory, 
				VkSharingMode sharingMode, 
				uint32_t queueFamilyIndexCount, 
				const uint32_t* p_queueFamilyIndices, 
				VkDeviceMemory parentDeviceMemory = VK_NULL_HANDLE, 
				VkDeviceSize offset = 0);

			VkCommandBuffer startSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool);

			void endSingleTimeCommandBuffer(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkCommandBuffer &commandBuffer);

			void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkImage &image, VkDeviceMemory &imageMemory, int amountOfMipLevels = 1);

			void createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView &imageView, uint32_t mipLevels = 1, 
				VkComponentSwizzle swizR = VK_COMPONENT_SWIZZLE_IDENTITY, 
				VkComponentSwizzle swizG = VK_COMPONENT_SWIZZLE_IDENTITY, 
				VkComponentSwizzle swizB = VK_COMPONENT_SWIZZLE_IDENTITY, 
				VkComponentSwizzle swizA = VK_COMPONENT_SWIZZLE_IDENTITY);

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

			void changeImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseMipLevel = 0, uint32_t levelCount = 1);
		}
	}
}

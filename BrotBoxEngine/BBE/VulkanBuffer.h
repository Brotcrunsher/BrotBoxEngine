#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanCommandPool;

			class VulkanBuffer
			{
			private:
				VkBuffer m_buffer         = VK_NULL_HANDLE;
				VkDeviceMemory m_memory   = VK_NULL_HANDLE;
				VkDeviceSize m_bufferSize = 0;
				
				VkDevice m_device                 = VK_NULL_HANDLE;
				VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
				VkBufferUsageFlags m_usage        = 0;

			public:
				bool m_wasCreated  = false;
				bool m_wasPreCreated = false;
				bool m_wasUploaded = false;
				bool m_isMapped    = false;

				void preCreate(const VulkanDevice &vulkanDevice, size_t sizeInBytes, VkBufferUsageFlags usage, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE, uint32_t queueFamilyIndexCount = 0, const uint32_t* p_queueFamilyIndices = nullptr);
				void create(const VulkanDevice &vulkanDevice, size_t sizeInBytes, VkBufferUsageFlags usage, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE, uint32_t queueFamilyIndexCount = 0, const uint32_t* p_queueFamilyIndices = nullptr, VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				void create(VkDevice vulkanDevice, VkPhysicalDevice physicalDevice, size_t sizeInBytes, VkBufferUsageFlags usage, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE, uint32_t queueFamilyIndexCount = 0, const uint32_t* p_queueFamilyIndices = nullptr, VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				void upload(const VulkanCommandPool &commandPool, VkQueue queue, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE, uint32_t queueFamilyIndexCount = 0, const uint32_t* p_queueFamilyIndices = nullptr);
				void upload(const VulkanCommandPool &commandPool, VkQueue queue, const VulkanBuffer &parentBuffer, VkDeviceSize offset, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE, uint32_t queueFamilyIndexCount = 0, const uint32_t* p_queueFamilyIndices = nullptr);
				void destroy();
				void* map();
				void unmap();
				void copy(const VulkanBuffer &other, VkCommandPool commandPool, VkQueue queue);

				VkBuffer getBuffer() const;
				VkDeviceMemory getMemory() const;
				VkDeviceSize getSize() const;

				bool isUsable();
			};
		}
	}
}

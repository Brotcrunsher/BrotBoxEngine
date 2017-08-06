#include "stdafx.h"
#include "BBE/VulkanBuffer.h"
#include "BBE/Exceptions.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanHelper.h"
#include "BBE/VulkanCommandPool.h"
#include "BBE/VulkanManager.h"

void bbe::INTERNAL::vulkan::VulkanBuffer::create(const VulkanDevice &vulkanDevice, size_t sizeInBytes, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* p_queueFamilyIndices)
{
	create(vulkanDevice.getDevice(), vulkanDevice.getPhysicalDevice(), sizeInBytes, usage, sharingMode, queueFamilyIndexCount, p_queueFamilyIndices);
}

void bbe::INTERNAL::vulkan::VulkanBuffer::create(VkDevice vulkanDevice, VkPhysicalDevice physicalDevice, size_t sizeInBytes, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t * p_queueFamilyIndices)
{
	if (sizeInBytes == 0)
	{
		throw bbe::IllegalBufferSize();
	}

	if (m_wasCreated)
	{
		throw bbe::AlreadyCreatedException();
	}

	if (m_wasUploaded)
	{
		throw bbe::AlreadyUploadedException();
	}

	m_bufferSize = sizeInBytes;
	m_device = vulkanDevice;
	m_physicalDevice = physicalDevice;
	m_usage = usage;

	createBuffer(m_device, m_physicalDevice, m_bufferSize, m_usage, m_buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_memory, sharingMode, queueFamilyIndexCount, p_queueFamilyIndices);

	m_wasCreated = true;
}

void bbe::INTERNAL::vulkan::VulkanBuffer::upload(const VulkanCommandPool &commandPool, VkQueue queue, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* p_queueFamilyIndices)
{
	if ((m_usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0)
	{
		throw BufferNoSourceException();
	}
	if (!m_wasCreated)
	{
		throw NotInitializedException();
	}
	if (m_wasUploaded)
	{
		throw BufferAlreadyUploadedException();
	}
	if (m_isMapped)
	{
		throw BufferAlreadyMappedException();
	}

	VkBuffer uploadedBuffer = VK_NULL_HANDLE;
	VkDeviceMemory uploadedMemory = VK_NULL_HANDLE;

	createBuffer(m_device, m_physicalDevice, m_bufferSize, m_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, uploadedBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uploadedMemory, sharingMode, queueFamilyIndexCount, p_queueFamilyIndices);

	copyBuffer(m_device, commandPool.getCommandPool(), queue, m_buffer, uploadedBuffer, m_bufferSize);

	vkDestroyBuffer(m_device, m_buffer, nullptr);
	vkFreeMemory(m_device, m_memory, nullptr);

	m_buffer = uploadedBuffer;
	m_memory = uploadedMemory;

	m_wasUploaded = true;
}

void bbe::INTERNAL::vulkan::VulkanBuffer::destroy()
{
	if (m_wasCreated)
	{
		if (m_isMapped)
		{
			throw BufferAlreadyMappedException();
		}
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		vkFreeMemory(m_device, m_memory, nullptr);
		m_buffer = VK_NULL_HANDLE;
		m_memory = VK_NULL_HANDLE;

		m_wasCreated = false;
		m_wasUploaded = false;
	}
}

void bbe::INTERNAL::vulkan::VulkanBuffer::destroyAtEndOfFrame()
{
	if (m_wasCreated)
	{
		VulkanManager::s_pinstance->addPendingDestructionBuffer(m_buffer, m_memory);
	}
	
}

void * bbe::INTERNAL::vulkan::VulkanBuffer::map()
{
	if (m_wasUploaded)
	{
		throw BufferAlreadyUploadedException();
	}
	if (!m_wasCreated)
	{
		throw NotInitializedException();
	}
	if (m_isMapped)
	{
		throw BufferAlreadyMappedException();
	}

	void* data;
	vkMapMemory(m_device, m_memory, 0, m_bufferSize, 0, &data);
	m_isMapped = true;

	return data;
}

void bbe::INTERNAL::vulkan::VulkanBuffer::unmap()
{
	if (!m_isMapped)
	{
		throw BufferIsNotMappedException();
	}

	vkUnmapMemory(m_device, m_memory);
	m_isMapped = false;
}

void bbe::INTERNAL::vulkan::VulkanBuffer::copy(const VulkanBuffer &other, VkCommandPool commandPool, VkQueue queue)
{
	if (!m_wasCreated)
	{
		throw NotInitializedException();
	}
	if (m_bufferSize < other.m_bufferSize)
	{
		throw BufferTooSmallException();
	}


	copyBuffer(m_device, commandPool, queue, other.m_buffer, m_buffer, m_bufferSize);
}

VkBuffer bbe::INTERNAL::vulkan::VulkanBuffer::getBuffer()
{
	return m_buffer;
}

VkDeviceSize bbe::INTERNAL::vulkan::VulkanBuffer::getSize()
{
	return m_bufferSize;
}

bool bbe::INTERNAL::vulkan::VulkanBuffer::isUsable()
{
	return (m_wasCreated || m_wasUploaded) && (!m_isMapped);
}

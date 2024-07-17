#include "BBE/Vulkan/VulkanBuffer.h"
#include "BBE/Error.h"
#include "BBE/Vulkan/VulkanDevice.h"
#include "BBE/Vulkan/VulkanHelper.h"
#include "BBE/Vulkan/VulkanCommandPool.h"
#include "BBE/Vulkan/VulkanManager.h"

void bbe::INTERNAL::vulkan::VulkanBuffer::preCreate(const VulkanDevice & vulkanDevice, size_t sizeInBytes, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t * p_queueFamilyIndices)
{
	m_bufferSize = sizeInBytes;
	m_device = vulkanDevice.getDevice();
	m_physicalDevice = vulkanDevice.getPhysicalDevice();
	m_usage = usage;
	preCreateBuffer(vulkanDevice.getDevice(), sizeInBytes, usage, m_buffer, sharingMode, queueFamilyIndexCount, p_queueFamilyIndices);
	m_wasPreCreated = true;
}

void bbe::INTERNAL::vulkan::VulkanBuffer::create(const VulkanDevice &vulkanDevice, size_t sizeInBytes, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* p_queueFamilyIndices, VkMemoryPropertyFlags memoryPropertyFlags)
{
	create(vulkanDevice.getDevice(), vulkanDevice.getPhysicalDevice(), sizeInBytes, usage, sharingMode, queueFamilyIndexCount, p_queueFamilyIndices, memoryPropertyFlags);
}

void bbe::INTERNAL::vulkan::VulkanBuffer::create(VkDevice vulkanDevice, VkPhysicalDevice physicalDevice, size_t sizeInBytes, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t * p_queueFamilyIndices, VkMemoryPropertyFlags memoryPropertyFlags)
{
	if (sizeInBytes == 0)
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}

	if (m_wasCreated)
	{
		bbe::Crash(bbe::Error::AlreadyCreated);
	}

	if (m_wasUploaded)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}

	m_bufferSize = sizeInBytes;
	m_device = vulkanDevice;
	m_physicalDevice = physicalDevice;
	m_usage = usage;

	if (m_wasPreCreated)
	{
		postCreateBuffer(m_device, m_physicalDevice, m_buffer, memoryPropertyFlags, m_memory, VK_NULL_HANDLE, 0);
	}
	else
	{
		createBuffer(m_device, m_physicalDevice, m_bufferSize, m_usage, m_buffer, memoryPropertyFlags, m_memory, sharingMode, queueFamilyIndexCount, p_queueFamilyIndices);
	}

	m_wasCreated = true;
}

void bbe::INTERNAL::vulkan::VulkanBuffer::upload(const VulkanCommandPool &commandPool, VkQueue queue, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* p_queueFamilyIndices)
{
	if ((m_usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0)
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}
	if (m_wasUploaded)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}
	if (m_isMapped)
	{
		bbe::Crash(bbe::Error::IllegalState);
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

void bbe::INTERNAL::vulkan::VulkanBuffer::upload(const VulkanCommandPool & commandPool, VkQueue queue, const VulkanBuffer & parentBuffer, VkDeviceSize offset, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t * p_queueFamilyIndices)
{
	if ((m_usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0)
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}
	if (m_wasUploaded)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}
	if (m_isMapped)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}

	VkBuffer uploadedBuffer = VK_NULL_HANDLE;
	VkDeviceMemory uploadedMemory = VK_NULL_HANDLE;

	createBuffer(m_device, m_physicalDevice, m_bufferSize, m_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, uploadedBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uploadedMemory, sharingMode, queueFamilyIndexCount, p_queueFamilyIndices, parentBuffer.m_memory, offset);

	copyBuffer(m_device, commandPool.getCommandPool(), queue, m_buffer, uploadedBuffer, m_bufferSize);

	vkDestroyBuffer(m_device, m_buffer, nullptr);
	vkFreeMemory(m_device, m_memory, nullptr);

	m_buffer = uploadedBuffer;
	m_memory = VK_NULL_HANDLE; //The Buffer does not own the memory!

	m_wasUploaded = true;
}

void bbe::INTERNAL::vulkan::VulkanBuffer::destroy()
{
	if (m_wasCreated || m_wasPreCreated)
	{
		if (m_isMapped)
		{
			bbe::Crash(bbe::Error::IllegalState);
		}
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
		if (m_memory != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_device, m_memory, nullptr);
			m_memory = VK_NULL_HANDLE;
		}

		m_wasCreated = false;
		m_wasUploaded = false;
	}
}

void * bbe::INTERNAL::vulkan::VulkanBuffer::map()
{
	if (m_wasUploaded)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}
	if (m_isMapped)
	{
		bbe::Crash(bbe::Error::IllegalState);
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
		bbe::Crash(bbe::Error::IllegalState);
	}

	vkUnmapMemory(m_device, m_memory);
	m_isMapped = false;
}

void bbe::INTERNAL::vulkan::VulkanBuffer::copy(const VulkanBuffer &other, VkCommandPool commandPool, VkQueue queue)
{
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}
	if (m_bufferSize < other.m_bufferSize)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}


	copyBuffer(m_device, commandPool, queue, other.m_buffer, m_buffer, m_bufferSize);
}

VkBuffer bbe::INTERNAL::vulkan::VulkanBuffer::getBuffer() const
{
	return m_buffer;
}

VkDeviceMemory bbe::INTERNAL::vulkan::VulkanBuffer::getMemory() const
{
	return m_memory;
}

VkDeviceSize bbe::INTERNAL::vulkan::VulkanBuffer::getSize() const
{
	return m_bufferSize;
}

bool bbe::INTERNAL::vulkan::VulkanBuffer::isUsable()
{
	return (m_wasCreated || m_wasUploaded) && (!m_isMapped);
}

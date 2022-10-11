#include "BBE/Vulkan/VulkanPhysicalDevices.h"
#include "BBE/Vulkan/VulkanInstance.h"
#include "BBE/Vulkan/VulkanSurface.h"
#include "BBE/Vulkan/VulkanHelper.h"
#include "BBE/List.h"

bbe::INTERNAL::vulkan::VulkanPhysicalDevice::VulkanPhysicalDevice(const VkPhysicalDevice & device, const VulkanSurface & surface)
	: m_device(device)
{
	vkGetPhysicalDeviceProperties(device, &m_properties);
	vkGetPhysicalDeviceFeatures(device, &m_features);
	vkGetPhysicalDeviceMemoryProperties(device, &m_memoryProperties);
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.getSurface(), &m_surfaceCapabilities);

	m_queueFamilyProperties = get_from_function<VkQueueFamilyProperties>(vkGetPhysicalDeviceQueueFamilyProperties, device);
	m_surfaceFormats = get_from_function<VkSurfaceFormatKHR>(vkGetPhysicalDeviceSurfaceFormatsKHR, device, surface.getSurface());
	m_presentModes = get_from_function<VkPresentModeKHR>(vkGetPhysicalDeviceSurfacePresentModesKHR, device, surface.getSurface());
	m_extensionProperties = get_from_function<VkExtensionProperties>(vkEnumerateDeviceExtensionProperties, device, nullptr);
}

uint32_t bbe::INTERNAL::vulkan::VulkanPhysicalDevice::findBestCompleteQueueIndex() const
{
	return 0; //TODO find best queue index which is complete
	          //TODO check if VK_QUEUE_GRAPHICS_BIT is supported via vkGetPhysicalDeviceQueueFamilyProperties
}

uint32_t bbe::INTERNAL::vulkan::VulkanPhysicalDevice::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags memoryPropertyFlags) const
{
	for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++)
	{
		if (typeBits & 1)
		{
			if ((m_memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
			{
				return i;
			}
		}
		typeBits = typeBits >> 1;
	}

	ASSERT_VULKAN(false);
	return 0;
}

VkPhysicalDevice bbe::INTERNAL::vulkan::VulkanPhysicalDevice::getDevice() const
{
	return m_device;
}

bbe::INTERNAL::vulkan::PhysicalDeviceContainer::PhysicalDeviceContainer()
{
	//do nothing
}

void bbe::INTERNAL::vulkan::PhysicalDeviceContainer::init(const VulkanInstance & instance, const VulkanSurface & surface)
{
	bbe::List<VkPhysicalDevice> physicalDevices = get_from_function<VkPhysicalDevice>(vkEnumeratePhysicalDevices, instance.getInstance());
	
	for (size_t i = 0; i < physicalDevices.getLength(); i++)
	{
		m_devices.add(VulkanPhysicalDevice(physicalDevices[i], surface));
	}
}

const bbe::INTERNAL::vulkan::VulkanPhysicalDevice & bbe::INTERNAL::vulkan::PhysicalDeviceContainer::findBestDevice(const VulkanSurface & surface) const
{
	for (size_t i = 0; i < m_devices.getLength(); i++)
	{
		VkBool32 supported = false;
		ASSERT_VULKAN(vkGetPhysicalDeviceSurfaceSupportKHR(m_devices[i].getDevice(), m_devices[i].findBestCompleteQueueIndex(), surface.getSurface(), &supported));
		if (supported)
		{
			return m_devices[i];
		}
	}
	return m_devices[0];	//TODO find best device!
							//Checken ob SurfaceSupport vorhanden ist via vkGetPhysicalDeviceSurfaceSupportKHR
}

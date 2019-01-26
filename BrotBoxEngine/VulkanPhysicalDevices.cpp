#include "BBE/VulkanPhysicalDevices.h"
#include "BBE/VulkanInstance.h"
#include "BBE/VulkanSurface.h"
#include "BBE/List.h"
#include "BBE/VulkanHelper.h"

bbe::INTERNAL::vulkan::VulkanPhysicalDevice::VulkanPhysicalDevice(const VkPhysicalDevice & device, const VulkanSurface & surface)
	: m_device(device)
{
	vkGetPhysicalDeviceProperties(device, &m_properties);
	vkGetPhysicalDeviceFeatures(device, &m_features);
	vkGetPhysicalDeviceMemoryProperties(device, &m_memoryProperties);
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.getSurface(), &m_surfaceCapabilities);

	uint32_t amountOfQueueFamilyProperties = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilyProperties, nullptr);
	m_queueFamilyProperties.resizeCapacityAndLength(amountOfQueueFamilyProperties);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilyProperties, m_queueFamilyProperties.getRaw());

	uint32_t amountOfSurfaceFormats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.getSurface(), &amountOfSurfaceFormats, nullptr);
	m_surfaceFormats.resizeCapacityAndLength(amountOfSurfaceFormats);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.getSurface(), &amountOfSurfaceFormats, m_surfaceFormats.getRaw());

	uint32_t amountOfPresentModes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.getSurface(), &amountOfPresentModes, nullptr);
	m_presentModes.resizeCapacityAndLength(amountOfPresentModes);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.getSurface(), &amountOfPresentModes, m_presentModes.getRaw());

	uint32_t amountOfExtensionProperties = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &amountOfExtensionProperties, nullptr);
	m_extensionProperties.resizeCapacityAndLength(amountOfExtensionProperties);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &amountOfExtensionProperties, m_extensionProperties.getRaw());
}

uint32_t bbe::INTERNAL::vulkan::VulkanPhysicalDevice::findBestCompleteQueueIndex() const
{
	return 0; //TODO find best queue index which is complete
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
	VkPhysicalDevice *physicalDevices = nullptr;
	uint32_t length = 0;
	ASSERT_VULKAN(vkEnumeratePhysicalDevices(instance.getInstance(), &length, nullptr));
	physicalDevices = new VkPhysicalDevice[length];
	ASSERT_VULKAN(vkEnumeratePhysicalDevices(instance.getInstance(), &length, physicalDevices));

	for (size_t i = 0; i < length; i++)
	{
		m_devices.add(VulkanPhysicalDevice(physicalDevices[i], surface));
	}
	delete[] physicalDevices;
}

const bbe::INTERNAL::vulkan::VulkanPhysicalDevice & bbe::INTERNAL::vulkan::PhysicalDeviceContainer::findBestDevice(const VulkanSurface & surface) const
{
	for (size_t i = 0; i < m_devices.getLength(); i++)
	{
		VkBool32 supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_devices[i].getDevice(), m_devices[i].findBestCompleteQueueIndex(), surface.getSurface(), &supported);
		if (supported)
		{
			return m_devices[i];
		}
	}
	return m_devices[0];	//TODO find best device!
							//Checken ob SurfaceSupport vorhanden ist via vkGetPhysicalDeviceSurfaceSupportKHR
}

#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "List.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanPhysicalDevice
			{
			private:
				VkPhysicalDevice                  m_device                = VK_NULL_HANDLE;
				VkPhysicalDeviceProperties        m_properties            = {};
				VkPhysicalDeviceFeatures          m_features              = {};
				VkPhysicalDeviceMemoryProperties  m_memoryProperties      = {};
				VkSurfaceCapabilitiesKHR          m_surfaceCapabilities   = {};
				List<VkQueueFamilyProperties>     m_queueFamilyProperties;
				List<VkSurfaceFormatKHR>          m_surfaceFormats;
				List<VkPresentModeKHR>            m_presentModes;
				List<VkExtensionProperties>       m_extensionProperties;


				//TODO check if present is supported by various families
				//VkBool32                          m_presentSupported      = false;

			public:

				VulkanPhysicalDevice(const VkPhysicalDevice &device, const VulkanSurface &surface)
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

				//PhysicalDevice(const PhysicalDevice&) = delete;
				//PhysicalDevice(PhysicalDevice&&) = delete;
				//PhysicalDevice& operator=(const PhysicalDevice&) = delete;
				//PhysicalDevice& operator=(PhysicalDevice&&) = delete;

				uint32_t findBestCompleteQueueIndex() const
				{
					return 0; //TODO find best queue index which is complete
				}

				VkPhysicalDevice getDevice() const
				{
					return m_device;
				}
			};

			class PhysicalDeviceContainer
			{
				//TODO use Allocators
			private:
				List<VulkanPhysicalDevice> m_devices;
			public:
				PhysicalDeviceContainer()
				{
					//do nothing
				}

				PhysicalDeviceContainer(const PhysicalDeviceContainer& other) = delete;
				PhysicalDeviceContainer(PhysicalDeviceContainer&& other) = delete;
				PhysicalDeviceContainer& operator=(const PhysicalDeviceContainer& other) = delete;
				PhysicalDeviceContainer& operator=(PhysicalDeviceContainer&& other) = delete;

				void init(const VulkanInstance &instance, const VulkanSurface &surface)
				{
					VkPhysicalDevice *physicalDevices = nullptr;
					uint32_t length = 0;
					vkEnumeratePhysicalDevices(instance.getInstance(), &length, nullptr);
					physicalDevices = new VkPhysicalDevice[length];
					vkEnumeratePhysicalDevices(instance.getInstance(), &length, physicalDevices);

					for (size_t i = 0; i < length; i++)
					{
						m_devices.add(VulkanPhysicalDevice(physicalDevices[i], surface));
					}
					delete physicalDevices;
				}

				~PhysicalDeviceContainer()
				{
				}

				const VulkanPhysicalDevice& findBestDevice(const VulkanSurface &surface) const
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
			};
		}
	}
}
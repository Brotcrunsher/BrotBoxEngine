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
			class PhysicalDevice
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

				PhysicalDevice(const VkPhysicalDevice &device, const Surface &surface)
					: m_device(device)
				{
					vkGetPhysicalDeviceProperties(device, &m_properties);
					vkGetPhysicalDeviceFeatures(device, &m_features);
					vkGetPhysicalDeviceMemoryProperties(device, &m_memoryProperties);
					vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.getSurface(), &m_surfaceCapabilities);

					uint32_t amountOfQueueFamilyProperties = 0;
					vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilyProperties, nullptr);
					m_queueFamilyProperties.resizeCapacity(amountOfQueueFamilyProperties);
					vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilyProperties, m_queueFamilyProperties.getRaw());

					uint32_t amountOfSurfaceFormats = 0;
					vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.getSurface(), &amountOfSurfaceFormats, nullptr);
					m_surfaceFormats.resizeCapacity(amountOfSurfaceFormats);
					vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.getSurface(), &amountOfSurfaceFormats, m_surfaceFormats.getRaw());

					uint32_t amountOfPresentModes = 0;
					vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.getSurface(), &amountOfPresentModes, nullptr);
					m_presentModes.resizeCapacity(amountOfPresentModes);
					vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.getSurface(), &amountOfPresentModes, m_presentModes.getRaw());
				
					uint32_t amountOfExtensionProperties = 0;
					vkEnumerateDeviceExtensionProperties(device, nullptr, &amountOfExtensionProperties, nullptr);
					m_extensionProperties.resizeCapacity(amountOfExtensionProperties);
					vkEnumerateDeviceExtensionProperties(device, nullptr, &amountOfExtensionProperties, m_extensionProperties.getRaw());
				}


			};

			class PhysicalDeviceContainer
			{
				//TODO use Allocators
			private:
				List<PhysicalDevice> m_devices;
			public:
				PhysicalDeviceContainer()
				{
					//do nothing
				}

				PhysicalDeviceContainer(const PhysicalDeviceContainer& other) = delete;
				PhysicalDeviceContainer(PhysicalDeviceContainer&& other) = delete;
				PhysicalDeviceContainer& operator=(const PhysicalDeviceContainer& other) = delete;
				PhysicalDeviceContainer& operator=(PhysicalDeviceContainer&& other) = delete;

				void init(const Instance &instance, const Surface &surface)
				{
					VkPhysicalDevice *physicalDevices = nullptr;
					uint32_t length = 0;
					vkEnumeratePhysicalDevices(instance.getInstance(), &length, nullptr);
					physicalDevices = new VkPhysicalDevice[length];
					vkEnumeratePhysicalDevices(instance.getInstance(), &length, physicalDevices);

					for (size_t i = 0; i < length; i++)
					{
						m_devices.pushBack(PhysicalDevice(physicalDevices[i], surface));
					}
					delete physicalDevices;
				}

				~PhysicalDeviceContainer()
				{
				}
			};
		}
	}
}
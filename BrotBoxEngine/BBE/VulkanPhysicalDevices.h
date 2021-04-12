#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "../BBE/List.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanInstance;
			class VulkanSurface;

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

				VulkanPhysicalDevice(const VkPhysicalDevice &device, const VulkanSurface &surface);


				uint32_t findBestCompleteQueueIndex() const;
				uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags memoryPropertyFlags) const;

				VkPhysicalDevice getDevice() const;
			};

			class PhysicalDeviceContainer
			{
				//TODO use Allocators
			private:
				List<VulkanPhysicalDevice> m_devices;
			public:
				PhysicalDeviceContainer();

				PhysicalDeviceContainer(const PhysicalDeviceContainer& other) = delete;
				PhysicalDeviceContainer(PhysicalDeviceContainer&& other) = delete;
				PhysicalDeviceContainer& operator=(const PhysicalDeviceContainer& other) = delete;
				PhysicalDeviceContainer& operator=(PhysicalDeviceContainer&& other) = delete;

				void init(const VulkanInstance &instance, const VulkanSurface &surface);

				const VulkanPhysicalDevice& findBestDevice(const VulkanSurface &surface) const;
			};
		}
	}
}

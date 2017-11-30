#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/List.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class PhysicalDeviceContainer;
			class VulkanSurface;
			class VulkanManager;

			class VulkanSharingBehaviour
			{
			public:
				VkSharingMode  m_sharingMode;
				List<uint32_t> m_queueFamilyIndices;
			};

			class VulkanDevice
			{
				friend class VulkanManager;
			private:
				VkDevice                   m_device         = VK_NULL_HANDLE;
				VkQueue                    m_queue          = VK_NULL_HANDLE;
				VkPhysicalDevice           m_physicalDevice = VK_NULL_HANDLE;
				List<VkSurfaceFormatKHR>   m_formats;
				uint32_t                   queueFamilyIndex = 0;
				VkPhysicalDeviceProperties m_properties     = {};

			public:
				VulkanDevice() {};

				void init(const PhysicalDeviceContainer &physicalDevices, const VulkanSurface &surface);

				VulkanDevice(const VulkanDevice&)            = delete;
				VulkanDevice(VulkanDevice&&)                 = delete;
				VulkanDevice& operator=(const VulkanDevice&) = delete;
				VulkanDevice& operator=(VulkanDevice&&)      = delete;


				void destroy();

				void waitIdle() const;

				VkDevice getDevice() const;

				VkPhysicalDevice getPhysicalDevice() const;

				VkFormat getFormat() const;

				VkColorSpaceKHR getColorSpace() const;

				VulkanSharingBehaviour getSharingBehaviour() const;

				VkPresentModeKHR getPresentMode() const;

				VkQueue getQueue() const;

				uint32_t getQueueFamilyIndex() const;
			};
		}
	}
}

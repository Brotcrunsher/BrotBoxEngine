#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "VulkanPhysicalDevices.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanSharingBehaviour
			{
			public:
				VkSharingMode m_sharingMode;
				List<uint32_t> m_queueFamilyIndices;
			};

			class VulkanDevice
			{
			private:
				VkDevice m_device = VK_NULL_HANDLE;
				VkQueue m_queue = VK_NULL_HANDLE;
				VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
				List<VkSurfaceFormatKHR> m_formats;

			public:
				VulkanDevice()
				{
					//DO NOTHING
				}

				void init(const PhysicalDeviceContainer &physicalDevices, const VulkanSurface &surface) {
					VulkanPhysicalDevice pd = physicalDevices.findBestDevice(surface);
					m_physicalDevice = pd.getDevice();
					uint32_t queueFamilyIndex = pd.findBestCompleteQueueIndex();

					VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
					deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					deviceQueueCreateInfo.pNext = nullptr;
					deviceQueueCreateInfo.flags = 0;
					deviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
					deviceQueueCreateInfo.queueCount = 1;	//TODO Check if this amount is valid
					List<float> queuePriorities;
					for (uint32_t i = 0; i < deviceQueueCreateInfo.queueCount; i++)
					{
						queuePriorities.add(1.0f);
					}
					deviceQueueCreateInfo.pQueuePriorities = queuePriorities.getRaw();

					VkPhysicalDeviceFeatures usedFeatures = {};

					const List<const char*> deviceExtensions = {
						VK_KHR_SWAPCHAIN_EXTENSION_NAME
					};

					VkDeviceCreateInfo deviceCreateInfo;
					deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
					deviceCreateInfo.pNext = nullptr;
					deviceCreateInfo.flags = 0;
					deviceCreateInfo.queueCreateInfoCount = 1;
					deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
					deviceCreateInfo.enabledLayerCount = 0;
					deviceCreateInfo.ppEnabledLayerNames = nullptr;
					deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.getLength();
					deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.getRaw();
					deviceCreateInfo.pEnabledFeatures = &usedFeatures;

					VkResult result = vkCreateDevice(pd.getDevice(), &deviceCreateInfo, nullptr, &m_device);
					ASSERT_VULKAN(result);

					vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &m_queue);


					uint32_t amountOfFormats = 0;
					vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface.getSurface(), &amountOfFormats, nullptr);
					m_formats.resizeCapacityAndLength(amountOfFormats);
					vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface.getSurface(), &amountOfFormats, m_formats.getRaw());
				}

				VulkanDevice(const VulkanDevice&)            = delete;
				VulkanDevice(VulkanDevice&&)                 = delete;
				VulkanDevice& operator=(const VulkanDevice&) = delete;
				VulkanDevice& operator=(VulkanDevice&&)      = delete;

				~VulkanDevice()
				{
					vkDestroyDevice(m_device, nullptr);
				}

				VkDevice getDevice() const
				{
					return m_device;
				}

				VkPhysicalDevice getPhysicalDevice() const
				{
					return m_physicalDevice;
				}

				VkFormat getFormat() const
				{
					//TODO find best supported format
					//via vkGetPhysicalDeviceSurfaceFormatsKHR, somehow prioritize every possible format and take the best available format
					//DO THIS TOGETHER WITH getColorSpace()!!!!!!!!!!!!!!
					return VK_FORMAT_B8G8R8A8_UNORM;
				}

				VkColorSpaceKHR getColorSpace() const
				{
					//TODO find best supported format
					//via vkGetPhysicalDeviceSurfaceFormatsKHR, somehow prioritize every possible format and take the best available format
					//DO THIS TOGETHER WITH getFormat()!!!!!!!!!!!!!!
					return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
				}

				VulkanSharingBehaviour getSharingBehaviour() const
				{
					VulkanSharingBehaviour vsb;
					//TODO check if presentation and rendering queue are the same. If yes, return VK_SHARING_MODE_EXCLUSIVE
					vsb.m_sharingMode = VK_SHARING_MODE_EXCLUSIVE;
					//TODO if not, set to VK_SHARING_MODE_CONCURRENT and add all queues to vsb.m_queueFamilyIndices.
					return vsb;
				}

				VkPresentModeKHR getPresentMode() const
				{
					//TODO check if VK_PRESENT_MODE_MAILBOX_KHR is supported.
					return VK_PRESENT_MODE_FIFO_KHR;
				}
			};
		}
	}
}
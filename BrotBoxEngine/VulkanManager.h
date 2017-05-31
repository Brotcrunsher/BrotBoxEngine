#pragma once

#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanPhysicalDevices.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager {
			private:
				Instance m_instance;
				VulkanSurface m_surface;
				PhysicalDeviceContainer m_physicalDeviceContainer;
				VulkanDevice m_device;
				VulkanSwapchain m_swapchain;
				GLFWwindow *m_window = nullptr;

			public:
				VulkanManager()
				{
					//DO NOTHING
				}

				VulkanManager(const VulkanManager& other) = delete;
				VulkanManager(VulkanManager&& other) = delete;
				VulkanManager& operator=(const VulkanManager& other) = delete;
				VulkanManager& operator=(VulkanManager&& other) = delete;

				void init(const char *appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow *window, uint32_t initialWindowWidth, uint32_t initialWindowHeight)
				{
					m_window = window;
					m_instance.init(appName, major, minor, patch);
					m_surface.init(m_instance, m_window);
					m_physicalDeviceContainer.init(m_instance, m_surface);
					m_device.init(m_physicalDeviceContainer, m_surface);
					m_swapchain.init(m_surface, m_device, initialWindowWidth, initialWindowHeight, nullptr);
				}
			};
		}
	}
}
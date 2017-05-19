#pragma once

#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanPhysicalDevices.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager {
			private:
				Instance m_instance;
				Surface m_surface;
				PhysicalDeviceContainer m_physicalDeviceContainer;
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

				void init(const char *appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow *window)
				{
					m_window = window;
					m_instance.init(appName, major, minor, patch);
					m_surface.init(m_instance, m_window);
					m_physicalDeviceContainer.init(m_instance, m_surface);
				}
			};
		}
	}
}
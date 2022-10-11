#pragma once

#include "GLFW/glfw3.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanInstance
			{
			private:
				VkInstance m_instance = VK_NULL_HANDLE;
#ifdef _DEBUG
				VkDebugUtilsMessengerEXT m_debugUtilsMessenger = VK_NULL_HANDLE;
				bool validationLayerPresent = false;
				bool debugUtilsExtensionPresent = false;
#endif

			public:
				VulkanInstance();

				void destroy();

				void init(const char *appName, uint32_t major, uint32_t minor, uint32_t patch);

				VulkanInstance(const VulkanInstance& other)            = delete;
				VulkanInstance(VulkanInstance&& other)                 = delete;
				VulkanInstance& operator=(const VulkanInstance& other) = delete;
				VulkanInstance& operator=(VulkanInstance&& other)      = delete;



				VkInstance getInstance() const;
			};
		}
	}
}

#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "List.h"
#include "VulkanHelper.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class Instance
			{
			private:
				VkInstance m_instance = VK_NULL_HANDLE;

			public:
				Instance()
				{
					//DO NOTHING
				}

				void init(const char *appName, uint32_t major, uint32_t minor, uint32_t patch)
				{
					VkApplicationInfo appInfo = {};
					appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
					appInfo.pApplicationName = appName;
					appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
					appInfo.pEngineName = "Brot Box Engine";
					appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
					appInfo.apiVersion = VK_API_VERSION_1_0;

					const bbe::List<const char*> validationLayers = {
						"VK_LAYER_LUNARG_standard_validation"
					};

					uint32_t amountOfGlfwExtensions = 0;
					auto glfwExtensions = glfwGetRequiredInstanceExtensions(&amountOfGlfwExtensions);

					VkInstanceCreateInfo instanceInfo = {};
					instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
					instanceInfo.pApplicationInfo = &appInfo;
					instanceInfo.enabledLayerCount = validationLayers.getLength();
					instanceInfo.ppEnabledLayerNames = validationLayers.getRaw();;
					instanceInfo.enabledExtensionCount = amountOfGlfwExtensions;
					instanceInfo.ppEnabledExtensionNames = glfwExtensions;

					VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_instance);
					ASSERT_VULKAN(result);
				}

				Instance(const Instance& other) = delete;
				Instance(Instance&& other) = delete;
				Instance& operator=(const Instance& other) = delete;
				Instance& operator=(Instance&& other) = delete;

				~Instance() {
					vkDestroyInstance(m_instance, nullptr);
				}


				VkInstance getInstance() const
				{
					return m_instance;
				}
			};
		}
	}
}
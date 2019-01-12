#include "BBE/VulkanInstance.h"
#include "BBE/List.h"
#include "BBE/VulkanHelper.h"

bbe::INTERNAL::vulkan::VulkanInstance::VulkanInstance()
{
	//DO NOTHING
}

void bbe::INTERNAL::vulkan::VulkanInstance::destroy()
{
	if (m_instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(m_instance, nullptr);
	}
}

void bbe::INTERNAL::vulkan::VulkanInstance::init(const char * appName, uint32_t major, uint32_t minor, uint32_t patch)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
	appInfo.pEngineName = "Brot Box Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	const bbe::List<const char*> validationLayers = {
#ifdef _DEBUG
		"VK_LAYER_LUNARG_standard_validation"
#endif
	};

	uint32_t amountOfGlfwExtensions = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&amountOfGlfwExtensions);

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = (uint32_t)validationLayers.getLength();
	instanceInfo.ppEnabledLayerNames = validationLayers.getRaw();;
	instanceInfo.enabledExtensionCount = amountOfGlfwExtensions;
	instanceInfo.ppEnabledExtensionNames = glfwExtensions;

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_instance);
	ASSERT_VULKAN(result);
}

VkInstance bbe::INTERNAL::vulkan::VulkanInstance::getInstance() const
{
	return m_instance;
}

#include "BBE/Vulkan/VulkanInstance.h"
#include "BBE/Vulkan/VulkanHelper.h"
#include "BBE/List.h"
#include "BBE/FatalErrors.h"

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
		"VK_LAYER_KHRONOS_validation"
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

	if (result == VkResult::VK_ERROR_OUT_OF_HOST_MEMORY)
	{
		bbe::INTERNAL::triggerFatalError("No more host memory!");
	}
	else if (result == VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY)
	{
		bbe::INTERNAL::triggerFatalError("No more device memory!");
	}
	else if (result == VkResult::VK_ERROR_INITIALIZATION_FAILED)
	{
		bbe::INTERNAL::triggerFatalError("Failed to initialize vulkan!");
	}
	else if (result == VkResult::VK_ERROR_LAYER_NOT_PRESENT)
	{
		bbe::INTERNAL::triggerFatalError("A layer was not present!");
	}
	else if (result == VkResult::VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		bbe::INTERNAL::triggerFatalError("An extension was not present!");
	}
	else if (result == VkResult::VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		bbe::INTERNAL::triggerFatalError("Your driver does not support vulkan.");
	}
	ASSERT_VULKAN(result);
}

VkInstance bbe::INTERNAL::vulkan::VulkanInstance::getInstance() const
{
	return m_instance;
}

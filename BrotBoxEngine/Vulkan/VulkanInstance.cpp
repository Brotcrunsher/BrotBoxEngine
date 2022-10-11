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
#ifdef _DEBUG
	if (validationLayerPresent && debugUtilsExtensionPresent) 
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(m_instance, m_debugUtilsMessenger, nullptr);
		}
		else
		{
			debugBreak();
		}
	}
#endif

	if (m_instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(m_instance, nullptr);
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL validationCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData)
{
	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		std::cout << callbackData->pMessage << std::endl << std::endl;
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			bbe::debugBreak();
		}
	}
	return VK_FALSE;
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

	bbe::List<const char*> validationLayers;
#ifdef _DEBUG
	bbe::List<VkLayerProperties> layerProperties = get_from_function<VkLayerProperties>(vkEnumerateInstanceLayerProperties);
#define BBE_VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"
	validationLayerPresent = layerProperties.contains([](const VkLayerProperties& p)
		{
			return p.layerName == bbe::String(BBE_VALIDATION_LAYER_NAME);
		});

	if (validationLayerPresent)
	{
		validationLayers.add(BBE_VALIDATION_LAYER_NAME);
	}
	else
	{
		std::cout << "WARNING: Could not find Validation Layer for Vulkan!" << std::endl;
	}
#undef BBE_VALIDATION_LAYER_NAME
#endif
#ifdef _DEBUG
	bbe::List<VkExtensionProperties> extensionsProperties = get_from_function<VkExtensionProperties>(vkEnumerateInstanceExtensionProperties, nullptr);
	debugUtilsExtensionPresent = extensionsProperties.contains([](const VkExtensionProperties& p)
		{ 
			return p.extensionName == bbe::String(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		});
#endif

	uint32_t amountOfGlfwExtensions = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&amountOfGlfwExtensions);

	bbe::List<const char*> extensions;
	for (uint32_t i = 0; i < amountOfGlfwExtensions; i++)
	{
		extensions.add(glfwExtensions[i]);
	}
#ifdef _DEBUG
	if (debugUtilsExtensionPresent)
	{
		extensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	else
	{
		std::cout << "WARNING: Could not find Debug Utils Extension for Vulkan!" << std::endl;
	}
#endif
#ifdef _DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerInfo{};
	debugUtilsMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugUtilsMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugUtilsMessengerInfo.pfnUserCallback = validationCallback;
#endif

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = (uint32_t)validationLayers.getLength();
	instanceInfo.ppEnabledLayerNames = validationLayers.getRaw();;
	instanceInfo.enabledExtensionCount = extensions.getLength();
	instanceInfo.ppEnabledExtensionNames = extensions.getRaw();
#ifdef _DEBUG
	instanceInfo.pNext = &debugUtilsMessengerInfo;
#endif

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

#ifdef _DEBUG
	if (validationLayerPresent && debugUtilsExtensionPresent)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			result = func(m_instance, &debugUtilsMessengerInfo, nullptr, &m_debugUtilsMessenger);
			ASSERT_VULKAN(result);
		}
		else
		{
			debugBreak();
		}
	}
#endif
}

VkInstance bbe::INTERNAL::vulkan::VulkanInstance::getInstance() const
{
	return m_instance;
}

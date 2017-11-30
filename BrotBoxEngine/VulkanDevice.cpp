#include "stdafx.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanSurface.h"
#include "BBE/VulkanPhysicalDevices.h"
#include "BBE/VulkanHelper.h"
#include "BBE/Exceptions.h"

void bbe::INTERNAL::vulkan::VulkanDevice::init(const PhysicalDeviceContainer & physicalDevices, const VulkanSurface & surface) {
	VulkanPhysicalDevice pd = physicalDevices.findBestDevice(surface);
	m_physicalDevice = pd.getDevice();
	queueFamilyIndex = pd.findBestCompleteQueueIndex();

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
	usedFeatures.samplerAnisotropy = VK_TRUE;
	usedFeatures.tessellationShader = VK_TRUE;
	usedFeatures.fillModeNonSolid = VK_TRUE;

	const List<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		"VK_KHR_sampler_mirror_clamp_to_edge"
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

	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
}

void bbe::INTERNAL::vulkan::VulkanDevice::destroy()
{
	if (m_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(m_device, nullptr);
		m_device = VK_NULL_HANDLE;
	}
}

void bbe::INTERNAL::vulkan::VulkanDevice::waitIdle() const
{
	if (m_device == VK_NULL_HANDLE)
	{
		throw NotInitializedException();
	}

	vkDeviceWaitIdle(m_device);
}

VkDevice bbe::INTERNAL::vulkan::VulkanDevice::getDevice() const
{
	return m_device;
}

VkPhysicalDevice bbe::INTERNAL::vulkan::VulkanDevice::getPhysicalDevice() const
{
	return m_physicalDevice;
}

VkFormat bbe::INTERNAL::vulkan::VulkanDevice::getFormat() const
{
	//TODO find best supported format
	//via vkGetPhysicalDeviceSurfaceFormatsKHR, somehow prioritize every possible format and take the best available format
	//DO THIS TOGETHER WITH getColorSpace()!!!!!!!!!!!!!!
	return VK_FORMAT_B8G8R8A8_UNORM;
}

VkColorSpaceKHR bbe::INTERNAL::vulkan::VulkanDevice::getColorSpace() const
{
	//TODO find best supported format
	//via vkGetPhysicalDeviceSurfaceFormatsKHR, somehow prioritize every possible format and take the best available format
	//DO THIS TOGETHER WITH getFormat()!!!!!!!!!!!!!!
	return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

bbe::INTERNAL::vulkan::VulkanSharingBehaviour bbe::INTERNAL::vulkan::VulkanDevice::getSharingBehaviour() const
{
	VulkanSharingBehaviour vsb;
	//TODO check if presentation and rendering queue are the same. If yes, return VK_SHARING_MODE_EXCLUSIVE
	vsb.m_sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//TODO if not, set to VK_SHARING_MODE_CONCURRENT and add all queues to vsb.m_queueFamilyIndices.
	return vsb;
}

VkPresentModeKHR bbe::INTERNAL::vulkan::VulkanDevice::getPresentMode() const
{
	//TODO check if VK_PRESENT_MODE_MAILBOX_KHR is supported.
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkQueue bbe::INTERNAL::vulkan::VulkanDevice::getQueue() const
{
	return m_queue;
}

uint32_t bbe::INTERNAL::vulkan::VulkanDevice::getQueueFamilyIndex() const
{
	return queueFamilyIndex;
}
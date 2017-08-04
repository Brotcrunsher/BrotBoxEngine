#include "stdafx.h"
#include "VulkanShader.h"
#include "SimpleFile.h"
#include "VulkanHelper.h"
#include "VulkanDevice.h"
#include "List.h"

bbe::INTERNAL::vulkan::VulkanShader::VulkanShader()
{
}

void bbe::INTERNAL::vulkan::VulkanShader::init(const VulkanDevice & device, const bbe::String & path)
{
	auto data = simpleFile::readBinaryFile(path);
	init(device, data);
}

void bbe::INTERNAL::vulkan::VulkanShader::init(const VulkanDevice & device, const bbe::List<char>& code)
{
	m_device = device.getDevice();

	VkShaderModuleCreateInfo shaderCreateInfo;
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.pNext = nullptr;
	shaderCreateInfo.flags = 0;
	shaderCreateInfo.codeSize = code.getLength();
	shaderCreateInfo.pCode = (uint32_t*)code.getRaw();

	VkResult result = vkCreateShaderModule(m_device, &shaderCreateInfo, nullptr, &m_shaderModule);
	ASSERT_VULKAN(result);
}

void bbe::INTERNAL::vulkan::VulkanShader::destroy()
{
	if (m_shaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
		m_shaderModule = VK_NULL_HANDLE;
	}
}

VkShaderModule bbe::INTERNAL::vulkan::VulkanShader::getModule()
{
	return m_shaderModule;
}

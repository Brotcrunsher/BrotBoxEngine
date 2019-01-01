#include "stdafx.h"
#include "BBE/VulkanShader.h"
#include "BBE/SimpleFile.h"
#include "BBE/VulkanHelper.h"
#include "BBE/VulkanDevice.h"
#include "BBE/List.h"
#include <iostream>

bbe::INTERNAL::vulkan::VulkanShader::VulkanShader()
{
}

void bbe::INTERNAL::vulkan::VulkanShader::init(const VulkanDevice & device, const bbe::String & path)
{
	std::cout << "Loading Shader: " << path.getRaw() << std::endl;
	auto data = simpleFile::readBinaryFile(path);
	std::cout << "Init Shader: " << path.getRaw() << std::endl;
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
		m_device       = VK_NULL_HANDLE;
	}
}

VkShaderModule bbe::INTERNAL::vulkan::VulkanShader::getModule()
{
	return m_shaderModule;
}

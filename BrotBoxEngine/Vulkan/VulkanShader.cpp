#include "BBE/Vulkan/VulkanShader.h"
#include "BBE/Vulkan/VulkanHelper.h"
#include "BBE/Vulkan/VulkanDevice.h"
#include "BBE/Vulkan/VulkanManager.h"
#include "BBE/SimpleFile.h"
#include "BBE/List.h"
#include <iostream>

bbe::INTERNAL::vulkan::VulkanShader::VulkanShader()
{
}

void bbe::INTERNAL::vulkan::VulkanShader::init(const bbe::String &path)
{
	if (VulkanManager::s_pinstance == nullptr)
	{
		bbe::Crash(bbe::Error::NullPointer);
	}
	init(VulkanManager::s_pinstance->getVulkanDevice(), path);
}

void bbe::INTERNAL::vulkan::VulkanShader::init(const VulkanDevice &device, const bbe::String &path)
{
	auto data = simpleFile::readBinaryFile(path);
	init(device, data);
}

void bbe::INTERNAL::vulkan::VulkanShader::init(const VulkanDevice &device, const bbe::ByteBuffer &code)
{
	m_device = device.getDevice();

	// Basic sanity checks before passing arbitrary bytes to the Vulkan driver.
	// Vulkan requires codeSize to be a multiple of 4 and pCode to point to
	// valid SPIR-V words.
	const size_t codeSize = code.getLength();
	if (codeSize == 0 || (codeSize % 4) != 0)
	{
		bbe::Crash(bbe::Error::IllegalArgument, "Invalid shader bytecode size");
	}

	const uint32_t *words = reinterpret_cast<const uint32_t *>(code.getRaw());
	// SPIR-V magic number: 0x07230203 in little endian.
	if (words[0] != 0x07230203u)
	{
		bbe::Crash(bbe::Error::IllegalArgument, "Shader bytecode is not valid SPIR-V");
	}

	VkShaderModuleCreateInfo shaderCreateInfo;
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.pNext = nullptr;
	shaderCreateInfo.flags = 0;
	shaderCreateInfo.codeSize = codeSize;
	shaderCreateInfo.pCode = words;

	VkResult result = vkCreateShaderModule(m_device, &shaderCreateInfo, nullptr, &m_shaderModule);
	ASSERT_VULKAN(result);
}

void bbe::INTERNAL::vulkan::VulkanShader::destroy()
{
	if (m_shaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
		m_shaderModule = VK_NULL_HANDLE;
		m_device = VK_NULL_HANDLE;
	}
}

VkShaderModule bbe::INTERNAL::vulkan::VulkanShader::getModule()
{
	return m_shaderModule;
}

#include "BBE/FragmentShader.h"
#include "BBE/Window.h"
#include "BBE/Vulkan/VulkanManager.h"
#include "BBE/Vulkan/VulkanDevice.h"
#include "BBE/Vulkan/VulkanRenderPass.h"
#include "BBE/PrimitiveBrush2D.h"

bbe::FragmentShader::FragmentShader()
{
}

bbe::FragmentShader::FragmentShader(const char* path)
{
	load(path);
}

bbe::FragmentShader::~FragmentShader()
{
	m_pipeline.destroy();
	m_shader.destroy();
}

void bbe::FragmentShader::load(const char* path)
{
	if (bbe::Window::INTERNAL_firstInstance == nullptr)
	{
		throw NullPointerException();
	}
	if (bbe::INTERNAL::vulkan::VulkanManager::s_pinstance == nullptr)
	{
		throw NullPointerException();
	}
	if (isLoaded)
	{
		throw AlreadyCreatedException();
	}

	m_shader.init(path);

	m_pipeline.init(bbe::INTERNAL::vulkan::VulkanManager::s_pinstance->getVertexShader2DPrimitive(), m_shader, bbe::Window::INTERNAL_firstInstance->getWidth(), bbe::Window::INTERNAL_firstInstance->getHeight());
	m_pipeline.addVertexBinding(0, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline.addVertexDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
	m_pipeline.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0, 64);
	m_pipeline.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 64, 64);
	m_pipeline.create(bbe::INTERNAL::vulkan::VulkanManager::s_pinstance->getVulkanDevice().getDevice(), bbe::INTERNAL::vulkan::VulkanManager::s_pinstance->getVulkanRenderPass().getRenderPass());

	isLoaded = true;
}

bbe::INTERNAL::vulkan::VulkanPipeline& bbe::FragmentShader::INTERNAL_getPipeline()
{
	if (!isLoaded)
	{
		throw NotInitializedException();
	}
	return m_pipeline;
}

void bbe::FragmentShader::setPushConstant(PrimitiveBrush2D& brush, uint32_t offset, uint32_t length, const void* data)
{
	if (offset < 80 || offset + length > 128)
	{
		//Only in the range of [80..128) the push constants are guaranteed to be present.
		throw IllegalArgumentException();
	}
	vkCmdPushConstants(brush.INTERNAL_getCurrentCommandBuffer(), brush.INTERNAL_getLayoutPrimitive(), VK_SHADER_STAGE_FRAGMENT_BIT, offset, length, data);
}

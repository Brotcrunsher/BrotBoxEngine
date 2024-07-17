#include "BBE/Vulkan/VulkanFragmentShader.h"
#include "BBE/Vulkan/VulkanManager.h"
#include "BBE/Window.h"

bbe::INTERNAL::vulkan::VulkanFragmentShader::VulkanFragmentShader(const bbe::FragmentShader& shader)
{
	if (shader.m_prendererData != nullptr)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}

	shader.m_prendererData = this;

	m_shader.init(bbe::INTERNAL::vulkan::VulkanManager::s_pinstance->getVulkanDevice(), shader.m_rawData);

	m_pipeline.init(bbe::INTERNAL::vulkan::VulkanManager::s_pinstance->getVertexShader2DPrimitive(), m_shader, bbe::Window::INTERNAL_firstInstance->getWidth(), bbe::Window::INTERNAL_firstInstance->getHeight());
	m_pipeline.addVertexBinding(0, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline.addVertexDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
	m_pipeline.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0, 64);
	m_pipeline.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 64, 64);
	m_pipeline.create(bbe::INTERNAL::vulkan::VulkanManager::s_pinstance->getVulkanDevice().getDevice(), bbe::INTERNAL::vulkan::VulkanManager::s_pinstance->getVulkanRenderPass().getRenderPass());
}

bbe::INTERNAL::vulkan::VulkanFragmentShader::~VulkanFragmentShader()
{
	m_pipeline.destroy();
	m_shader.destroy();
}

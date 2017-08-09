#include "stdafx.h"
#include "BBE/PrimitiveBrush3D.h"
#include "BBE/VulkanDevice.h"
#include "BBE/Color.h"

void bbe::PrimitiveBrush3D::INTERNAL_setColor(float r, float g, float b, float a)
{
	Color c(r, g, b, a);
	vkCmdPushConstants(m_currentCommandBuffer, m_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color), &c);
}

void bbe::PrimitiveBrush3D::INTERNAL_beginDraw(bbe::INTERNAL::vulkan::VulkanDevice & device, VkCommandBuffer commandBuffer, VkPipelineLayout layout, int width, int height)
{
	m_layout = layout;
	m_currentCommandBuffer = commandBuffer;
	m_device = device.getDevice();
	m_physicalDevice = device.getPhysicalDevice();
	m_screenWidth = width;
	m_screenHeight = height;

	setColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void bbe::PrimitiveBrush3D::create(const INTERNAL::vulkan::VulkanDevice &vulkanDevice)
{
	Matrix4 mat;
	m_uboModel.create(vulkanDevice, sizeof(Matrix4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	{
		void* data = m_uboModel.map();
		memcpy(data, &mat, sizeof(Matrix4));
		m_uboModel.unmap();
	}
	

	m_uboViewProjection.create(vulkanDevice, sizeof(Matrix4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	{
		void* data = m_uboViewProjection.map();
		memcpy(data, &mat, sizeof(Matrix4));
		m_uboViewProjection.unmap();
	}
}

void bbe::PrimitiveBrush3D::destroy()
{
	m_uboModel.destroy();
	m_uboViewProjection.destroy();
}

void bbe::PrimitiveBrush3D::setColor(float r, float g, float b, float a)
{
	INTERNAL_setColor(r, g, b, a);
}

void bbe::PrimitiveBrush3D::setColor(float r, float g, float b)
{
	INTERNAL_setColor(r, g, b, 1.0f);
}

void bbe::PrimitiveBrush3D::setColor(const Color & c)
{
	INTERNAL_setColor(c.r, c.g, c.b, c.a);
}

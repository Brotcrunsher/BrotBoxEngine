#include "stdafx.h"
#include "BBE/PrimitiveBrush2D.h"
#include "BBE/VulkanBuffer.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanManager.h"

void bbe::PrimitiveBrush2D::INTERNAL_beginDraw(bbe::INTERNAL::vulkan::VulkanDevice &device, VkCommandBuffer commandBuffer, VkPipelineLayout layout, int width, int height)
{
	m_layout = layout;
	m_currentCommandBuffer = commandBuffer;
	m_device = device.getDevice();
	m_physicalDevice = device.getPhysicalDevice();
	m_screenWidth = width;
	m_screenHeight = height;

	setColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void bbe::PrimitiveBrush2D::INTERNAL_fillRect(Rectangle &rect)
{
	float pushConstants[] = {rect.getX() / m_screenWidth * 2.f - 1.f, rect.getY() / m_screenHeight * 2.f - 1.f, rect.getWidth() / m_screenWidth * 2.f, rect.getHeight() / m_screenHeight * 2.f};



	vkCmdPushConstants(m_currentCommandBuffer, m_layout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(float) * 4, pushConstants);

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = Rectangle::s_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = Rectangle::s_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(m_currentCommandBuffer, 6, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush2D::INTERNAL_setColor(float r, float g, float b, float a)
{
	Color c(r, g, b, a);
	vkCmdPushConstants(m_currentCommandBuffer, m_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color), &c);
}

void bbe::PrimitiveBrush2D::fillRect(Rectangle & rect)
{
	INTERNAL_fillRect(rect);
}

void bbe::PrimitiveBrush2D::fillRect(float x, float y, float width, float height)
{
	if (width < 0)
	{
		x -= width;
		width = -width;
	}

	if (height < 0)
	{
		y -= height;
		height = -height;
	}

	Rectangle rect(x, y, width, height);
	INTERNAL_fillRect(rect);
}

void bbe::PrimitiveBrush2D::setColor(float r, float g, float b, float a)
{
	INTERNAL_setColor(r, g, b, a);
}

void bbe::PrimitiveBrush2D::setColor(float r, float g, float b)
{
	INTERNAL_setColor(r, g, b, 1.0f);
}

void bbe::PrimitiveBrush2D::setColor(Color & c)
{
	INTERNAL_setColor(c.r, c.g, c.b, c.a);
}

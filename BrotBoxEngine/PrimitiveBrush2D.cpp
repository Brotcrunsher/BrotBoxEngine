#include "stdafx.h"
#include "BBE/PrimitiveBrush2D.h"
#include "BBE/VulkanBuffer.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanManager.h"
#include "BBE/Image.h"

void bbe::PrimitiveBrush2D::INTERNAL_beginDraw(
	INTERNAL::vulkan::VulkanDevice &device,
	INTERNAL::vulkan::VulkanCommandPool &commandPool,
	INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool,
	INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayout, 
	VkCommandBuffer commandBuffer, 
	VkPipeline pipelinePrimitive, 
	VkPipelineLayout layoutPrimitive, 
	VkPipeline pipelineImage, 
	VkPipelineLayout layoutImage, 
	int width, int height)
{
	m_layoutPrimitive = layoutPrimitive;
	m_pipelinePrimitive = pipelinePrimitive;
	m_layoutImage = layoutImage;
	m_pipelineImage = pipelineImage;
	m_currentCommandBuffer = commandBuffer;
	m_pdevice = &device;
	m_pcommandPool = &commandPool;
	m_pdescriptorPool = &descriptorPool;
	m_pdescriptorSetLayout = &descriptorSetLayout;
	m_screenWidth = width;
	m_screenHeight = height;

	setColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void bbe::PrimitiveBrush2D::INTERNAL_fillRect(const Rectangle &rect)
{
	if (m_pipelineRecord != PipelineRecord::PRIMITIVE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelinePrimitive);
		m_pipelineRecord = PipelineRecord::PRIMITIVE;
	}

	float pushConstants[] = {rect.getX() / m_screenWidth * 2.f - 1.f, rect.getY() / m_screenHeight * 2.f - 1.f, rect.getWidth() / m_screenWidth * 2.f, rect.getHeight() / m_screenHeight * 2.f};

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(float) * 4, pushConstants);

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = Rectangle::s_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = Rectangle::s_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(m_currentCommandBuffer, 6, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush2D::INTERNAL_drawImage(const Rectangle & rect, const Image & image)
{
	if (m_pipelineRecord != PipelineRecord::IMAGE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineImage);
		m_pipelineRecord = PipelineRecord::IMAGE;
	}

	image.createAndUpload(*m_pdevice, *m_pcommandPool, *m_pdescriptorPool, *m_pdescriptorSetLayout);

	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutImage, 0, 1, image.m_descriptorSet.getPDescriptorSet(), 0, nullptr);

	float pushConstants[] = { rect.getX() / m_screenWidth * 2.f - 1.f, rect.getY() / m_screenHeight * 2.f - 1.f, rect.getWidth() / m_screenWidth * 2.f, rect.getHeight() / m_screenHeight * 2.f };

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(float) * 4, pushConstants);

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = Rectangle::s_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = Rectangle::s_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(m_currentCommandBuffer, 6, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush2D::INTERNAL_fillCircle(const Circle & circle)
{
	if (m_pipelineRecord != PipelineRecord::PRIMITIVE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelinePrimitive);
		m_pipelineRecord = PipelineRecord::PRIMITIVE;
	}
	float pushConstants[] = { circle.getX() / m_screenWidth * 2.f - 1.f, circle.getY() / m_screenHeight * 2.f - 1.f, circle.getWidth() / m_screenWidth * 2.f, circle.getHeight() / m_screenHeight * 2.f };

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(float) * 4, pushConstants);

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = Circle::s_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = Circle::s_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(m_currentCommandBuffer, (Circle::AMOUNTOFVERTICES - 2) * 3, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush2D::INTERNAL_setColor(float r, float g, float b, float a)
{
	Color c(r, g, b, a);
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color), &c);
}

void bbe::PrimitiveBrush2D::fillRect(const Rectangle & rect)
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

void bbe::PrimitiveBrush2D::fillCircle(const Circle & circle)
{
	INTERNAL_fillCircle(circle);
}

void bbe::PrimitiveBrush2D::fillCircle(float x, float y, float width, float height)
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

	Circle circle(x, y, width, height);
	INTERNAL_fillCircle(circle);
}

void bbe::PrimitiveBrush2D::drawImage(const Rectangle & rect, const Image & image)
{
	INTERNAL_drawImage(rect, image);
}

void bbe::PrimitiveBrush2D::drawImage(float x, float y, float width, float height, const Image & image)
{
	INTERNAL_drawImage(Rectangle(x, y, width, height), image);
}

void bbe::PrimitiveBrush2D::setColor(float r, float g, float b, float a)
{
	INTERNAL_setColor(r, g, b, a);
}

void bbe::PrimitiveBrush2D::setColor(float r, float g, float b)
{
	INTERNAL_setColor(r, g, b, 1.0f);
}

void bbe::PrimitiveBrush2D::setColor(const Color & c)
{
	INTERNAL_setColor(c.r, c.g, c.b, c.a);
}

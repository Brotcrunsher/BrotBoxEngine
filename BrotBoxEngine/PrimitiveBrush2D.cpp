#include "BBE/PrimitiveBrush2D.h"
#include "BBE/VulkanBuffer.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanManager.h"
#include "BBE/VulkanPipeline.h"
#include "BBE/Image.h"

void bbe::PrimitiveBrush2D::INTERNAL_beginDraw(
	INTERNAL::vulkan::VulkanDevice &device,
	INTERNAL::vulkan::VulkanCommandPool &commandPool,
	INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool,
	INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayout, 
	VkCommandBuffer commandBuffer,
	INTERNAL::vulkan::VulkanPipeline &pipelinePrimitive,
	INTERNAL::vulkan::VulkanPipeline &pipelineImage,
	int width, int height)
{
	m_layoutPrimitive = pipelinePrimitive.getLayout();
	m_ppipelinePrimitive = &pipelinePrimitive;
	m_layoutImage = pipelineImage.getLayout();
	m_ppipelineImage = &pipelineImage;
	m_currentCommandBuffer = commandBuffer;
	m_pdevice = &device;
	m_pcommandPool = &commandPool;
	m_pdescriptorPool = &descriptorPool;
	m_pdescriptorSetLayout = &descriptorSetLayout;
	m_screenWidth = width;
	m_screenHeight = height;

	m_pipelineRecord = PipelineRecord2D::NONE;

	setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
}

void bbe::PrimitiveBrush2D::INTERNAL_fillRect(const Rectangle &rect)
{
	if (m_pipelineRecord != PipelineRecord2D::PRIMITIVE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelinePrimitive->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord2D::PRIMITIVE;
	}

	
	static float previousWidth  = -10000000;
	static float previousHeight = -10000000;

	if (rect.getWidth() != previousWidth || rect.getHeight() != previousHeight)
	{
		float pushConstants[] = { rect.getX() / m_screenWidth * 2.f - 1.f, rect.getY() / m_screenHeight * 2.f - 1.f, rect.getWidth() / m_screenWidth * 2.f, rect.getHeight() / m_screenHeight * 2.f };
		vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(float) * 4, pushConstants);
		previousWidth = rect.getWidth();
		previousHeight = rect.getHeight();
	}
	else
	{
		float pushConstants[] = { rect.getX() / m_screenWidth * 2.f - 1.f, rect.getY() / m_screenHeight * 2.f - 1.f };
		vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(float) * 2, pushConstants);
	}

	if (m_shapeRecord != ShapeRecord2D::RECTANGLE) {
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffer = Rectangle::s_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

		buffer = Rectangle::s_indexBuffer.getBuffer();
		vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
		m_shapeRecord = ShapeRecord2D::RECTANGLE;
	}

	vkCmdDrawIndexed(m_currentCommandBuffer, 6, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush2D::INTERNAL_drawImage(const Rectangle & rect, const Image & image)
{
	if (m_pipelineRecord != PipelineRecord2D::IMAGE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelineImage->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord2D::IMAGE;
	}

	image.createAndUpload(*m_pdevice, *m_pcommandPool, *m_pdescriptorPool, *m_pdescriptorSetLayout);

	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutImage, 0, 1, image.getDescriptorSet().getPDescriptorSet(), 0, nullptr);

	float pushConstants[] = { rect.getX() / m_screenWidth * 2.f - 1.f, rect.getY() / m_screenHeight * 2.f - 1.f, rect.getWidth() / m_screenWidth * 2.f, rect.getHeight() / m_screenHeight * 2.f };

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(float) * 4, pushConstants);


	if (m_shapeRecord != ShapeRecord2D::RECTANGLE) {
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffer = Rectangle::s_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

		buffer = Rectangle::s_indexBuffer.getBuffer();
		vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
		m_shapeRecord = ShapeRecord2D::RECTANGLE;
	}

	vkCmdDrawIndexed(m_currentCommandBuffer, 6, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush2D::INTERNAL_fillCircle(const Circle & circle)
{
	if (m_pipelineRecord != PipelineRecord2D::PRIMITIVE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelinePrimitive->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord2D::PRIMITIVE;
	}
	float pushConstants[] = { circle.getX() / m_screenWidth * 2.f - 1.f, circle.getY() / m_screenHeight * 2.f - 1.f, circle.getWidth() / m_screenWidth * 2.f, circle.getHeight() / m_screenHeight * 2.f };

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(float) * 4, pushConstants);


	if (m_shapeRecord != ShapeRecord2D::CIRCLE) {
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffer = Circle::s_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

		buffer = Circle::s_indexBuffer.getBuffer();
		vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
		m_shapeRecord = ShapeRecord2D::CIRCLE;
	}

	vkCmdDrawIndexed(m_currentCommandBuffer, (Circle::AMOUNTOFVERTICES - 2) * 3, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush2D::INTERNAL_setColor(float r, float g, float b, float a)
{
	static Color lastColor(-1000, -1000, -1000);
	Color c(r, g, b, a);
	if (c.r != lastColor.r || c.g != lastColor.g || c.b != lastColor.b || c.a != lastColor.a)
	{
		vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color), &c);
		lastColor = c;
	}
}

bbe::PrimitiveBrush2D::PrimitiveBrush2D()
{
	m_screenHeight = -1;
	m_screenWidth  = -1;
	//do nothing
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

void bbe::PrimitiveBrush2D::setColorRGB(float r, float g, float b, float a)
{
	INTERNAL_setColor(r, g, b, a);
}

void bbe::PrimitiveBrush2D::setColorRGB(const Vector3& c)
{
	//UNTESTED
	setColorRGB(c.x, c.y, c.z);
}

void bbe::PrimitiveBrush2D::setColorRGB(float r, float g, float b)
{
	INTERNAL_setColor(r, g, b, 1.0f);
}

void bbe::PrimitiveBrush2D::setColorRGB(const Color & c)
{
	INTERNAL_setColor(c.r, c.g, c.b, c.a);
}

void bbe::PrimitiveBrush2D::setColorHSV(float h, float s, float v, float a)
{
	auto rgb = bbe::Color::HSVtoRGB(h, s, v);
	setColorRGB(rgb.x, rgb.y, rgb.z, a);
}

void bbe::PrimitiveBrush2D::setColorHSV(float h, float s, float v)
{
	setColorHSV(h, s, v, 1);
}

void bbe::PrimitiveBrush2D::setFillMode(FillMode fm)
{
	m_fillMode = fm;
}

bbe::FillMode bbe::PrimitiveBrush2D::getFillMode()
{
	return m_fillMode;
}

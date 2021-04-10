#include "BBE/PrimitiveBrush2D.h"
#include "BBE/VulkanBuffer.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanManager.h"
#include "BBE/VulkanPipeline.h"
#include "BBE/Image.h"
#include "BBE/Math.h"
#include "BBE/FragmentShader.h"
#include "BBE/RectangleRotated.h"

void bbe::PrimitiveBrush2D::INTERNAL_beginDraw(
	INTERNAL::vulkan::VulkanDevice &device,
	INTERNAL::vulkan::VulkanCommandPool &commandPool,
	INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool,
	INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayout, 
	VkCommandBuffer commandBuffer,
	INTERNAL::vulkan::VulkanPipeline &pipelinePrimitive,
	INTERNAL::vulkan::VulkanPipeline &pipelineImage,
	GLFWwindow* window,
	int width, int height,
	uint32_t imageIndex)
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
	setOutlineRGB(1.0f, 1.0f, 1.0f, 1.0f);
	setOutlineWidth(0.f);

	float pushConstants[] = { static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight) };
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, 24, sizeof(float) * 2, pushConstants);

	glfwGetWindowContentScale(window, &m_windowXScale, &m_windowYScale);

	for (size_t i = 0; i < m_delayedBufferDeletes[imageIndex].getLength(); i++)
	{
		vkDestroyBuffer(m_pdevice->getDevice(), m_delayedBufferDeletes[imageIndex][i].m_buffer, nullptr);
		vkFreeMemory   (m_pdevice->getDevice(), m_delayedBufferDeletes[imageIndex][i].m_memory, nullptr);
	}
	m_delayedBufferDeletes[imageIndex].clear();
	m_imageIndex = imageIndex;
}

void bbe::PrimitiveBrush2D::INTERNAL_init(const uint32_t amountOfFrames)
{
	if (m_delayedBufferDeletes.getLength() < amountOfFrames)
	{
		m_delayedBufferDeletes.resizeCapacityAndLength(amountOfFrames);
	}
}

void bbe::PrimitiveBrush2D::INTERNAL_bindRectBuffers()
{
	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = bbe::Rectangle::s_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = Rectangle::s_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
}

void bbe::PrimitiveBrush2D::INTERNAL_fillRect(const Rectangle &rect, float rotation, float outlineWidth, FragmentShader* shader)
{
	if (outlineWidth > 0)
	{
		Color oldColor = m_color;
		setColorRGB(m_outlineColor);
		INTERNAL_fillRect(rect, rotation, 0, shader);
		setColorRGB(oldColor);
	}
	if (shader != nullptr)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->INTERNAL_getPipeline().getPipeline(m_fillMode));
	}
	else if (m_pipelineRecord != PipelineRecord2D::PRIMITIVE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelinePrimitive->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord2D::PRIMITIVE;
	}

	float pushConstants[] = {
		(rect.getX() + outlineWidth + m_offset.x) * m_windowXScale, 
		(rect.getY() + outlineWidth + m_offset.y) * m_windowYScale, 
		(rect.getWidth()  - outlineWidth * 2) * m_windowXScale,
		(rect.getHeight() - outlineWidth * 2) * m_windowYScale,
		rotation
	};
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 5, pushConstants);

	INTERNAL_bindRectBuffers();

	vkCmdDrawIndexed(m_currentCommandBuffer, 6, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush2D::INTERNAL_drawImage(const Rectangle & rect, const Image & image, float rotation)
{
	if (m_pipelineRecord != PipelineRecord2D::IMAGE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelineImage->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord2D::IMAGE;
	}

	image.createAndUpload(*m_pdevice, *m_pcommandPool, *m_pdescriptorPool, *m_pdescriptorSetLayout);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutImage, 0, 1, image.getDescriptorSet().getPDescriptorSet(), 0, nullptr);

	float pushConstants[] = {
		(rect.getX() + m_offset.x) * m_windowXScale,
		(rect.getY() + m_offset.y) * m_windowYScale,
		rect.getWidth() * m_windowXScale,
		rect.getHeight() * m_windowYScale
	};
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 4, pushConstants);

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, 16, sizeof(float), &rotation);

	INTERNAL_bindRectBuffers();

	vkCmdDrawIndexed(m_currentCommandBuffer, 6, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush2D::INTERNAL_fillCircle(const Circle & circle, float outlineWidth)
{
	if (outlineWidth > 0)
	{
		Color oldColor = m_color;
		setColorRGB(m_outlineColor);
		INTERNAL_fillCircle(circle, 0);
		setColorRGB(oldColor);
	}
	if (m_pipelineRecord != PipelineRecord2D::PRIMITIVE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelinePrimitive->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord2D::PRIMITIVE;
	}
	float pushConstants[] = {
		(circle.getX() + outlineWidth + m_offset.x) * m_windowXScale, 
		(circle.getY() + outlineWidth + m_offset.y) * m_windowYScale, 
		(circle.getWidth() - outlineWidth * 2) * m_windowXScale, 
		(circle.getHeight() - outlineWidth * 2) * m_windowYScale,
		0
	};

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 5, pushConstants);


	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = Circle::s_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = Circle::s_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(m_currentCommandBuffer, (Circle::AMOUNTOFVERTICES - 2) * 3, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush2D::INTERNAL_setColor(float r, float g, float b, float a)
{
	Color newColor(r, g, b, a);
	if (newColor.r != m_color.r || newColor.g != m_color.g || newColor.b != m_color.b || newColor.a != m_color.a)
	{
		vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_FRAGMENT_BIT, 64, sizeof(Color), &newColor);
		m_color = newColor;
	}
}

void bbe::PrimitiveBrush2D::INTERNAL_destroy()
{
	for (size_t i = 0; i < m_delayedBufferDeletes.getLength(); i++)
	{
		for (size_t k = 0; k < m_delayedBufferDeletes[i].getLength(); k++)
		{
			vkDestroyBuffer(m_pdevice->getDevice(), m_delayedBufferDeletes[i][k].m_buffer, nullptr);
			vkFreeMemory   (m_pdevice->getDevice(), m_delayedBufferDeletes[i][k].m_memory, nullptr);
		}
	}
}

bbe::PrimitiveBrush2D::PrimitiveBrush2D()
{
	m_screenHeight = -1;
	m_screenWidth  = -1;
	//do nothing
}

void bbe::PrimitiveBrush2D::fillRect(const Rectangle & rect, float rotation, FragmentShader* shader)
{
	INTERNAL_fillRect(rect, rotation, m_outlineWidth, shader);
}

void bbe::PrimitiveBrush2D::fillRect(float x, float y, float width, float height, float rotation, FragmentShader* shader)
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
	INTERNAL_fillRect(rect, rotation, m_outlineWidth, shader);
}

void bbe::PrimitiveBrush2D::fillRect(const Vector2& pos, float width, float height, float rotation, FragmentShader* shader)
{
	fillRect(pos.x, pos.y, width, height, rotation, shader);
}

void bbe::PrimitiveBrush2D::fillRect(float x, float y, const Vector2& dimensions, float rotation, FragmentShader* shader)
{
	fillRect(x, y, dimensions.x, dimensions.y, rotation, shader);
}

void bbe::PrimitiveBrush2D::fillRect(const Vector2& pos, const Vector2& dimensions, float rotation, FragmentShader* shader)
{
	fillRect(pos.x, pos.y, dimensions.x, dimensions.y, rotation, shader);
}

void bbe::PrimitiveBrush2D::fillRect(const PhysRectangle& rect, FragmentShader* shader)
{
	fillRect(rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight(), rect.getAngle(), shader);
}

void bbe::PrimitiveBrush2D::fillRect(const RectangleRotated& rect, FragmentShader* shader)
{
	fillRect(rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight(), rect.getRotation(), shader);
}

void bbe::PrimitiveBrush2D::fillCircle(const Circle & circle)
{
	INTERNAL_fillCircle(circle, m_outlineWidth);
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
	INTERNAL_fillCircle(circle, m_outlineWidth);
}

void bbe::PrimitiveBrush2D::fillCircle(const Vector2& pos, float width, float height)
{
	fillCircle(pos.x, pos.y, width, height);
}

void bbe::PrimitiveBrush2D::fillCircle(float x, float y, const Vector2& dimensions)
{
	fillCircle(x, y, dimensions.x, dimensions.y);
}

void bbe::PrimitiveBrush2D::fillCircle(const Vector2& pos, const Vector2& dimensions)
{
	fillCircle(pos.x, pos.y, dimensions.x, dimensions.y);
}

void bbe::PrimitiveBrush2D::fillCircle(const PhysCircle& circle)
{
	fillCircle(circle.getX(), circle.getY(), circle.getRadius() * 2, circle.getRadius() * 2);
}

void bbe::PrimitiveBrush2D::drawImage(const Rectangle & rect, const Image & image, float rotation)
{
	INTERNAL_drawImage(rect, image, rotation);
}

void bbe::PrimitiveBrush2D::drawImage(float x, float y, float width, float height, const Image & image, float rotation)
{
	INTERNAL_drawImage(Rectangle(x, y, width, height), image, rotation);
}

void bbe::PrimitiveBrush2D::drawImage(const Vector2& pos, float width, float height, const Image& image, float rotation)
{
	INTERNAL_drawImage(Rectangle(pos.x, pos.y, width, height), image, rotation);
}

void bbe::PrimitiveBrush2D::fillTriangle(const Vector2& a, const Vector2& b, const Vector2& c)
{
	fillVertexIndexList({ 0, 1, 2 }, { a, b, c });
}

void bbe::PrimitiveBrush2D::drawImage(float x, float y, const Vector2& dimensions, const Image& image, float rotation)
{
	INTERNAL_drawImage(Rectangle(x, y, dimensions.x, dimensions.y), image, rotation);
}

void bbe::PrimitiveBrush2D::drawImage(const Vector2& pos, const Vector2& dimensions, const Image& image, float rotation)
{
	INTERNAL_drawImage(Rectangle(pos.x, pos.y, dimensions.x, dimensions.y), image, rotation);
}

void bbe::PrimitiveBrush2D::fillLine(float x1, float y1, float x2, float y2, float lineWidth)
{
	fillLine(Vector2(x1, y1), Vector2(x2, y2), lineWidth);
}

void bbe::PrimitiveBrush2D::fillLine(const Vector2& p1, float x2, float y2, float lineWidth)
{
	fillLine(p1, Vector2(x2, y2), lineWidth);
}

void bbe::PrimitiveBrush2D::fillLine(float x1, float y1, const Vector2& p2, float lineWidth)
{
	fillLine(Vector2(x1, y1), p2, lineWidth);
}

void bbe::PrimitiveBrush2D::fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint, const bbe::List<Vector2>& controlPoints)
{
	fillBezierCurve(BezierCurve2(startPoint, endPoint, controlPoints));
}

void bbe::PrimitiveBrush2D::fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint)
{
	fillBezierCurve(BezierCurve2(startPoint, endPoint));
}

void bbe::PrimitiveBrush2D::fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint, const Vector2& control)
{
	fillBezierCurve(BezierCurve2(startPoint, endPoint, control));
}

void bbe::PrimitiveBrush2D::fillArrow(const Vector2& p1, const Vector2& p2, float tailWidth, float spikeInnerLength, float spikeOuterLength, float spikeAngle, bool dynamicSpikeLength)
{
	const Vector2 dir = p2 - p1;
	const Vector2 dirNorm = dir.normalize();
	const Vector2 leftNorm = dirNorm.rotate90CounterClockwise();

	if (dynamicSpikeLength)
	{
		const float dirLength = dir.getLength();
		if (dirLength < spikeInnerLength)
		{
			const float ratio = dirLength / spikeInnerLength;
			spikeInnerLength = dirLength;
			spikeOuterLength *= ratio;
		}
	}

	const Vector2 spikeStart = p2 - dirNorm * spikeInnerLength;
	const Vector2 tailEndLeft = spikeStart + leftNorm * tailWidth * 0.5f;
	const Vector2 tailEndRight = spikeStart - leftNorm * tailWidth * 0.5f;
	const Vector2 tailStartLeft = p1 + leftNorm * tailWidth * 0.5f;
	const Vector2 tailStartRight = p1 - leftNorm * tailWidth * 0.5f;

	const Vector2 spikeStartLeft  = (spikeStart - p2).withLenght(spikeOuterLength).rotate(+spikeAngle) + p2;
	const Vector2 spikeStartRight = (spikeStart - p2).withLenght(spikeOuterLength).rotate(-spikeAngle) + p2;

	const Vector2 tailSpikeIntersectionLeft  = bbe::Line2{ tailStartLeft,  tailEndLeft } .getIntersection(bbe::Line2{ spikeStart, spikeStartLeft });
	const Vector2 tailSpikeIntersectionRight = bbe::Line2{ tailStartRight, tailEndRight }.getIntersection(bbe::Line2{ spikeStart, spikeStartRight });

	fillVertexIndexList({0, 4, 6, 1, 6, 5, 0, 6, 1, 2, 0, 1, 2, 1, 3}, { tailSpikeIntersectionLeft, tailSpikeIntersectionRight, tailStartLeft, tailStartRight, spikeStartLeft, spikeStartRight, p2});
}

void bbe::PrimitiveBrush2D::fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint, const Vector2& control1, const Vector2& control2)
{
	fillBezierCurve(BezierCurve2(startPoint, endPoint, control1, control2));
}

void bbe::PrimitiveBrush2D::fillBezierCurve(const BezierCurve2& bc, float lineWidth)
{
	bbe::Vector2 previousPoint = bc.getStartPoint();
	for (float t = 0; t <= 1; t += 0.01f)
	{
		const bbe::Vector2 currentPoint = bc.evaluate(t);

		fillLine(previousPoint, currentPoint, lineWidth);

		previousPoint = currentPoint;
	}
}

void bbe::PrimitiveBrush2D::fillLine(const Vector2& p1, const Vector2& p2, float lineWidth)
{
	const Vector2 dir = p2 - p1;
	const float dist = dir.getLength();
	if (dist == 0) return;
	const Vector2 midPoint = p1 + dir * 0.5;
	const Vector2 topLeft = midPoint - Vector2(lineWidth / 2, dist / 2);
	const Rectangle rect(topLeft, lineWidth, dist);
	const float angle = dir.getAngle() - bbe::Math::toRadians(90);

	fillRect(rect, angle);
}

void bbe::PrimitiveBrush2D::fillLine(const Line2& line, float lineWidth)
{
	fillLine(line.m_start, line.m_stop, lineWidth);
}

void bbe::PrimitiveBrush2D::fillLineStrip(const bbe::List<bbe::Vector2> &points, bool closed, float lineWidth)
{
	for (size_t i = 1; i < points.getLength(); i++)
	{
		fillLine(points[i - 1], points[i], lineWidth);
	}
	if (closed && points.getLength() > 0)
	{
		fillLine(points[0], points[points.getLength() - 1], lineWidth);
	}
}

void bbe::PrimitiveBrush2D::fillText(float x, float y, const char* text, const bbe::Font& font)
{
	fillText(Vector2(x, y), text, font);
}

void bbe::PrimitiveBrush2D::fillText(const Vector2& p, const char* text, const bbe::Font& font)
{
	const float lineStart = p.x;
	
	Vector2 currentPosition = p;

	while (*text)
	{
		if (*text == '\n')
		{
			currentPosition.x = lineStart;
			currentPosition.y += font.getPixelsFromLineToLine();
		}
		else if (*text == ' ')
		{
			currentPosition.x += font.getLeftSideBearing(*text) + font.getAdvanceWidth(*text);
		}
		else
		{
			currentPosition.x += font.getLeftSideBearing(*text);
			const bbe::Image& charImage = font.getImage(*text);
			drawImage(currentPosition.x, currentPosition.y + font.getVerticalOffset(*text), charImage.getDimensions(), charImage);
			currentPosition.x += font.getAdvanceWidth(*text);
		}

		text++;
	}
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

void bbe::PrimitiveBrush2D::setColorRGB(const Vector3& c, float a)
{
	setColorRGB(c.x, c.y, c.z, a);
}

void bbe::PrimitiveBrush2D::setColorRGB(float r, float g, float b)
{
	INTERNAL_setColor(r, g, b, 1.0f);
}

void bbe::PrimitiveBrush2D::setColorRGB(const Color & c)
{
	INTERNAL_setColor(c.r, c.g, c.b, c.a);
}

void bbe::PrimitiveBrush2D::setOutlineRGB(float r, float g, float b, float a)
{
	setOutlineRGB(bbe::Color(r, g, b, a));
}

void bbe::PrimitiveBrush2D::setOutlineRGB(float r, float g, float b)
{
	setOutlineRGB(bbe::Color(r, g, b, 1));
}

void bbe::PrimitiveBrush2D::setOutlineRGB(const Vector3& c)
{
	setOutlineRGB(bbe::Color(c.x, c.y, c.z, 1));
}

void bbe::PrimitiveBrush2D::setOutlineRGB(const Vector3& c, float a)
{
	setOutlineRGB(bbe::Color(c.x, c.y, c.z, a));
}

void bbe::PrimitiveBrush2D::setOutlineRGB(const Color& c)
{
	m_outlineColor = c;
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

void bbe::PrimitiveBrush2D::setOutlineHSV(float h, float s, float v, float a)
{
	auto rgb = bbe::Color::HSVtoRGB(h, s, v);
	setOutlineRGB(rgb, a);
}

void bbe::PrimitiveBrush2D::setOutlineHSV(float h, float s, float v)
{
	setOutlineHSV(h, s, v, 1);
}

void bbe::PrimitiveBrush2D::setOffset(float x, float y)
{
	setOffset(bbe::Vector2{ x, y });
}

void bbe::PrimitiveBrush2D::setOffset(const bbe::Vector2& offset)
{
	m_offset = offset;
}

bbe::Vector2 bbe::PrimitiveBrush2D::getOffset() const
{
	return m_offset;
}

void bbe::PrimitiveBrush2D::setOutlineWidth(float outlineWidht)
{
	m_outlineWidth = outlineWidht;
}

void bbe::PrimitiveBrush2D::setFillMode(FillMode fm)
{
	m_fillMode = fm;
}

bbe::FillMode bbe::PrimitiveBrush2D::getFillMode()
{
	return m_fillMode;
}

VkCommandBuffer bbe::PrimitiveBrush2D::INTERNAL_getCurrentCommandBuffer()
{
	return m_currentCommandBuffer;
}

VkPipelineLayout bbe::PrimitiveBrush2D::INTERNAL_getLayoutPrimitive()
{
	return m_layoutPrimitive;
}

void bbe::PrimitiveBrush2D::fillVertexIndexList(const bbe::List<uint32_t>& indices, const bbe::List<bbe::Vector2>& vertices)
{
	fillVertexIndexList(indices.getRaw(), indices.getLength(), vertices.getRaw(), vertices.getLength());
}

void bbe::PrimitiveBrush2D::fillVertexIndexList(const uint32_t* indices, uint32_t amountOfIndices, const bbe::Vector2* vertices, uint32_t amountOfVertices)
{
	// These two asserts make sure that we can cast the vertices array to a float array.
	static_assert(alignof(bbe::Vector2) == alignof(float));
	static_assert(sizeof(bbe::Vector2) == 2 * sizeof(float));

	bbe::INTERNAL::vulkan::VulkanBuffer indexBuffer;
	bbe::INTERNAL::vulkan::VulkanBuffer vertexBuffer;
	indexBuffer.create(m_pdevice->getDevice(), m_pdevice->getPhysicalDevice(), sizeof(uint32_t) * amountOfIndices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void* indexDataBuf = indexBuffer.map();
	memcpy(indexDataBuf, indices, sizeof(uint32_t) * amountOfIndices);
	indexBuffer.unmap();

	vertexBuffer.create(m_pdevice->getDevice(), m_pdevice->getPhysicalDevice(), sizeof(Vector2) * amountOfVertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	void* vertexDataBuf = vertexBuffer.map();
	memcpy(vertexDataBuf, (float*)vertices, sizeof(Vector2) * amountOfVertices);
	vertexBuffer.unmap();

	if (m_pipelineRecord != PipelineRecord2D::PRIMITIVE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelinePrimitive->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord2D::PRIMITIVE;
	}

	float pushConstants[] = {
		m_offset.x * m_windowXScale,
		m_offset.y * m_windowYScale,
		m_windowXScale,
		m_windowYScale,
		0
	};
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 5, pushConstants);

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(m_currentCommandBuffer, amountOfIndices, 1, 0, 0, 0);

	m_delayedBufferDeletes[m_imageIndex].add({ indexBuffer .getBuffer(), indexBuffer .getMemory() });
	m_delayedBufferDeletes[m_imageIndex].add({ vertexBuffer.getBuffer(), vertexBuffer.getMemory() });
}

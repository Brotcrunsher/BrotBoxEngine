#include "stdafx.h"
#include "BBE/Rectangle.h"
#include "BBE/Vector2.h"
#include "BBE/VulkanDevice.h"

bbe::INTERNAL::vulkan::VulkanBuffer* bbe::Rectangle::getVulkanVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, int screenWidth, int screenHeight)
{
	if (m_bufferDirty)
	{
		m_vertexBuffer.destroy();
	}
	if (m_vertexBuffer.m_wasCreated == false)
	{
		createVertexBuffer(device, physicalDevice, screenWidth, screenHeight);
	}
	return &m_vertexBuffer;
}

bbe::INTERNAL::vulkan::VulkanBuffer * bbe::Rectangle::getVulkanIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice)
{
	if (m_bufferDirty)
	{
		m_indexBuffer.destroy();
	}
	if (m_indexBuffer.m_wasCreated == false)
	{
		createIndexBuffer(device, physicalDevice);
	}
	return &m_indexBuffer;
}

void bbe::Rectangle::createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, int screenWidth, int screenHeight)
{
	m_vertexBuffer.destroy();

	m_vertexBuffer.create(device, physicalDevice, sizeof(Vector2) * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	//todo upload buffer

	float x = (m_x / screenWidth ) * 2 - 1;
	float y = (m_y / screenHeight) * 2 - 1;
	float width = (m_width / screenWidth) * 2;
	float height = (m_height / screenHeight) * 2;

	float data[] = {
		 x        , y         ,
		 x + width, y         ,
		 x + width, y + height,
		 x        , y + height,
	};
	void* dataBuf = m_vertexBuffer.map();
	memcpy(dataBuf, data, sizeof(Vector2) * 4);
	m_vertexBuffer.unmap();
	m_bufferDirty = false;
}

void bbe::Rectangle::createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice)
{
	//TODO the indexbuffer should stay the same for all Rectangles? Maybe make it static and upload only a single one?
	m_indexBuffer.destroy();

	m_indexBuffer.create(device, physicalDevice, sizeof(uint32_t) * 6, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	static uint32_t data[] = {
		0, 1, 2, 0, 2, 3
	};
	void* dataBuf = m_indexBuffer.map();
	memcpy(dataBuf, data, sizeof(uint32_t) * 6);
	m_indexBuffer.unmap();

}

bbe::Rectangle::Rectangle()
	: m_x(0), m_y(0), m_width(0), m_height(0)
{
}

bbe::Rectangle::Rectangle(float x, float y, float width, float height)
	: m_x(x), m_y(y), m_width(width), m_height(height)
{
}

bbe::Rectangle::~Rectangle()
{
	m_vertexBuffer.destroyAtEndOfFrame();
	m_indexBuffer .destroyAtEndOfFrame();
}

bbe::Rectangle::Rectangle(const Rectangle &other)
{
	set(other.getX(), other.getY(), other.getWidth(), other.getHeight());
}

bbe::Rectangle::Rectangle(Rectangle &&other)
{
	set(other.getX(), other.getY(), other.getWidth(), other.getHeight());
	if (other.m_bufferDirty == false)
	{
		m_vertexBuffer = std::move(other.m_vertexBuffer);
		m_indexBuffer = std::move(other.m_indexBuffer);
		m_bufferDirty = false;
	}
}

bbe::Rectangle & bbe::Rectangle::operator=(const Rectangle &other)
{
	m_vertexBuffer.destroy();
	m_indexBuffer.destroy();
	set(other.getX(), other.getY(), other.getWidth(), other.getHeight());
	m_bufferDirty = true;
	return *this;
}

bbe::Rectangle & bbe::Rectangle::operator=(Rectangle &&other)
{
	m_vertexBuffer.destroy();
	m_indexBuffer.destroy();
	set(other.getX(), other.getY(), other.getWidth(), other.getHeight());
	m_bufferDirty = true;
	if (other.m_bufferDirty == false)
	{
		m_vertexBuffer = std::move(other.m_vertexBuffer);
		m_indexBuffer = std::move(other.m_indexBuffer);
		m_bufferDirty = false;
	}
	return *this;
}

float bbe::Rectangle::getX() const
{
	return m_x;
}

float bbe::Rectangle::getY() const
{
	return m_y;
}

float bbe::Rectangle::getWidth() const
{
	return m_width;
}

float bbe::Rectangle::getHeight() const
{
	return m_height;
}

void bbe::Rectangle::setX(float x)
{
	m_x = x;
}

void bbe::Rectangle::setY(float y)
{
	m_y = y;
}

void bbe::Rectangle::setWidth(float width)
{
	m_width = width;
}

void bbe::Rectangle::setHeight(float height)
{
	m_height = height;
}

void bbe::Rectangle::set(float x, float y, float width, float height)
{
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;
}

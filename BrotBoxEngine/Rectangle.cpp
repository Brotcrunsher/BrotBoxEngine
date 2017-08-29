#include "stdafx.h"
#include "BBE/Rectangle.h"
#include "BBE/Vector2.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanManager.h"

bbe::INTERNAL::vulkan::VulkanBuffer bbe::Rectangle::s_indexBuffer;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::Rectangle::s_vertexBuffer;

void bbe::Rectangle::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue)
{
	s_initVertexBuffer(device, physicalDevice, commandPool, queue);
	s_initIndexBuffer (device, physicalDevice, commandPool, queue);
}

void bbe::Rectangle::s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue)
{
	s_indexBuffer.create(device, physicalDevice, sizeof(uint32_t) * 6, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	uint32_t data[] = {
		0, 1, 2, 0, 2, 3
	};
	void* dataBuf = s_indexBuffer.map();
	memcpy(dataBuf, data, sizeof(uint32_t) * 6);
	s_indexBuffer.unmap();

	s_indexBuffer.upload(commandPool, queue);
}

void bbe::Rectangle::s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue)
{
	s_vertexBuffer.create(device, physicalDevice, sizeof(float) * 8, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	float data[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
	};

	void* dataBuf = s_vertexBuffer.map();
	memcpy(dataBuf, data, sizeof(float) * 8);
	s_vertexBuffer.unmap();

	s_vertexBuffer.upload(commandPool, queue);
}

void bbe::Rectangle::s_destroy()
{
	s_indexBuffer.destroy();
	s_vertexBuffer.destroy();
}

bbe::Rectangle::Rectangle()
	: m_x(0), m_y(0), m_width(0), m_height(0)
{
}

bbe::Rectangle::Rectangle(float x, float y, float width, float height)
	: m_x(x), m_y(y), m_width(width), m_height(height)
{
}

bbe::Rectangle::Rectangle(const Vector2 & vec, float width, float height)
	: m_x(vec.x), m_y(vec.y), m_width(width), m_height(height)
{
}

bbe::Rectangle::Rectangle(float x, float y, const Vector2 & dim)
	: m_x(x), m_y(y), m_width(dim.x), m_height(dim.y)
{
}

bbe::Rectangle::Rectangle(const Vector2 & vec, const Vector2 & dim)
	: m_x(vec.x), m_y(vec.y), m_width(dim.x), m_height(dim.y)
{
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

void bbe::Rectangle::setPos(float x, float y)
{
	m_x = x;
	m_y = y;
}

void bbe::Rectangle::setPos(const Vector2 & vec)
{
	m_x = vec.x;
	m_y = vec.y;
}

void bbe::Rectangle::setWidth(float width)
{
	m_width = width;
}

void bbe::Rectangle::setHeight(float height)
{
	m_height = height;
}

void bbe::Rectangle::setDim(float width, float height)
{
	m_width = width;
	m_height = height;
}

void bbe::Rectangle::setDim(const Vector2 & vec)
{
	m_width = vec.x;
	m_height = vec.y;
}

void bbe::Rectangle::set(float x, float y, float width, float height)
{
	setX(x);
	setY(y);
	setWidth(width);
	setHeight(height);
}

void bbe::Rectangle::translate(float x, float y)
{
	m_x += x;
	m_y += y;
}

void bbe::Rectangle::translate(const Vector2 & vec)
{
	translate(vec.x, vec.y);
}

float bbe::Rectangle::getDistanceTo(const Vector2 & vec)
{
	//UNTESTED
	if (vec.x < m_x)
	{
		if (vec.y < m_y)
		{
			return vec.getDistanceTo(Vector2(m_x, m_y));
		}
		else if (vec.y > m_y + m_height)
		{
			return vec.getDistanceTo(Vector2(m_x, m_y + m_height));
		}
		else
		{
			return m_x - vec.x;
		}
	}
	else if (vec.x > m_x + m_width)
	{
		if (vec.y < m_y)
		{
			return vec.getDistanceTo(Vector2(m_x + m_width, m_y));
		}
		else if (vec.y > m_y + m_height)
		{
			return vec.getDistanceTo(Vector2(m_x + m_width, m_y + m_height));
		}
		else
		{
			return vec.x - (m_x + m_width);
		}
	}
	else if (vec.y < m_y)
	{
		return m_y - vec.y;
	}
	else if (vec.y > m_y + m_height)
	{
		return vec.y - (m_y + m_height);
	}
	else
	{
		return 0;
	}
}

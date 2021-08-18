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

bbe::Vector2 bbe::Rectangle::getPos() const
{
	return Vector2(m_x, m_y);
}

float bbe::Rectangle::getX() const
{
	return m_x;
}

float bbe::Rectangle::getY() const
{
	return m_y;
}

bbe::Vector2 bbe::Rectangle::getDim() const
{
	return Vector2(m_width, m_height);
}

bbe::Rectangle bbe::Rectangle::combine(const Rectangle& other) const
{
	const float left   = bbe::Math::min(this->getLeft(), other.getLeft());
	const float right  = bbe::Math::max(this->getRight(), other.getRight());
	const float top    = bbe::Math::min(this->getTop(), other.getTop());
	const float bottom = bbe::Math::max(this->getBottom(), other.getBottom());
	return Rectangle(
		left, 
		top,
		right - left,
		bottom - top
	);
}

bbe::Rectangle bbe::Rectangle::offset(const Vector2& off) const
{
	return Rectangle(m_x + off.x, m_y + off.y, m_width, m_height);
}

float bbe::Rectangle::getLeft() const
{
	return bbe::Math::min(m_x, m_x + m_width);
}

float bbe::Rectangle::getRight() const
{
	return bbe::Math::max(m_x, m_x + m_width);
}

float bbe::Rectangle::getTop() const
{
	return bbe::Math::min(m_y, m_y + m_height);
}

float bbe::Rectangle::getBottom() const
{
	return bbe::Math::max(m_y, m_y + m_height);
}

float bbe::Rectangle::getWidth() const
{
	return m_width;
}

float bbe::Rectangle::getHeight() const
{
	return m_height;
}

bbe::Vector2 bbe::Rectangle::getCenter() const
{
	return bbe::Vector2(
		getX() + getWidth()  / 2,
		getY() + getHeight() / 2
	);
}

void bbe::Rectangle::getVertices(bbe::List<bbe::Vector2>& outVertices) const
{
	outVertices.clear();

	outVertices.add({ m_x,           m_y });
	outVertices.add({ m_x,           m_y + m_height });
	outVertices.add({ m_x + m_width, m_y + m_height });
	outVertices.add({ m_x + m_width, m_y });
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

bool bbe::Rectangle::isPointInRectangle(const Vector2 point) const
{
	return point.x > this->getX() && point.x < this->getX() + this->getWidth()
		&& point.y > this->getY() && point.y < this->getY() + this->getHeight();
}

bool bbe::Rectangle::intersects(const Rectangle& rectangle) const
{
	const Rectangle hitZone(
		rectangle.getX() - this->getWidth(),
		rectangle.getY() - this->getHeight(),
		this->getWidth()  + rectangle.getWidth(),
		this->getHeight() + rectangle.getHeight()
	);
	return hitZone.isPointInRectangle(this->getPos());
}

bool bbe::Rectangle::intersects(const Circle& circle) const
{
	if (circle.getWidth() != circle.getHeight()) throw NotImplementedException();

	const bbe::Vector2 circleMidPoint = circle.getPos() - circle.getDim() / 2;

	if (   circleMidPoint.x >= m_x           - circle.getWidth() / 2
		&& circleMidPoint.x <= m_x + m_width + circle.getWidth() / 2
		&& circleMidPoint.y >= m_y
		&& circleMidPoint.y <= m_y + m_height)
		return true;

	if (   circleMidPoint.x >= m_x
		&& circleMidPoint.x <= m_x + m_width
		&& circleMidPoint.y >= m_y            - circle.getHeight() / 2
		&& circleMidPoint.y <= m_y + m_height + circle.getHeight() / 2)
		return true;

	if (circleMidPoint.getDistanceTo(m_x          , m_y           ) < circle.getWidth() / 2) return true;
	if (circleMidPoint.getDistanceTo(m_x + m_width, m_y           ) < circle.getWidth() / 2) return true;
	if (circleMidPoint.getDistanceTo(m_x          , m_y + m_height) < circle.getWidth() / 2) return true;
	if (circleMidPoint.getDistanceTo(m_x + m_width, m_y + m_height) < circle.getWidth() / 2) return true;

	return false;
}

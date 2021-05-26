#include "BBE/Circle.h"
#include "BBE/Rectangle.h"
#include "BBE/Vector2.h"
#include "BBE/List.h"
#include "BBE/Math.h"
#include <cstring>

const uint32_t bbe::Circle::AMOUNTOFVERTICES = 32;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::Circle::s_indexBuffer;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::Circle::s_vertexBuffer;

void bbe::Circle::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	s_initVertexBuffer(device, physicalDevice, commandPool, queue);
	s_initIndexBuffer(device, physicalDevice, commandPool, queue);
}

void bbe::Circle::s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	List<uint32_t> indices;
	for (uint32_t i = 1; i < AMOUNTOFVERTICES - 1; i++)
	{
		indices.add(0);
		indices.add(i);
		indices.add(i + 1);
	}

	s_indexBuffer.create(device, physicalDevice, sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void* dataBuf = s_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
	s_indexBuffer.unmap();

	s_indexBuffer.upload(commandPool, queue);
}

void bbe::Circle::s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	List<Vector2> vertices;
	for (std::size_t i = 0; i < AMOUNTOFVERTICES; i++)
	{
		vertices.add(Vector2::createVector2OnUnitCircle((float)i / (float)AMOUNTOFVERTICES * 2 * Math::PI) / 2 + Vector2(0.5f, 0.5f));
	}

	s_vertexBuffer.create(device, physicalDevice, sizeof(Vector2) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	void* dataBuf = s_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(Vector2) * vertices.getLength());
	s_vertexBuffer.unmap();

	s_vertexBuffer.upload(commandPool, queue);
}

void bbe::Circle::s_destroy()
{
	s_indexBuffer.destroy();
	s_vertexBuffer.destroy();
}

bbe::Circle::Circle()
	: m_x(0), m_y(0), m_width(0), m_height(0)	//Todo Rethink: Should this constructor really initialize the member?
{
	//UNTESTED
}

bbe::Circle::Circle(float x, float y, float width, float height)
	: m_x(x), m_y(y), m_width(width), m_height(height)
{
	//UNTESTED
}

bbe::Circle::Circle(const Vector2 & vec, float width, float height)
	: m_x(vec.x), m_y(vec.y), m_width(width), m_height(height)
{
	//UNTESTED
}

bbe::Circle::Circle(float x, float y, const Vector2 & dim)
	: m_x(x), m_y(y), m_width(dim.x), m_height(dim.y)
{
	//UNTESTED
}

bbe::Circle::Circle(const Vector2 & vec, const Vector2 & dim)
	: m_x(vec.x), m_y(vec.y), m_width(dim.x), m_height(dim.y)
{
	//UNTESTED
}

float bbe::Circle::getX() const
{
	//UNTESTED
	return m_x;
}

float bbe::Circle::getY() const
{
	//UNTESTED
	return m_y;
}

bbe::Vector2 bbe::Circle::getPos() const
{
	//UNTESTED
	return bbe::Vector2(m_x, m_y);
}

bbe::Vector2 bbe::Circle::getMiddle() const
{
	return getPos() + getDim() / 2;
}

float bbe::Circle::getWidth() const
{
	//UNTESTED
	return m_width;
}

float bbe::Circle::getHeight() const
{
	//UNTESTED
	return m_height;
}

bbe::Vector2 bbe::Circle::getDim() const
{
	//UNTESTED
	return bbe::Vector2(m_width, m_height);
}

void bbe::Circle::setX(float x)
{
	//UNTESTED
	m_x = x;
}

void bbe::Circle::setY(float y)
{
	//UNTESTED
	m_y = y;
}

void bbe::Circle::setPos(float x, float y)
{
	//UNTESTED
	m_x = x;
	m_y = y;
}

void bbe::Circle::setPos(const Vector2 & vec)
{
	//UNTESTED
	m_x = vec.x;
	m_y = vec.y;
}

void bbe::Circle::setWidth(float width)
{
	//UNTESTED
	m_width = width;
}

void bbe::Circle::setHeight(float height)
{
	//UNTESTED
	m_height = height;
}

void bbe::Circle::setDim(float width, float height)
{
	//UNTESTED
	m_width = width;
	m_height = height;
}

void bbe::Circle::setDim(const Vector2 & vec)
{
	//UNTESTED
	m_width = vec.x;
	m_height = vec.y;
}

void bbe::Circle::set(float x, float y, float width, float height)
{
	//UNTESTED
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;
}

void bbe::Circle::translate(float x, float y)
{
	//UNTESTED
	m_x += x;
	m_y += y;
}

void bbe::Circle::translate(const Vector2 & vec)
{
	//UNTESTED
	translate(vec.x, vec.y);
}

bool bbe::Circle::intersects(const Circle& other) const
{
	if (getWidth() != getHeight() || other.getWidth() != other.getHeight())
	{
		//Only supported for circles, not ovals!
		throw NotImplementedException();
	}

	const float distance = getMiddle().getDistanceTo(other.getMiddle());

	return distance < (getWidth() + other.getWidth()) / 2;
}

bool bbe::Circle::intersects(const Rectangle& other) const
{
	return other.intersects(*this);
}

bool bbe::Circle::resolveIntersection(Circle& other, float massThis, float massOther)
{
	if (this == &other)
	{
		//Can't resolve intersection with itself!
		throw IllegalArgumentException();
	}

	if (!intersects(other))
	{
		//They don't interesect. Nothing to do!
		return false;
	}

	const Vector2 midThis  =       getMiddle();
	const Vector2 midOther = other.getMiddle();

	const Vector2 direction = midThis - midOther;
	const Vector2 normalizedDirection = direction.normalize();

	const float distanceBeforeResolve = direction.getLength();
	const float distanceAfterResolve = (getWidth() + other.getWidth()) / 2;
	const float massSum = massThis + massOther;
	
	const Vector2 moveVectorThis  =  normalizedDirection * (massOther / massSum);
	const Vector2 moveVectorOther = -normalizedDirection * (massThis  / massSum);

	this->setPos(this->getPos() + moveVectorThis);
	other.setPos(other.getPos() + moveVectorOther);

	return true;
}

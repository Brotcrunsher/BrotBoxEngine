// TODO: Make independent of RenderMode
#ifdef BBE_RENDERER_VULKAN
#include "BBE/Circle.h"
#include "BBE/Rectangle.h"
#include "BBE/Vector2.h"
#include "BBE/List.h"
#include "BBE/Math.h"
#include <cstring>

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

bbe::Circle bbe::Circle::offset(const Vector2& off) const
{
	return Circle(
		m_x + off.x,
		m_y + off.y,
		m_width,
		m_height
	);
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

void bbe::Circle::shrinkInPlace(float val)
{
	m_x += val;
	m_y += val;
	m_width -= val * 2;
	m_height -= val * 2;
}

bbe::Circle bbe::Circle::shrinked(float val) const
{
	return Circle(
		m_x + val,
		m_y + val,
		m_width - val * 2,
		m_height - val * 2
	);
}

bbe::Circle bbe::Circle::stretchedSpace(float x, float y) const
{
	return Circle(
		m_x * x,
		m_y * y,
		m_width * x,
		m_height * y
	);
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
#endif

#include "BBE/RectangleRotated.h"
#include "BBE/Rectangle.h"
#include "BBE/Vector2.h"

bbe::RectangleRotated::RectangleRotated()
	: m_x(0), m_y(0), m_width(0), m_height(0), m_rotation(0)
{
}

bbe::RectangleRotated::RectangleRotated(float x, float y, float width, float height, float rotation)
	: m_x(x), m_y(y), m_width(width), m_height(height), m_rotation(rotation)
{
}

bbe::RectangleRotated::RectangleRotated(const Vector2& vec, float width, float height, float rotation)
	: m_x(vec.x), m_y(vec.y), m_width(width), m_height(height), m_rotation(rotation)
{
}

bbe::RectangleRotated::RectangleRotated(float x, float y, const Vector2& dim, float rotation)
	: m_x(x), m_y(y), m_width(dim.x), m_height(dim.y), m_rotation(rotation)
{
}

bbe::RectangleRotated::RectangleRotated(const Vector2& vec, const Vector2& dim, float rotation)
	: m_x(vec.x), m_y(vec.y), m_width(dim.x), m_height(dim.y), m_rotation(rotation)
{
}

bbe::RectangleRotated::RectangleRotated(const Rectangle& rect, float rotation)
	: m_x(rect.getX()), m_y(rect.getY()), m_width(rect.getWidth()), m_height(rect.getHeight()), m_rotation(rotation)
{
}

bool bbe::RectangleRotated::operator==(const RectangleRotated& other) const
{
	return
		m_x        == other.m_x &&
		m_y        == other.m_y &&
		m_width    == other.m_width &&
		m_height   == other.m_height &&
		m_rotation == other.m_rotation;
}

float bbe::RectangleRotated::getX() const
{
	return m_x;
}

float bbe::RectangleRotated::getY() const
{
	return m_y;
}

bbe::Vector2 bbe::RectangleRotated::getPos() const
{
	return { getX(), getY() };
}

float bbe::RectangleRotated::getWidth() const
{
	return m_width;
}

float bbe::RectangleRotated::getHeight() const
{
	return m_height;
}

bbe::Vector2 bbe::RectangleRotated::getDim() const
{
	return { getWidth(), getHeight() };
}

float bbe::RectangleRotated::getRotation() const
{
	return m_rotation;
}

void bbe::RectangleRotated::setX(float x)
{
	m_x = x;
}

void bbe::RectangleRotated::setY(float y)
{
	m_y = y;
}

void bbe::RectangleRotated::setWidth(float width)
{
	m_width = width;
}

void bbe::RectangleRotated::setHeight(float height)
{
	m_height = height;
}

void bbe::RectangleRotated::setRotation(float rotation)
{
	m_rotation = rotation;
}

bbe::Vector2 bbe::RectangleRotated::getCenter() const
{
	return bbe::Vector2(
		m_x + m_width /2.0f, 
		m_y + m_height/2.0f);
}

void bbe::RectangleRotated::getVertices(bbe::List<bbe::Vector2>& outVertices) const
{
	outVertices.clear();

	// Unrotated points
	const bbe::Vector2 p1 = { m_x,           m_y };
	const bbe::Vector2 p2 = { m_x,           m_y + m_height};
	const bbe::Vector2 p3 = { m_x + m_width, m_y + m_height};
	const bbe::Vector2 p4 = { m_x + m_width, m_y};

	const bbe::Vector2 center = getCenter();
	// Rotated points
	outVertices.add(p1.rotate(m_rotation, center));
	outVertices.add(p2.rotate(m_rotation, center));
	outVertices.add(p3.rotate(m_rotation, center));
	outVertices.add(p4.rotate(m_rotation, center));
}

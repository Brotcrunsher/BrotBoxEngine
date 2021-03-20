#include "BBE/RectangleRotated.h"
#include "BBE/Rectangle.h"
#include "BBE/Vector2.h"

bool bbe::RectangleRotated::projectionsIntersect(const ProjectionResult& pr1, const ProjectionResult& pr2)
{
	const float pr1StartLengthSq = pr1.start.getLengthSqSigned();
	const float pr2StartLengthSq = pr2.start.getLengthSqSigned();

	if (pr1StartLengthSq < pr2StartLengthSq)
	{
		const float pr1StopLengthSq = pr1.stop.getLengthSqSigned();
		return pr2StartLengthSq <= pr1StopLengthSq;
	}
	else if(pr2StartLengthSq < pr1StartLengthSq)
	{
		const float pr2StopLengthSq = pr2.stop.getLengthSqSigned();
		return pr1StartLengthSq <= pr2StopLengthSq;
	}
	else
	{
		return true;
	}

	return false;
}

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

bbe::List<bbe::Vector2> bbe::RectangleRotated::getVertices() const
{
	bbe::List<bbe::Vector2> retVal;
	getVertices(retVal);
	return retVal;
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

bbe::List<bbe::Vector2> bbe::RectangleRotated::getNormals() const
{
	auto vertices = getVertices();
	bbe::List<bbe::Vector2> retVal;
	retVal.resizeCapacityAndLength(vertices.getLength());

	for (size_t i = 0; i < vertices.getLength() - 1; i++)
	{
		retVal[i] = vertices[i + 1] - vertices[i];
	}

	retVal[retVal.getLength() - 1] = vertices[0] - vertices[vertices.getLength() - 1];

	for (size_t i = 0; i < retVal.getLength(); i++)
	{
		retVal[i] = retVal[i].rotate90Clockwise();
		retVal[i] = retVal[i].normalize();
	}

	return retVal;
}

bbe::RectangleRotated::ProjectionResult bbe::RectangleRotated::project(const bbe::Vector2& projection) const
{
	auto vertices = getVertices();
	auto projections = bbe::Math::project(vertices, projection);

	float minLengthSq = projections[0].getLengthSqSigned();
	float maxLengthSq = minLengthSq;
	ProjectionResult retVal{ projections[0], projections[0] };

	for (size_t i = 1; i < projections.getLength(); i++)
	{
		const float lengthSq = projections[i].getLengthSqSigned();
		if (lengthSq > maxLengthSq)
		{
			maxLengthSq = lengthSq;
			retVal.stop = projections[i];
		}
		if (lengthSq < minLengthSq)
		{
			minLengthSq = lengthSq;
			retVal.start = projections[i];
		}
	}

	return retVal;
}

bool bbe::RectangleRotated::intersects(const RectangleRotated& other) const
{
	if (*this == other) return true;

	auto normalsThis  =       getNormals();
	auto normalsOther = other.getNormals();

	for (const bbe::Vector2 &normal : normalsThis)
	{
		auto p1 =       project(normal);
		auto p2 = other.project(normal);
		if (!projectionsIntersect(p1, p2)) return false;
	}

	for (const bbe::Vector2& normal : normalsOther)
	{
		auto p1 = project(normal);
		auto p2 = other.project(normal);
		if (!projectionsIntersect(p1, p2)) return false;
	}

	return true;
}

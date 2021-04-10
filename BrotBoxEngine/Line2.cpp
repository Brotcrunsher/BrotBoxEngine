#include "BBE/Line2.h"
#include "BBE/Math.h"

bbe::Line2::Relationship bbe::Line2::getRelationship(const Line2& other) const
{
	// See: https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
	const bbe::Vector2 r = getDirection();
	const bbe::Vector2 s = other.getDirection();

	const float rxs = r.cross(s);

	const bbe::Vector2 qmp = other.m_start - m_start;
	const float qmpxr = qmp.cross(r);

	if (rxs == 0 && qmpxr == 0) bbe::Line2::Relationship::COLLINEAR;

	if (rxs == 0 && qmpxr != 0) bbe::Line2::Relationship::PARALLEL;

	const float u = qmpxr / rxs;
	const float t = qmp.cross(s) / rxs;

	if (u >= 0.f && u <= 1.f && t >= 0.f && t <= 1.f) return bbe::Line2::Relationship::INTERSECTING;
	else return bbe::Line2::Relationship::NONE_INTERSECTING;
}

bbe::Vector2 bbe::Line2::getIntersection(const Line2& other) const
{
	// See: https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
	const bbe::Vector2 r = getDirection();
	const bbe::Vector2 s = other.getDirection();

	const float rxs = r.cross(s);

	const bbe::Vector2 qmp = other.m_start - m_start;
	const float qmpxr = qmp.cross(r);

	if (rxs == 0 && qmpxr == 0) return m_start; // Collinear.

	if (rxs == 0 && qmpxr != 0) return { bbe::Math::INFINITY_POSITIVE , bbe::Math::INFINITY_POSITIVE }; // Parallel

	const float u = qmpxr / rxs;

	return other.m_start + s * u;
}

float bbe::Line2::getAngle() const
{
	return getDirection().getAngle();
}

bbe::Vector2 bbe::Line2::getDirection() const
{
	return m_stop - m_start;
}

bbe::Line2::Line2()
{
}

bbe::Line2::Line2(const bbe::Vector2& start, const bbe::Vector2& stop)
	: m_start(start), m_stop(stop)
{
}

bbe::Line2::Line2(float startX, float startY, const bbe::Vector2& stop)
	: m_start({ startX, startY }), m_stop(stop)
{
}

bbe::Line2::Line2(const bbe::Vector2& start, float stopX, float stopY)
	: m_start(start), m_stop({ stopX, stopY })
{
}

bbe::Line2::Line2(float startX, float startY, float stopX, float stopY)
	: m_start({ startX, startY }), m_stop({ stopX, stopY })
{
}

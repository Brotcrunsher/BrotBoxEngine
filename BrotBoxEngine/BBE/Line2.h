#pragma once

#include "../BBE/Vector2.h"

namespace bbe
{
	class Line2
	{
	public:
		enum class Relationship
		{
			COLLINEAR,
			PARALLEL,
			INTERSECTING,
			NONE_INTERSECTING,
		};

	public:
		bbe::Vector2 m_start;
		bbe::Vector2 m_stop;

	public:
		Line2();
		Line2(const bbe::Vector2& start,  const bbe::Vector2& stop);
		Line2(float startX, float startY, const bbe::Vector2& stop);
		Line2(const bbe::Vector2& start,  float stopX, float stopY);
		Line2(float startX, float startY, float stopX, float stopY);

		Relationship getRelationship(const Line2& other) const;
		bbe::Vector2 getIntersection(const Line2& other) const;
		float getAngle() const;
		bbe::Vector2 getDirection() const;

	};
}

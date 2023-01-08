#pragma once

#include "../BBE/Vector2.h"

namespace bbe
{
	template<typename Vec>
	class Line2_t
	{
	public:
		enum class Relationship
		{
			COLLINEAR,
			PARALLEL,
			INTERSECTING,
			NONE_INTERSECTING,
		};
		using SubType = typename Vec::SubType;

	public:
		Vec m_start;
		Vec m_stop;

	public:
		Line2_t()
		{
		}
		Line2_t(const Vec& start,  const Vec& stop)
			: m_start(start), m_stop(stop)
		{
		}
		Line2_t(SubType startX, SubType startY, const Vec& stop)
			: m_start({ startX, startY }), m_stop(stop)
		{
		}
		Line2_t(const Vec& start,  SubType stopX, SubType stopY)
			: m_start(start), m_stop({ stopX, stopY })
		{
		}
		Line2_t(SubType startX, SubType startY, SubType stopX, SubType stopY)
			: m_start({ startX, startY }), m_stop({ stopX, stopY })
		{
		}

		Relationship getRelationship(const Line2_t& other) const
		{
			// See: https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
			const Vec r = getDirection();
			const Vec s = other.getDirection();

			const SubType rxs = r.cross(s);

			const Vec qmp = other.m_start - m_start;
			const SubType qmpxr = qmp.cross(r);

			if (rxs == 0 && qmpxr == 0) bbe::Line2_t<Vec>::Relationship::COLLINEAR;

			if (rxs == 0 && qmpxr != 0) bbe::Line2_t<Vec>::Relationship::PARALLEL;

			const SubType u = qmpxr / rxs;
			const SubType t = qmp.cross(s) / rxs;

			if (u >= 0.f && u <= 1.f && t >= 0.f && t <= 1.f) return bbe::Line2_t<Vec>::Relationship::INTERSECTING;
			else return bbe::Line2_t<Vec>::Relationship::NONE_INTERSECTING;
		}
		Vec getIntersection(const Line2_t<Vec>& other) const
		{
			// See: https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
			const Vec r = getDirection();
			const Vec s = other.getDirection();

			const SubType rxs = r.cross(s);

			const Vec qmp = other.m_start - m_start;
			const SubType qmpxr = qmp.cross(r);

			if (rxs == 0 && qmpxr == 0) return m_start; // Collinear.

			if (rxs == 0 && qmpxr != 0) return { bbe::Math::INFINITY_POSITIVE , bbe::Math::INFINITY_POSITIVE }; // Parallel

			const SubType u = qmpxr / rxs;

			return other.m_start + s * u;
		}
		SubType getAngle() const
		{
			return getDirection().getAngle();
		}
		Vec getDirection() const
		{
			return m_stop - m_start;
		}

		bbe::Line2_t<Vec> operator+(const Vec& translation) const
		{
			return bbe::Line2_t<Vec>(m_start + translation, m_stop + translation);
		}
	};

	using Line2  = Line2_t<bbe::Vector2>;
	using Line2i = Line2_t<bbe::Vector2i>;
}

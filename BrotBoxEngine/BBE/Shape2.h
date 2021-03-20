#pragma once

#include "../BBE/Vector2.h"

namespace bbe
{
	class Shape2
	{
	public:
		struct ProjectionResult
		{
			bbe::Vector2 start;
			bbe::Vector2 stop;
		};
	protected:
		static bool projectionsIntersect(const ProjectionResult& pr1, const ProjectionResult& pr2);
	public:
		virtual bbe::Vector2 getCenter() const = 0;

		virtual bbe::List<bbe::Vector2> getVertices() const;
		virtual void getVertices(bbe::List<bbe::Vector2>& outVertices) const = 0;

		virtual bbe::List<bbe::Vector2> getNormals() const;

		virtual ProjectionResult project(const bbe::Vector2& projection) const;

		virtual bool intersects(const Shape2& other) const;
	};
}
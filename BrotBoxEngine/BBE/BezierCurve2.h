#pragma once

#include "../BBE/Vector2.h"
#include "../BBE/List.h"

namespace bbe
{
	class BezierCurve2
	{
	private:
		Vector2 startPoint;
		Vector2 endPoint;
		bbe::List<Vector2> controlPoints;

	public:
		BezierCurve2();
		BezierCurve2(const Vector2& startPoint, const Vector2& endPoint, const bbe::List<Vector2> &controlPoints);
		BezierCurve2(const Vector2& startPoint, const Vector2& endPoint);
		BezierCurve2(const Vector2& startPoint, const Vector2& endPoint, const Vector2& control);
		BezierCurve2(const Vector2& startPoint, const Vector2& endPoint, const Vector2& control1, const Vector2& control2);

		Vector2 evaluate(float t) const;
		void addControlPoint(const Vector2& point);

		bbe::List<Vector2> getIntermediatePoints(float t, unsigned stage) const;

		bbe::List<Vector2>& getControlPoints();
		Vector2& getStartPoint();
		Vector2& getEndPoint();

		const bbe::List<Vector2>& getControlPoints() const;
		const Vector2& getStartPoint() const;
		const Vector2& getEndPoint() const;
	};
}
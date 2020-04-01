#include "BBE/BezierCurve2.h"
#include "BBE/Math.h"

bbe::BezierCurve2::BezierCurve2()
{
	// Do nothing
}

bbe::BezierCurve2::BezierCurve2(const Vector2& startPoint, const Vector2& endPoint, const bbe::List<Vector2>& controlPoints)
	: startPoint(startPoint),
	endPoint(endPoint),
	controlPoints(controlPoints)
{
}

bbe::BezierCurve2::BezierCurve2(const Vector2& startPoint, const Vector2& endPoint)
	: startPoint(startPoint),
	endPoint(endPoint)
{
}

bbe::BezierCurve2::BezierCurve2(const Vector2& startPoint, const Vector2& endPoint, const Vector2& control)
	: startPoint(startPoint),
	endPoint(endPoint),
	controlPoints({ control })
{
}

bbe::BezierCurve2::BezierCurve2(const Vector2& startPoint, const Vector2& endPoint, const Vector2& control1, const Vector2& control2)
	: startPoint(startPoint),
	endPoint(endPoint),
	controlPoints({ control1, control2 })
{
}

bbe::Vector2 bbe::BezierCurve2::evaluate(float t) const
{
	return bbe::Math::interpolateBezier(startPoint, endPoint, t, controlPoints);
}

void bbe::BezierCurve2::addControlPoint(const Vector2& point)
{
	controlPoints.add(point);
}

bbe::List<bbe::Vector2> bbe::BezierCurve2::getIntermediatePoints(float t, unsigned stage) const
{
	if (stage == 0)
	{
		bbe::List<bbe::Vector2> retVal(controlPoints.getLength() + 2);
		retVal.add(startPoint);
		retVal.addArray(controlPoints.getRaw(), controlPoints.getLength());
		retVal.add(endPoint);
		return retVal;
	}
	else
	{
		bbe::List<bbe::Vector2> previousStagePoints = getIntermediatePoints(t, stage - 1);
		bbe::List<bbe::Vector2> retVal(previousStagePoints.getLength() - 1);

		for (size_t i = 0; i < previousStagePoints.getLength() - 1; i++)
		{
			retVal.add(bbe::Math::interpolateLinear(previousStagePoints[i], previousStagePoints[i + 1], t));
		}

		return retVal;
	}


}

bbe::List<bbe::Vector2>& bbe::BezierCurve2::getControlPoints()
{
	return controlPoints;
}

bbe::Vector2& bbe::BezierCurve2::getStartPoint()
{
	return startPoint;
}

bbe::Vector2& bbe::BezierCurve2::getEndPoint()
{
	return endPoint;
}

const bbe::List<bbe::Vector2>& bbe::BezierCurve2::getControlPoints() const
{
	return controlPoints;
}

const bbe::Vector2& bbe::BezierCurve2::getStartPoint() const
{
	return startPoint;
}

const bbe::Vector2& bbe::BezierCurve2::getEndPoint() const
{
	return endPoint;
}

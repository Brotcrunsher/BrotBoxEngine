#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	bbe::Array<bbe::Vector2, 3> bezierPoints = { {100, 500}, {500, 100}, {100, 100} };

	bbe::Array<bbe::Vector2, 4> hermitePoints = { {600, 500}, {1100, 100}, {600, 400}, {1100, 200} };

	bbe::List<bbe::Vector2*> allPoints;

	bbe::Vector2* getClosest(const bbe::Vector2& pos)
	{
		bbe::Vector2* closest = nullptr;
		float minDist = std::numeric_limits<float>::infinity();

		for (bbe::Vector2* p : allPoints)
		{
			const float dist = pos.getDistanceTo(*p);
			if (dist < minDist)
			{
				minDist = dist;
				closest = p;
			}
		}

		return closest;
	}

	virtual void onStart() override
	{
		for (bbe::Vector2& p : bezierPoints)
		{
			allPoints.add(&p);
		}
		for (bbe::Vector2& p : hermitePoints)
		{
			allPoints.add(&p);
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		if (isMouseDown(bbe::MouseButton::LEFT))
		{
			bbe::Vector2 mousePosition = getMouse();
			bbe::Vector2* selected = getClosest(mousePosition);
			if (selected != nullptr)
			{
				*selected = mousePosition;
			}
		}

		if (isKeyDown(bbe::Key::SPACE))
		{
			for (bbe::Vector2 *p : allPoints)
			{
				*p = bbe::Vector2();
			}
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		constexpr float circleSize = 10;
		for (bbe::Vector2* p : allPoints)
		{
			brush.fillCircle(p->x - circleSize / 2, p->y - circleSize / 2, circleSize, circleSize);
		}

		bbe::Vector2 previousPoint = bezierPoints[0];
		for (float t = 0; t <= 1; t += 0.01)
		{
			const bbe::Vector2 currentPoint = bbe::Math::interpolateBezier(bezierPoints[0], bezierPoints[1], t, bezierPoints[2]);

			brush.fillLine(previousPoint, currentPoint);

			previousPoint = currentPoint;
		}

		previousPoint = hermitePoints[0];
		for (float t = 0; t <= 1; t += 0.01)
		{
			const bbe::Vector2 currentPoint = bbe::Math::interpolateHermite(hermitePoints[0], hermitePoints[1], t, (hermitePoints[2] - hermitePoints[0]) * 10, (hermitePoints[1] - hermitePoints[3]) * 10);

			brush.fillLine(previousPoint, currentPoint);

			previousPoint = currentPoint;
		}

		const float interpVal = (bbe::Math::sin(getTimeSinceStartSeconds()) + 1) * 0.5f;
		const bbe::Vector2 a = bbe::Math::interpolateLinear(bezierPoints[0], bezierPoints[2], interpVal);
		const bbe::Vector2 b = bbe::Math::interpolateLinear(bezierPoints[2], bezierPoints[1], interpVal);
		const bbe::Vector2 c = bbe::Math::interpolateLinear(a, b, interpVal);

		brush.fillLine(bezierPoints[0], bezierPoints[2]);
		brush.fillLine(bezierPoints[1], bezierPoints[2]);
		brush.fillLine(a, b);

		brush.fillCircle(a.x - circleSize / 2, a.y - circleSize / 2, circleSize, circleSize);
		brush.fillCircle(b.x - circleSize / 2, b.y - circleSize / 2, circleSize, circleSize);
		brush.fillCircle(c.x - circleSize / 2, c.y - circleSize / 2, circleSize, circleSize);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Curves!");
	delete mg;
}

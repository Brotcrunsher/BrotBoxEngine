#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <array>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	bbe::List<bbe::Vector2> points;

	virtual void onStart() override
	{
		bbe::Random rand;
		for (int i = 0; i < 10; i++)
		{
			points.add(rand.randomVector2(WINDOW_WIDTH, WINDOW_HEIGHT));
		}
	}

	virtual void update(float timeSinceLastFrame) override
	{
		const bbe::Vector2 mousePos = getMouse();
		if (isMouseDown(bbe::MouseButton::LEFT))
		{
			bbe::Vector2* moveVec = bbe::Math::getClosest(mousePos, points);
			if (moveVec)
			{
				*moveVec = mousePos;
			}
		}

		if (isKeyPressed(bbe::Key::SPACE))
		{
			points.add(mousePos);
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		brush.setColorRGB(1, 1, 1);
		auto hull = bbe::Math::getConvexHull(points);
		brush.fillLineStrip(hull, true);

		brush.setColorRGB(1, 0, 1);
		for (const bbe::Vector2& p : points)
		{
			brush.fillCircle(p - bbe::Vector2{ 5, 5 }, { 11, 11 });
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "ConvexHull!");
	delete mg;
}

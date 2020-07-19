#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <set>
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_fixture.h"

constexpr float WINDOW_WIDTH  = 1280.f;
constexpr float WINDOW_HEIGHT =  720.f;

class MyGame : public bbe::Game
{
	bbe::List<bbe::PhysRectangle> bodies;
	bbe::List<bbe::PhysRectangle> statics;

	bbe::PhysRectangle createStatic(const bbe::Rectangle& rect, float angle)
	{
		bbe::PhysRectangle retVal(this, rect, angle);
		retVal.freeze();
		return retVal;
	}

	virtual void onStart() override
	{
		statics.add(createStatic(bbe::Rectangle(0, 700, WINDOW_WIDTH, 20), 0));
		statics.add(createStatic(bbe::Rectangle(0, 0, 20, WINDOW_HEIGHT), 0));
		statics.add(createStatic(bbe::Rectangle(WINDOW_WIDTH - 20, 0, 20, WINDOW_HEIGHT), 0));

		for (int i = 0; i < 1500; i++)
		{
			bodies.add(bbe::PhysRectangle(this, bbe::Rectangle(WINDOW_WIDTH / 2, 670 - i * 30, 20, 20), 0));
		}

	}
	virtual void update(float timeSinceLastFrame) override
	{
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		brush.setColorRGB(1, 1, 1);
		for (const bbe::PhysRectangle& r : statics)
		{
			brush.fillRect(r);
		}
		brush.setColorRGB(0, 0, 1);
		brush.setOutlineRGB(1, 0, 0);
		brush.setOutlineWidth(1);
		for (const bbe::PhysRectangle& r : bodies)
		{
			brush.fillRect(r);
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "2D Physics!");

    return 0;
}


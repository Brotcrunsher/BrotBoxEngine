#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <set>
#include "box2d/b2_revolute_joint.h"
#include "box2d/b2_world.h"

constexpr float WINDOW_WIDTH  = 1280.f;
constexpr float WINDOW_HEIGHT =  720.f;

class MyGame : public bbe::Game
{
	bbe::List<bbe::PhysRectangle> bodiesRect;
	bbe::List<bbe::PhysCircle> bodiesCircle;
	bbe::List<bbe::PhysRectangle> statics;

	bbe::PhysRectangle createStatic(const bbe::Rectangle& rect, float angle)
	{
		bbe::PhysRectangle retVal(this, rect, angle);
		retVal.freeze();
		return retVal;
	}

	virtual void onStart() override
	{
		//Dominos
		statics.add(createStatic(bbe::Rectangle(100, 100, 540, 20), 0));
		statics.add(createStatic(bbe::Rectangle(49, 48, 50, 20), 0.5));
		statics.add(createStatic(bbe::Rectangle(80, 60, 20, 60), 0));
		for (int i = 0; i < 12; i++)
		{
			bodiesRect.add(bbe::PhysRectangle(this, bbe::Rectangle(120 + i * 40, 50, 10, 50)));
		}
		bodiesCircle.add(bbe::PhysCircle(this, 50, 0, 10, 0));

		//Rope swing
		bodiesCircle.add(bbe::PhysCircle(this, 618, 90, 10, 0));
		statics.add(createStatic(bbe::Rectangle(800, 0, 20, 20), 0));
		bodiesCircle.last().addJointRope(statics.last(), 250);

		//Upward swinging doors
		statics.add(createStatic(bbe::Rectangle(620, 220, 20, 20), 0));
		statics.add(createStatic(bbe::Rectangle(91, 287, 540, 20), -0.25f));
		bodiesCircle.add(bbe::PhysCircle(this, 610, 180, 20, 0));
		for (int i = 0; i < 7; i++)
		{
			float x = 100 + i * 60;
			float heightOfWhiteBlock = 160 - i * 15;
			statics.add(createStatic(bbe::Rectangle(x, 120, 20, heightOfWhiteBlock), 0));
			bodiesRect.add(bbe::PhysRectangle(this, bbe::Rectangle(x + 5, 117 + heightOfWhiteBlock, 10, 65)));
			statics.last().addJointRevolute(bodiesRect.last(), { x + 10, 120 + heightOfWhiteBlock });
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
		brush.setColorRGB(0, 1, 0);
		brush.fillLine(bodiesCircle[1].getCenterOfMass(), statics[3].getCenterOfMass());

		brush.setColorRGB(0, 0, 1);
		brush.setOutlineRGB(1, 0, 0);
		brush.setOutlineWidth(1);
		for (const bbe::PhysRectangle& r : bodiesRect)
		{
			brush.fillRect(r);
		}
		for (const bbe::PhysCircle& c : bodiesCircle)
		{
			brush.fillCircle(c);
		}

		brush.setColorRGB(1, 1, 1);
		brush.setOutlineWidth(0);
		for (const bbe::PhysRectangle& r : statics)
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


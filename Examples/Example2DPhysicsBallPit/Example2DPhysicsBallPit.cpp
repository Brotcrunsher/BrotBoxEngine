#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <set>

constexpr float WINDOW_WIDTH  = 1280.f;
constexpr float WINDOW_HEIGHT =  720.f;

class MyGame : public bbe::Game
{
	struct CircleWithExtraData
	{
		float hue = 0;
		float value = 0;
		bbe::PhysCircle circle;
	};
	bbe::List<CircleWithExtraData> circles;
	bbe::Random rand;

	virtual void onStart() override
	{
		constexpr float blockerWidth = 20;
		bbe::PhysRectangle topBlocker    = bbe::PhysRectangle(this, -blockerWidth, -blockerWidth, WINDOW_WIDTH + blockerWidth * 2, blockerWidth);
		bbe::PhysRectangle bottomBlocker = bbe::PhysRectangle(this, -blockerWidth, WINDOW_HEIGHT, WINDOW_WIDTH + blockerWidth * 2, blockerWidth);
		bbe::PhysRectangle leftBlocker   = bbe::PhysRectangle(this, -blockerWidth, -blockerWidth, blockerWidth, WINDOW_HEIGHT + blockerWidth * 2);
		bbe::PhysRectangle rightBlocker  = bbe::PhysRectangle(this, WINDOW_WIDTH , -blockerWidth, blockerWidth, WINDOW_HEIGHT + blockerWidth * 2);
		topBlocker   .freeze();
		bottomBlocker.freeze();
		leftBlocker  .freeze();
		rightBlocker .freeze();

		getPhysWorld()->setGravity({ 0, 10 });


		constexpr float playerRadius = 40;
		circles.add({0, 0, bbe::PhysCircle{
			this,
			rand.randomFloat(WINDOW_WIDTH - playerRadius * 2),
			rand.randomFloat(WINDOW_HEIGHT - playerRadius * 2),
			playerRadius,
		}});
		circles.last().circle.setFriction(0.1);
		circles.last().circle.setDensity(1000);
		for (uint32_t i = 0; i < 1024 * 2; i++)
		{
			constexpr float radius = 8;
			bbe::PhysCircle c = bbe::PhysCircle{
				this,
				rand.randomFloat(WINDOW_WIDTH - radius * 2),
				rand.randomFloat(WINDOW_HEIGHT - radius * 2),
				radius,
			};
			c.setFriction(0.1);
			c.setDensity(0.1);
			c.addSpeed(rand.randomVector2InUnitSphere() * 100);
			circles.add({ 0, 0, c });
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << "FPS: " << (1.f / timeSinceLastFrame) << std::endl;

		if (isKeyDown(bbe::Key::W))
		{
			circles[0].circle.addSpeed(bbe::Vector2(0, -4000) * timeSinceLastFrame);
		}
		if (isKeyDown(bbe::Key::S))
		{
			circles[0].circle.addSpeed(bbe::Vector2(0, 4000) * timeSinceLastFrame);
		}
		if (isKeyDown(bbe::Key::A))
		{
			circles[0].circle.addSpeed(bbe::Vector2(-4000, 0) * timeSinceLastFrame);
		}
		if (isKeyDown(bbe::Key::D))
		{
			circles[0].circle.addSpeed(bbe::Vector2(4000, 0) * timeSinceLastFrame);
		}

		if (isKeyPressed(bbe::Key::SPACE))
		{
			for (size_t i = 0; i < circles.getLength(); i++)
			{
				circles[i].hue = (circles[i].circle.getX() / WINDOW_WIDTH) * 360;
				circles[i].value = (circles[i].circle.getY() / WINDOW_HEIGHT);
			}
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		brush.setColorRGB(1, 1, 1);
		brush.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		for (const CircleWithExtraData& c : circles)
		{
			brush.setColorHSV(c.hue, 1, c.value);
			brush.fillCircle(c.circle);
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
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}


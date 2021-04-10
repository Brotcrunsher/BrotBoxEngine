#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	bbe::List<float> vals;
	virtual void onStart() override
	{
		for (int i = 0; i < 11; i++)
		{
			vals.add(i);
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		for (size_t i = 0; i < vals.getLength(); i++)
		{
			vals[i] += timeSinceLastFrame * 2;
			if (vals[i] > vals.getLength()) vals[i] -= vals.getLength();
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}

	void draw(bbe::PrimitiveBrush2D& brush, bool inverted, const bbe::Vector2 &offset)
	{
		for (size_t i = 0; i < vals.getLength(); i++)
		{
			float val = vals[i];
			if (inverted) val = vals.getLength() - val;
			brush.setColorRGB(1, 1, 1, 1 - val / 5);
			bbe::Vector2 start = { bbe::Math::max(val * 40 - 20 + offset.x, offset.x), offset.y };
			bbe::Vector2 end = { val * 40 + offset.x, offset.y };
			if (inverted)
			{
				bbe::Vector2 temp = start;
				start = end;
				end = temp;
			}
			for (double rot = 0; rot < bbe::Math::TAU; rot += bbe::Math::TAU / 12.)
			{
				brush.fillArrow(start.rotate(rot, offset), end.rotate(rot, offset), 3, 10, 15);
			}
		}
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		bbe::Vector2 offset = { 300.f, WINDOW_HEIGHT / 2 };
		draw(brush, false, offset);
		offset = { WINDOW_WIDTH - 300.f, WINDOW_HEIGHT / 2 };
		draw(brush, true, offset);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Divergence!");
	delete mg;
}

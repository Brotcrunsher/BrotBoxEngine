#include "BBE/BrotBoxEngine.h"
#include <iostream>

class MyGame : public bbe::Game
{
	bbe::Grid<bool> booleanGrid;

	virtual void onStart() override
	{
		booleanGrid =
		{
			{true , false, true , false, false},
			{false, false, false, true , false},
			{true , false, true , false, false},
			{true , false, false, false, true },
		};
	}
	virtual void update(float timeSinceLastFrame) override
	{
		bbe::Vector2 mouse = getMouse();
		bbe::Vector2i mousei = bbe::Vector2i{ (int32_t)mouse.x, (int32_t)mouse.y };
		bbe::Vector2i gridIndex = (mousei - bbe::Vector2i(10, 10)) / 10;

		if (   gridIndex.x >= 0 && gridIndex.x < booleanGrid.getWidth()
			&& gridIndex.y >= 0 && gridIndex.y < booleanGrid.getHeight())
		{
			if (isMouseDown(bbe::MouseButton::LEFT))
			{
				booleanGrid[gridIndex] = true;
			}
			if (isMouseDown(bbe::MouseButton::RIGHT))
			{
				booleanGrid[gridIndex] = false;
			}
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		for (size_t i = 0; i < booleanGrid.getWidth(); i++)
		{
			for (size_t k = 0; k < booleanGrid.getHeight(); k++)
			{
				if (booleanGrid[i][k])
				{
					brush.setColorRGB(0, 1, 0);
				}
				else
				{
					brush.setColorRGB(1, 0, 0);
				}
				brush.fillRect(10 + 10 * i, 10 + 10 * k, 10, 10);
			}
		}

		brush.setColorRGB(1, 1, 1);

		bbe::Rectanglei biggestRect = booleanGrid.getBiggestRect(true);
		brush.sketchRect(10 + biggestRect.getX() * 10, 10 + biggestRect.getY() * 10, biggestRect.getWidth() * 10, biggestRect.getHeight() * 10);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "Grid!");

    return 0;
}


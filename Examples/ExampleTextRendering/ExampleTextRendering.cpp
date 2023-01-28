#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	virtual void onStart() override
	{
	}
	virtual void update(float timeSinceLastFrame) override
	{
	}
	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		brush.setColorRGB(1, 0, 0);
		brush.fillRect(10, 10, 200, 50);
		brush.setColorRGB(0, 0.5, 0, 0.5);
		brush.fillRect(20, 40, 200, 50);
		brush.setColorRGB(1, 1, 1);
		brush.fillText(20, 20, "This is the first string that is\never fully drawn with BBE! ygpq", 20);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame* mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "ExampleTextRendering!");
	delete mg;
}

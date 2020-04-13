#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	bbe::Font myFont;

	virtual void onStart() override
	{
		myFont.load("arial.ttf", 20);
	}
	virtual void update(float timeSinceLastFrame) override
	{
	}
	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		brush.setColorRGB(1, 1, 1);
		brush.fillText(20, 20, "This is the first string that is ever fully drawn with BBE! ygpq", myFont);
		brush.setColorRGB(0.5, 0.5, 0.5);
		brush.fillRect(10, 10, 200, 50);
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

#include "BBE/BrotBoxEngine.h"
#include <iostream>

class MyGame : public bbe::Game
{
	virtual void onStart() override
	{
	}
	virtual void update(float timeSinceLastFrame) override
	{
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "Template!");

    return 0;
}


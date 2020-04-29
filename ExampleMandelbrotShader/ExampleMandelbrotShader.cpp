#include "BBE/BrotBoxEngine.h"
#include <iostream>

class MyGame : public bbe::Game
{
public:
	bbe::FragmentShader mandelbrotShader;

	int max_iteration = 100;

	double middleX = -0.75;
	double middleY = 0;

	double rangeX = 3.5;
	double rangeY = 2;

	virtual void onStart() override
	{
		mandelbrotShader.load("fragMandelbrot.spv");
	}

	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << "FPS: " << (1 / timeSinceLastFrame) << std::endl;

		if (isKeyDown(bbe::Key::DOWN))
		{
			int newMaxIteration = max_iteration * 0.9;
			if (newMaxIteration == max_iteration)
				max_iteration--;
			else
				max_iteration = newMaxIteration;
		}
		if (isKeyDown(bbe::Key::UP))
		{
			int newMaxIteration = max_iteration / 0.9;
			if (newMaxIteration == max_iteration)
				max_iteration++;
			else
				max_iteration = newMaxIteration;
		}

		float sprintMultiplier = 1;
		if (isKeyDown(bbe::Key::LEFT_SHIFT))
		{
			sprintMultiplier = 6;
		}

		if (isKeyDown(bbe::Key::W))
		{
			middleY -= 0.1 * sprintMultiplier * timeSinceLastFrame * rangeY;
		}
		if (isKeyDown(bbe::Key::S))
		{
			middleY += 0.1 * sprintMultiplier * timeSinceLastFrame * rangeY;
		}
		if (isKeyDown(bbe::Key::A))
		{
			middleX -= 0.1 * sprintMultiplier * timeSinceLastFrame * rangeX;
		}
		if (isKeyDown(bbe::Key::D))
		{
			middleX += 0.1 * sprintMultiplier * timeSinceLastFrame * rangeX;
		}

		if (getMouseScrollY() > 0)
		{
			rangeX *= 0.8;
			rangeY *= 0.8;
		}
		else if(getMouseScrollY() < 0)
		{
			rangeX /= 0.8;
			rangeY /= 0.8;
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		mandelbrotShader.setPushConstant(brush, 128, 8, &middleX);
		mandelbrotShader.setPushConstant(brush, 136, 8, &middleY);
		mandelbrotShader.setPushConstant(brush, 144, 8, &rangeX);
		mandelbrotShader.setPushConstant(brush, 152, 8, &rangeY);
		mandelbrotShader.setPushConstant(brush, 160, 4, &max_iteration);
		brush.fillRect(0, 0, getWindowWidth(), getWindowHeight(), 0, &mandelbrotShader);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	constexpr int WINDOW_WIDTH = 1280;
	constexpr int WINDOW_HEIGHT = 720;
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "MandelbrotShader");
	delete mg;
}

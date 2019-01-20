#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	int max_iteration = 10;

	double middleX = -0.75;
	double middleY = 0;

	double rangeX = 3.5;
	double rangeY = 2;

	virtual void onStart() override
	{
	}
	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << (1 / timeSinceLastFrame) << std::endl;

		if (isKeyDown(bbe::Key::DOWN))
		{
			max_iteration--;
		}
		if (isKeyDown(bbe::Key::UP))
		{
			max_iteration++;
		}

		if (getMouseScrollY() > 0)
		{
			rangeX *= 0.9;
			rangeY *= 0.9;
		}
		else if(getMouseScrollY() < 0)
		{
			rangeX /= 0.9;
			rangeY /= 0.9;
		}

	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		for (int x = 0; x < WINDOW_WIDTH; x++)
		{
			for (int y = 0; y < WINDOW_HEIGHT; y++)
			{
				double x0 = (double)x / (double)WINDOW_WIDTH;
				double y0 = (double)y / (double)WINDOW_HEIGHT;

				x0 = x0 * rangeX + middleX - rangeX / 2;
				y0 = y0 * rangeY + middleY - rangeY / 2;

				double real      = 0;
				double imaginary = 0;

				int iteration = 0;
				while (real * real + imaginary * imaginary <= 4 && iteration < max_iteration)
				{
					double temp = real * real - imaginary * imaginary + x0;
					imaginary = 2 * real * imaginary + y0;
					real = temp;
					iteration++;
				}

				double colorVal = (double)iteration / max_iteration;

				brush.setColorHSV(colorVal * 360, 1, 1 - colorVal);

				brush.fillRect(x, y, 1, 1);
			}
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Mandelbrot");
	delete mg;
}

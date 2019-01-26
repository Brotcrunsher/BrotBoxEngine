#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <thread>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
public:
	int max_iteration = 100;

	double middleX = -0.75;
	double middleY = 0;

	double rangeX = 3.5;
	double rangeY = 2;

	double picData[WINDOW_WIDTH][WINDOW_HEIGHT];

	constexpr static int numThreads = 16;

	void work(int id)
	{
		for (int x = id; x < WINDOW_WIDTH; x += numThreads)
		{
			for (int y = 0; y < WINDOW_HEIGHT; y++)
			{
				double x0 = (double)x / (double)WINDOW_WIDTH;
				double y0 = (double)y / (double)WINDOW_HEIGHT;

				x0 = x0 * rangeX + middleX - rangeX / 2;
				y0 = y0 * rangeY + middleY - rangeY / 2;

				double real = 0;
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

				picData[x][y] = colorVal;
			}
		}
	}

	virtual void onStart() override
	{
	}
	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << (1 / timeSinceLastFrame) << std::endl;

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

		if (isKeyDown(bbe::Key::W))
		{
			middleY -= 0.1 * timeSinceLastFrame * rangeY;
		}
		if (isKeyDown(bbe::Key::S))
		{
			middleY += 0.1 * timeSinceLastFrame * rangeY;
		}
		if (isKeyDown(bbe::Key::A))
		{
			middleX -= 0.1 * timeSinceLastFrame * rangeX;
		}
		if (isKeyDown(bbe::Key::D))
		{
			middleX += 0.1 * timeSinceLastFrame * rangeX;
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


		bbe::List<std::thread> threads;

		for (int i = 0; i < numThreads; i++)
		{
			threads.add(std::thread(&MyGame::work, this, i));
		}

		for (int i = 0; i < numThreads; i++)
		{
			threads[i].join();
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
				brush.setColorHSV(-picData[x][y] * 360 + 240, 1, 1 - picData[x][y]);

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

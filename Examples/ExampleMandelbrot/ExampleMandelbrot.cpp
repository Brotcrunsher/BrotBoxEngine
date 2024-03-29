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

	constexpr static int numThreads =
#ifdef __EMSCRIPTEN__
		1;
#else
		12;
#endif

	std::condition_variable workerConditional;
	std::condition_variable managerConditional;
	std::mutex workerMutex;

	bbe::List<std::thread> threads;
	int startSignals = 0;
	int stopSignals = numThreads;

	bool shutdown = false;
	std::mutex shutdownMutex;
	void doShutdown()
	{
		std::unique_lock lock(shutdownMutex);
		shutdown = true;
	}
	bool isShutdown()
	{
		std::unique_lock lock(shutdownMutex);
		return shutdown;
	}

	void startWorkers()
	{
		{
			std::unique_lock<std::mutex> lock(workerMutex);
			startSignals = numThreads;
			stopSignals = 0;
		}
		workerConditional.notify_all();
	}

	void finishedWork()
	{
		{
			std::unique_lock<std::mutex> lock(workerMutex);
			stopSignals++;
		}
		managerConditional.notify_one();
	}

	void waitForWork()
	{
		std::unique_lock<std::mutex> lock(workerMutex);
		workerConditional.wait(lock, [this] { return startSignals > 0; });
		startSignals--;
	}

	void waitForWorkers()
	{
		std::unique_lock<std::mutex> lock(workerMutex);
		managerConditional.wait(lock, [this] { return stopSignals == numThreads; });
	}

	void work(int id)
	{
		while (!isShutdown())
		{
			waitForWork();
			for (int x = id; x < WINDOW_WIDTH; x += numThreads)
			{
				const double x0 = (double)x / (double)WINDOW_WIDTH * rangeX + middleX - rangeX / 2;
				for (int y = 0; y < WINDOW_HEIGHT; y++)
				{
					const double y0 = (double)y / (double)WINDOW_HEIGHT * rangeY + middleY - rangeY / 2;

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
			finishedWork();
		}
	}

	virtual void onStart() override
	{
		for (int i = 0; i < numThreads; i++)
		{
			threads.add(std::thread(&MyGame::work, this, i));
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		static float runningAverage = 29;
		const float currentFps = (1 / timeSinceLastFrame);
		runningAverage = 0.9999f * runningAverage + 0.0001f * currentFps;
		BBELOGLN("AVG FPS: " << runningAverage);

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

		startWorkers();
		waitForWorkers();
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		bbe::List<float> floats;
		floats.resizeCapacityAndLength(WINDOW_WIDTH * WINDOW_HEIGHT);
		float* dataArr = floats.getRaw();

		for (int32_t k = 0; k < WINDOW_HEIGHT; k++)
		{
			for (int32_t i = 0; i < WINDOW_WIDTH; i++)
			{
				const bbe::Color color = bbe::Color::HSVtoRGB(-picData[i][k] * 360 + 240, 1, 1 - picData[i][k]);
				float* p = dataArr + k * WINDOW_WIDTH + i;
				unsigned char* pc = (unsigned char*)p;
				pc[0] = bbe::Math::clamp(color.r * 255, 0.f, 255.f);
				pc[1] = bbe::Math::clamp(color.g * 255, 0.f, 255.f);
				pc[2] = bbe::Math::clamp(color.b * 255, 0.f, 255.f);
				pc[3] = 255;
			}
		}

		bbe::Image image;
		image.load(WINDOW_WIDTH, WINDOW_HEIGHT, (bbe::byte*)dataArr, bbe::ImageFormat::R8G8B8A8);
		brush.drawImage(0, 0, image);
	}
	virtual void onEnd() override
	{
		doShutdown();
		startWorkers();
		for (size_t i = 0; i < threads.getLength(); i++)
		{
			threads[i].join();
		}
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Mandelbrot");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}

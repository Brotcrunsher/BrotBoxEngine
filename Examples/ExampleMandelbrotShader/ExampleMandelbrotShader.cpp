#include "BBE/BrotBoxEngine.h"
#include "AssetStore.h"
#include <iostream>

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

	virtual void onStart() override
	{
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
#ifdef BBE_RENDERER_VULKAN
		assetStore::Mandelbrot()->setPushConstant( 80, 8, &middleX);
		assetStore::Mandelbrot()->setPushConstant( 88, 8, &middleY);
		assetStore::Mandelbrot()->setPushConstant( 96, 8, &rangeX);
		assetStore::Mandelbrot()->setPushConstant(104, 8, &rangeY);
		assetStore::Mandelbrot()->setPushConstant(112, 4, &max_iteration);
#elif defined(BBE_RENDERER_OPENGL)
		assetStore::Mandelbrot()->setUniform1d("middleX", middleX);
		assetStore::Mandelbrot()->setUniform1d("middleY", middleY);
		assetStore::Mandelbrot()->setUniform1d("rangeX", rangeX);
		assetStore::Mandelbrot()->setUniform1d("rangeY", rangeY);
		assetStore::Mandelbrot()->setUniform1i("max_iteration", max_iteration);
#endif
		brush.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, assetStore::Mandelbrot());
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	bbe::Settings::setShaderDoublesAllowed(true);
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "MandelbrotShader");
	delete mg;
}

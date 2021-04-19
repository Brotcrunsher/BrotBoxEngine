#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <future>

constexpr int32_t WINDOW_WIDTH = 1280;
constexpr int32_t WINDOW_HEIGHT = 720;

// Inspired from: https://mikeash.com/pyblog/fluid-simulation-for-dummies.html
// Thank you Mike Ash!


class FluidSquare {
private:
	constexpr static int32_t iter = 4;

	size_t width;
	size_t height;
	float diff;
	float visc;

	bbe::List<float> densityR;
	bbe::List<float> densityG;
	bbe::List<float> densityB;

	bbe::List<float> Vx;
	bbe::List<float> Vy;

	bbe::List<float> Vx0;
	bbe::List<float> Vy0;

	static size_t coordToIndex(int32_t x, int32_t y, int32_t width)
	{
		return x + y * width;
	}

	size_t coordToIndex(int32_t x, int32_t y) const
	{
		return coordToIndex(x, y, getWidth());
	}

	size_t coordToIndexChecked(int32_t x, int32_t y) const
	{
		x = bbe::Math::clamp<int32_t>(x, 0, width - 1);
		y = bbe::Math::clamp<int32_t>(y, 0, height - 1);
		return coordToIndex(x, y);
	}

public:
	size_t getWidth() const
	{
		return width;
	}

	size_t getHeight() const
	{
		return height;
	}

	float getDensityR(size_t x, size_t y) const
	{
		return densityR[coordToIndex(x, y)];
	}

	float getDensityG(size_t x, size_t y) const
	{
		return densityG[coordToIndex(x, y)];
	}

	float getDensityB(size_t x, size_t y) const
	{
		return densityB[coordToIndex(x, y)];
	}

public:
	FluidSquare(size_t width, size_t height, float diffusion, float viscosity)
		: width(width), height(height), diff(diffusion), visc(viscosity)
	{
		densityR.resizeCapacityAndLength(width * height);
		densityG.resizeCapacityAndLength(width * height);
		densityB.resizeCapacityAndLength(width * height);
		Vx      .resizeCapacityAndLength(width * height); 
		Vy      .resizeCapacityAndLength(width * height); 
		Vx0     .resizeCapacityAndLength(width * height);
		Vy0     .resizeCapacityAndLength(width * height);
	}

	void addDensityR(int32_t x, int32_t y, float amount)
	{
		densityR[coordToIndexChecked(bbe::Math::clamp<int32_t>(x, 0, width - 1), bbe::Math::clamp<int32_t>(y, 0, height - 1))] += amount;
	}

	void addDensityG(int32_t x, int32_t y, float amount)
	{
		densityG[coordToIndexChecked(bbe::Math::clamp<int32_t>(x, 0, width - 1), bbe::Math::clamp<int32_t>(y, 0, height - 1))] += amount;
	}

	void addDensityB(int32_t x, int32_t y, float amount)
	{
		densityB[coordToIndexChecked(bbe::Math::clamp<int32_t>(x, 0, width - 1), bbe::Math::clamp<int32_t>(y, 0, height - 1))] += amount;
	}

	void addVelocity(int32_t x, int32_t y, float amountX, float amountY)
	{
		const size_t index = coordToIndexChecked(x, y);

		Vx[index] += amountX;
		Vy[index] += amountY;
	}

	enum class BoundBehaviour
	{
		DENSITY = 0,
		X_SPEED = 1,
		Y_SPEED = 2,
	};

	void setBound(BoundBehaviour b, bbe::List<float> &x)
	{
		for (size_t i = 1; i < getWidth() - 1; i++) {
			x[coordToIndex(i, 0)]               = b == BoundBehaviour::Y_SPEED ? -x[coordToIndex(i, 1)]               : x[coordToIndex(i, 1)];
			x[coordToIndex(i, getHeight() - 1)] = b == BoundBehaviour::Y_SPEED ? -x[coordToIndex(i, getHeight() - 2)] : x[coordToIndex(i, getHeight() - 2)];
		}
		for (size_t i = 1; i < getHeight() - 1; i++) {
			x[coordToIndex(0, i)]              = b == BoundBehaviour::X_SPEED ? -x[coordToIndex(1, i)]              : x[coordToIndex(1, i)];
			x[coordToIndex(getWidth() - 1, i)] = b == BoundBehaviour::X_SPEED ? -x[coordToIndex(getWidth() - 2, i)] : x[coordToIndex(getWidth() - 2, i)];
		}

		x[coordToIndex(0             , 0)]               = 0.5f * (x[coordToIndex(1, 0)]                            + x[coordToIndex(0, 1)]);
		x[coordToIndex(0             , getHeight() - 1)] = 0.5f * (x[coordToIndex(1, getHeight() - 1)]              + x[coordToIndex(0, getHeight() - 2)]);
		x[coordToIndex(getWidth() - 1, 0)]               = 0.5f * (x[coordToIndex(getWidth() - 2, 0)]               + x[coordToIndex(getWidth() - 1, 1)]);
		x[coordToIndex(getWidth() - 1, getHeight() - 1)] = 0.5f * (x[coordToIndex(getWidth() - 2, getHeight() - 1)] + x[coordToIndex(getWidth() - 1, getHeight() - 2)]);
	}

	static void linearSolveThread(size_t start, size_t end, size_t width, size_t height, bbe::List<float>* x, const bbe::List<float>* x0, float a, float cReciprocal)
	{
		// See: Gauss-Seidel
		for (size_t i = start; i < end && i < width - 1; i++) {
			for (size_t j = 1; j < height - 1; j++) {
				(*x)[coordToIndex(i, j, width)] =
					((*x0)[coordToIndex(i, j, width)]
						+ a * ((*x)[coordToIndex(i + 1, j, width)]
							 + (*x)[coordToIndex(i - 1, j, width)]
							 + (*x)[coordToIndex(i, j + 1, width)]
							 + (*x)[coordToIndex(i, j - 1, width)]
							)) * cReciprocal;
			}
		}
	}

	void linearSolve(BoundBehaviour b, bbe::List<float>& x, const bbe::List<float>& x0, float a, float c)
	{
		constexpr size_t amountOfThreads = 10;
		const float cReciprocal = 1.0 / c;
		for (int32_t k = 0; k < iter; k++) {
			bbe::List<std::future<void>> futures;
			for (size_t i = 0; i < amountOfThreads; i++)
			{
				size_t start = 1 + (getWidth() * i / amountOfThreads);
				size_t end  = 1 + (getWidth() * (i + 1) / amountOfThreads);
				futures.add(std::async(std::launch::async, &FluidSquare::linearSolveThread, start, end, getWidth(), getHeight(), &x, &x0, a, cReciprocal));
			}
			for (size_t i = 0; i < amountOfThreads; i++)
			{
				futures[i].wait();
			}
			setBound(b, x);
		}
	}

	void diffuse(BoundBehaviour b, bbe::List<float>& x, const bbe::List<float>& x0, float diff, float dt)
	{
		const float a = dt * diff * (getWidth() - 2) * (getHeight() - 2);
		linearSolve(b, x, x0, a, 1 + 4 * a);
	}

	void clearDivergence(bbe::List<float>& velocX, bbe::List<float>& velocY, bbe::List<float>& p, bbe::List<float>& divergence)
	{
		// See Helmholtz Decomposition
		for (size_t i = 1; i < getWidth() - 1; i++) {
			for (size_t j = 1; j < getHeight() - 1; j++) {
				divergence[coordToIndex(i, j)] = -0.5f * (
					velocX[coordToIndex(i + 1, j)]
					- velocX[coordToIndex(i - 1, j)]
					+ velocY[coordToIndex(i, j + 1)]
					- velocY[coordToIndex(i, j - 1)]
					);
			}
		}
		for (size_t i = 0; i < getWidth() * getHeight(); i++)
		{
			p[i] = 0;
		}
		setBound(BoundBehaviour::DENSITY, divergence);
		linearSolve(BoundBehaviour::DENSITY, p, divergence, 1, 4);

		for (size_t i = 1; i < getWidth() - 1; i++) {
			for (size_t j = 1; j < getHeight() - 1; j++) {
				velocX[coordToIndex(i, j)] -= 0.5f * (p[coordToIndex(i + 1, j)]
					- p[coordToIndex(i - 1, j)]);
				velocY[coordToIndex(i, j)] -= 0.5f * (p[coordToIndex(i, j + 1)]
					- p[coordToIndex(i, j - 1)]);
			}
		}
		setBound(BoundBehaviour::X_SPEED, velocX);
		setBound(BoundBehaviour::Y_SPEED, velocY);
	}

	void advect(BoundBehaviour b, bbe::List<float>& d, const bbe::List<float>& d0, const bbe::List<float>& velocX, const bbe::List<float>& velocY, float dt)
	{
		const float NfloatX = getWidth();
		const float NfloatY = getHeight();

		for (size_t i = 1; i < getWidth() - 1; i++) {
			for (size_t j = 1; j < getHeight() - 1; j++) {
				const float x = bbe::Math::clamp(i - dt * velocX[coordToIndex(i, j)], 0.f, getWidth() - 1.1f);
				const float y = bbe::Math::clamp(j - dt * velocY[coordToIndex(i, j)], 0.f, getHeight() - 1.1f);

				const float i0 = bbe::Math::floor(x);
				const float i1 = i0 + 1.0f;
				const float j0 = bbe::Math::floor(y);
				const float j1 = j0 + 1.0f;

				const float s1 = x - i0;
				const float s0 = 1.0f - s1;
				const float t1 = y - j0;
				const float t0 = 1.0f - t1;

				d[coordToIndex(i, j)] =
					  s0 * (t0 * d0[coordToIndex(i0, j0)]
						 + (t1 * d0[coordToIndex(i0, j1)]))
					+ s1 * (t0 * d0[coordToIndex(i1, j0)]
						 + (t1 * d0[coordToIndex(i1, j1)]));
			}
		}
		setBound(b, d);
	}

	void densityStep(bbe::List<float>* list, float dt)
	{
		bbe::List<float> s;
		s.resizeCapacityAndLength(getWidth() * getHeight());
		diffuse(BoundBehaviour::DENSITY, s, *list, diff, dt);
		advect (BoundBehaviour::DENSITY, *list, s, Vx, Vy, dt);
	}

	void step(float dt)
	{
		diffuse(BoundBehaviour::X_SPEED, Vx0, Vx, visc, dt);
		diffuse(BoundBehaviour::Y_SPEED, Vy0, Vy, visc, dt);

		clearDivergence(Vx0, Vy0, Vx, Vy);

		advect(BoundBehaviour::X_SPEED, Vx, Vx0, Vx0, Vy0, dt);
		advect(BoundBehaviour::Y_SPEED, Vy, Vy0, Vx0, Vy0, dt);

		clearDivergence(Vx, Vy, Vx0, Vy0);

		auto futureR = std::async(std::launch::async, &FluidSquare::densityStep, this, &densityR, dt);
		auto futureG = std::async(std::launch::async, &FluidSquare::densityStep, this, &densityG, dt);
		auto futureB = std::async(std::launch::async, &FluidSquare::densityStep, this, &densityB, dt);

		futureR.wait();
		futureG.wait();
		futureB.wait();
	}

	void densityLoss()
	{
		for (size_t i = 0; i < getWidth() * getHeight(); i++)
		{
			densityR[i] *= 0.998f;
			densityG[i] *= 0.998f;
			densityB[i] *= 0.998f;
		}
	}
};

class MyGame : public bbe::Game
{
public:
	float timeSinceStart = 0;
	float timeSinceStep = 0;
	float stepTime = 1.f / 30.f;
	int pixelSize = 2;
	FluidSquare square = FluidSquare(WINDOW_WIDTH / pixelSize + 1, WINDOW_HEIGHT / pixelSize + 1, 0.f, 0.0000001f);

	virtual void onStart() override
	{
	}

	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << "FPS: " << (1.f / timeSinceLastFrame) << std::endl;

		timeSinceStart += timeSinceLastFrame;
		timeSinceStep  += timeSinceLastFrame;

		if (timeSinceStep > stepTime)
		{
			if (isMouseDown(bbe::MouseButton::LEFT))
			{
				bbe::Vector3 color = bbe::Color::HSVtoRGB(timeSinceStart * 90, 1, 1);
				timeSinceStep -= stepTime;
				constexpr int amountOfSteps = 10;
				for (int step = 0; step < amountOfSteps; step++)
				{
					const bbe::Vector2 mouse = getMouse() - getMouseDelta() * step / amountOfSteps;
					const size_t mouseX = bbe::Math::clamp<size_t>((int)mouse.x / pixelSize, 0, square.getWidth() - 1);
					const size_t mouseY = bbe::Math::clamp<size_t>((int)mouse.y / pixelSize, 0, square.getHeight() - 1);
					constexpr int brushSize = 6;
					for (int i = -brushSize; i <= brushSize; i++)
					{
						for (int k = -brushSize; k <= brushSize; k++)
						{
							bbe::Vector2 brushLocation(i, k);
							if (brushLocation.getLength() <= brushSize)
							{
								const float distance = brushLocation.getLength();
								const float t = distance / brushSize;
								brushLocation = brushLocation.normalize({ 0, 0 });
								square.addVelocity(mouseX + i, mouseY + k, (getMouseDelta().x + brushLocation.x) * 10.f, (getMouseDelta().y + brushLocation.y) * 10.f);
								square.addDensityR(mouseX + i, mouseY + k, bbe::Math::interpolateLinear(color.x * 1.f / amountOfSteps, 0, t));
								square.addDensityG(mouseX + i, mouseY + k, bbe::Math::interpolateLinear(color.y * 1.f / amountOfSteps, 0, t));
								square.addDensityB(mouseX + i, mouseY + k, bbe::Math::interpolateLinear(color.z * 1.f / amountOfSteps, 0, t));
							}
						}
					}
				}
			}
			square.step(stepTime);
			square.densityLoss();
		}
	}

	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}

	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		bbe::List<float> data;
		data.resizeCapacityAndLength(square.getWidth() * square.getHeight());
		for (size_t i = 0; i < square.getWidth(); i++)
		{
			for (size_t k = 0; k < square.getHeight(); k++)
			{
				const size_t index = i + k * square.getWidth();
				const float valR = square.getDensityR(i, k);
				const float valG = square.getDensityG(i, k);
				const float valB = square.getDensityB(i, k);
				char* c = (char*)&data[index];
				c[0] = 255 * bbe::Math::min(valR, 1.f);
				c[1] = 255 * bbe::Math::min(valG, 1.f);
				c[2] = 255 * bbe::Math::min(valB, 1.f);
				c[3] = 255;
			}
		}
		bbe::Image image = bbe::Image(square.getWidth(), square.getHeight(), data.getRaw(), bbe::ImageFormat::R8G8B8A8);
		brush.drawImage(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, image, 0);
	}

	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Fluid Simulation");
	delete mg;
}

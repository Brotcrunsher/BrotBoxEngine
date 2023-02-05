#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <future>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

enum class State
{
	physics,
	slow_down,
	freeze,
	restarting,
};

State& operator++(State& s)
{
	switch (s)
	{
	case State::physics:    return s = State::slow_down;
	case State::slow_down:  return s = State::freeze;
	case State::freeze:     return s = State::restarting;
	case State::restarting: return s = State::physics;
	}
}

float getStateDuration(State& s)
{
	switch (s)
	{
	case State::physics:    return 60.f;
	case State::slow_down:  return 1.f;
	case State::freeze:		return 1.f;
	case State::restarting: return 3.f;
	}
}

class MyGame : public bbe::Game
{
	State state = State::freeze;

	class DoublePendulum
	{
	public:
		enum class DrawMode
		{
			INNER,
			OUTER,
			BOTH
		};

		constexpr static double L1 = 180;
		constexpr static double L2 = 162;
		constexpr static double g = 100;
		constexpr static double m1 = 10;
		constexpr static double m2 = 1;
		constexpr static double tickTime = 0.0001;

		double interpolationStartOmega1;
		double interpolationStartOmega2;

		double omega1;
		double omega2;

		double velo1;
		double velo2;

		DoublePendulum(double omega1, double omega2, double velo1, double velo2)
			: omega1(omega1), omega2(omega2), velo1(velo1), velo2(velo2)
		{

		}

		void tick()
		{
			const double accel1 =
				(-g * (2. * m1 + m2) * sin(omega1) - m2 * g * sin(omega1 - 2. * omega2) - 2. * sin(omega1 - omega2) * m2 * (velo2 * velo2 * L2 + velo1 * velo1 * L1 * cos(omega1 - omega2)))
				/
				(L1 * (2 * m1 + m2 - m2 * cos(2. * omega1 - 2. * omega2)));

			const double accel2 =
				(2. * sin(omega1 - omega2) * (velo1 * velo1 * L1 * (m1 + m2) + g * (m1 + m2) * cos(omega1) + velo2 * velo2 * L2 * m2 * cos(omega1 - omega2)))
				/
				(L2 * (2 * m1 + m2 - m2 * cos(2. * omega1 - 2. * omega2)));

			velo1 += accel1 * tickTime;
			velo2 += accel2 * tickTime;
			omega1 += velo1 * tickTime;
			omega2 += velo2 * tickTime;

			if (omega1 > bbe::Math::PI) omega1 -= bbe::Math::PI * 2.;
			if (omega1 < -bbe::Math::PI) omega1 += bbe::Math::PI * 2.;

			if (omega2 > bbe::Math::PI) omega2 -= bbe::Math::PI * 2.;
			if (omega2 < -bbe::Math::PI) omega2 += bbe::Math::PI * 2.;
		}

		void draw(bbe::PrimitiveBrush2D& brush, DrawMode drawMode)
		{
			const bbe::Vector2 offset = { WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 };

			float x1 = offset.x + L1 * bbe::Math::sin(omega1);
			float y1 = offset.y + L1 * bbe::Math::cos(omega1);
			float x2 = x1 + L2 * bbe::Math::sin(omega2);
			float y2 = y1 + L2 * bbe::Math::cos(omega2);

			if (drawMode == DrawMode::INNER || drawMode == DrawMode::BOTH) brush.fillLine(offset, x1, y1);
			if (drawMode == DrawMode::OUTER || drawMode == DrawMode::BOTH) brush.fillLine(x1, y1, x2, y2);
		}
	};

	bbe::List<DoublePendulum> pendulums;

	virtual void onStart() override
	{
		for (int i = 0; i < 5000; i++)
		{
			float percentage = (float)i / 5000.f;
			pendulums.add(DoublePendulum(
				bbe::Math::TAU * percentage,
				bbe::Math::TAU * percentage,
				-bbe::Math::PI / 2,
				-bbe::Math::PI / 2
			));
		}
	}

	void tickPendulums(size_t from, size_t to, size_t ticks)
	{
		for (size_t k = from; k < to && k < pendulums.getLength(); k++)
		{
			for (size_t i = 0; i < ticks; i++)
			{
				pendulums[k].tick();
			}
		}
	}
	
	virtual void update(float timeSinceLastFrame) override
	{
		static float stateTime = 0;
		stateTime += timeSinceLastFrame;
		float statePercentage = stateTime / getStateDuration(state);
		if (statePercentage > 1) statePercentage = 1;

		if (state == State::physics
			|| state == State::slow_down)
		{
			static float timeSinceLastTick = 0;
			timeSinceLastTick += timeSinceLastFrame;

			size_t ticks = timeSinceLastTick / DoublePendulum::tickTime;
			if (ticks > 0)
			{
				timeSinceLastTick -= ticks * DoublePendulum::tickTime; timeSinceLastTick -= DoublePendulum::tickTime;

				if (state == State::slow_down)
				{
					ticks = (size_t)((float)ticks * (1.f - statePercentage));
				}

				bbe::List<std::future<void>> futures;
				const size_t increment = pendulums.getLength() / 12;
				for (size_t i = 0; i < pendulums.getLength(); i += increment)
				{
					futures.add(std::async(std::launch::async, &MyGame::tickPendulums, this, i, i + increment, ticks));
				}
				for (size_t i = 0; i < futures.getLength(); i++)
				{
					futures[i].wait();
				}
			}
		}
		else if (state == State::restarting)
		{
			for (size_t i = 0; i < pendulums.getLength(); i++)
			{
				pendulums[i].omega1 = bbe::Math::interpolateCosine(pendulums[i].interpolationStartOmega1, pendulums[0].omega1, statePercentage);
				pendulums[i].omega2 = bbe::Math::interpolateCosine(pendulums[i].interpolationStartOmega2, pendulums[0].omega2, statePercentage);
			}
		}

		if (statePercentage == 1)
		{
			++state;
			stateTime = 0;

			if (state == State::physics)
			{
				for (size_t i = 0; i < pendulums.getLength(); i++)
				{
					pendulums[i].omega1 = pendulums[0].omega1 + 0.0000001 * i;
					pendulums[i].omega2 = pendulums[0].omega2;
					pendulums[i].velo1 = pendulums[0].velo1;
					pendulums[i].velo2 = pendulums[0].velo2;
				}
			}

			if (state == State::restarting)
			{
				for (size_t i = 0; i < pendulums.getLength(); i++)
				{
					pendulums[i].interpolationStartOmega1 = pendulums[i].omega1;
					pendulums[i].interpolationStartOmega2 = pendulums[i].omega2;
					pendulums[i].velo1 = pendulums[0].velo1;
					pendulums[i].velo2 = pendulums[0].velo2;
				}
			}
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		for (size_t k = 0; k < pendulums.getLength(); k++)
		{
			brush.setColorHSV(360.f * k / pendulums.getLength(), 1, 1, 0.1f);
			pendulums[k].draw(brush, DoublePendulum::DrawMode::INNER);
		}
		for (size_t k = 0; k < pendulums.getLength(); k++)
		{
			brush.setColorHSV(360.f * k / pendulums.getLength(), 1, 1, 0.1f);
			pendulums[k].draw(brush, DoublePendulum::DrawMode::OUTER);
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	// Enable these to record. WARNING: Generates a LOT of images!
	//mg->setScreenshotRecordingMode();
	//mg->setMaxFrame(10 * 60 * 60);
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Double Pendulum!");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}

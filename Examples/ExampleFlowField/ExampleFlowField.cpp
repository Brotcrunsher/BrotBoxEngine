#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <set>

constexpr float WINDOW_WIDTH  = 1280.f;
constexpr float WINDOW_HEIGHT =  720.f;

class MyGame : public bbe::Game
{
	struct FlowField
	{
		constexpr static float gridSize = 20;
		constexpr static size_t gridWidth = WINDOW_WIDTH / gridSize + 1;
		constexpr static size_t gridHeight = WINDOW_HEIGHT / gridSize + 1;
		bbe::Vector2 grid[gridWidth][gridHeight];

		void reset()
		{
			for (size_t i = 0; i < gridWidth; i++)
			{
				for (size_t k = 0; k < gridHeight; k++)
				{
					grid[i][k] = {};
				}
			}
		}

		size_t locationToGridX(float x)
		{
			x = bbe::Math::clamp(x, 0.f, WINDOW_WIDTH);
			return x / gridSize;
		}

		size_t locationToGridY(float y)
		{
			y = bbe::Math::clamp(y, 0.f, WINDOW_HEIGHT);
			return y / gridSize;
		}

		bbe::Vector2 getCellCenter(size_t x, size_t y)
		{
			return bbe::Vector2(x * gridSize + gridSize / 2, y * gridSize + gridSize / 2);
		}

		void addElement(const bbe::Vector2& start, const bbe::Vector2& dir)
		{
			const bbe::Vector2 normDir = dir.normalize();
			const int64_t locationX = (int64_t)locationToGridX(start.x);
			const int64_t locationY = (int64_t)locationToGridY(start.y);

			const int64_t fromX = bbe::Math::clamp<int64_t>(locationX - 10, 0, gridWidth - 1);
			const int64_t toX   = bbe::Math::clamp<int64_t>(locationX + 10, 0, gridWidth - 1);
			
			const int64_t fromY = bbe::Math::clamp<int64_t>(locationY - 10, 0, gridHeight - 1);
			const int64_t toY   = bbe::Math::clamp<int64_t>(locationY + 10, 0, gridHeight - 1);

			for (auto i = fromX; i <= toX; i++)
			{
				for (auto k = fromY; k <= toY; k++)
				{
					const bbe::Vector2 startToCellCenter = getCellCenter(i, k) - start;
					const float dot = startToCellCenter.normalize() * normDir;
					if (dot > 0.5)
					{
						float length = startToCellCenter.getLength();
						if (length > 10)
						{
							grid[i][k] += startToCellCenter / length / length;
						}
					}
				}
			}
		}

		void draw(bbe::PrimitiveBrush2D& brush)
		{
			for (size_t i = 0; i < gridWidth; i++)
			{
				for (size_t k = 0; k < gridHeight; k++)
				{
					if (grid[i][k].x > 0)
					{
						brush.setColorRGB(1, 0, 0);
					}
					else
					{
						brush.setColorRGB(0, 0, 1);
					}
					const bbe::Vector2 cellCenter = getCellCenter(i, k);
					brush.fillLine(cellCenter, cellCenter + grid[i][k] * 50, 3);
				}
			}
		}

		bbe::Vector2 getFlow(const bbe::Vector2& pos, const bbe::Vector2& dir)
		{
			const bbe::Vector2 normDir = dir.normalize();
			const int64_t locationX = (int64_t)locationToGridX(pos.x);
			const int64_t locationY = (int64_t)locationToGridY(pos.y);

			const int64_t fromX = bbe::Math::clamp<int64_t>(locationX - 5, 0, gridWidth - 1);
			const int64_t toX   = bbe::Math::clamp<int64_t>(locationX + 5, 0, gridWidth - 1);

			const int64_t fromY = bbe::Math::clamp<int64_t>(locationY - 5, 0, gridHeight - 1);
			const int64_t toY   = bbe::Math::clamp<int64_t>(locationY + 5, 0, gridHeight - 1);

			bool flowFieldContradictsMovement = false;
			for (auto i = fromX; i <= toX; i++)
			{
				const bbe::Vector2 startToCellCenter = getCellCenter(i, locationY) - pos;
				const float dot = startToCellCenter.normalize() * normDir;
				const float dot2 = grid[i][locationY] * dir;
				if (dot > 0 && dot2 < 0)
				{
					flowFieldContradictsMovement = true;
					break;
				}
			}
			if (!flowFieldContradictsMovement)
			{
				return normDir * 100;
			}

			float biggestDot = -10000;
			bbe::Vector2 retVal;
			for (auto i = fromX; i <= toX; i++)
			{
				for (auto k = fromY; k <= toY; k++)
				{
					const bbe::Vector2 startToCellCenter = getCellCenter(i, k) - pos;
					if (startToCellCenter.isSameDirection(normDir))
					{
						const float dot = grid[i][k] * dir;
						if (dot > biggestDot)
						{
							biggestDot = dot;
							retVal = startToCellCenter.normalize() * 100;
						}
					}
				}
			}
			return retVal;
		}
	};

	FlowField flowField;

	bbe::List<bbe::PhysCircle> leftToRightCircles;
	bbe::List<bbe::PhysCircle> rightToLeftCircles;
	bbe::Random rand;

	void reinitialize()
	{
		for (bbe::PhysCircle& c : leftToRightCircles)
		{
			c.destroy();
		}
		leftToRightCircles.clear();
		for (bbe::PhysCircle& c : rightToLeftCircles)
		{
			c.destroy();
		}
		rightToLeftCircles.clear();

		for (uint32_t i = 0; i < 256; i++)
		{
			leftToRightCircles.add(bbe::PhysCircle(this, rand.randomFloat(300.f), rand.randomFloat(WINDOW_HEIGHT - 20), 10));
			rightToLeftCircles.add(bbe::PhysCircle(this, WINDOW_WIDTH - 300.f + rand.randomFloat(300.f), rand.randomFloat(WINDOW_HEIGHT - 20), 10));
		}
	}

	virtual void onStart() override
	{
		constexpr float blockerWidth = 20;
		bbe::PhysRectangle topBlocker    = bbe::PhysRectangle(this, -blockerWidth, -blockerWidth, WINDOW_WIDTH + blockerWidth * 2, blockerWidth);
		bbe::PhysRectangle bottomBlocker = bbe::PhysRectangle(this, -blockerWidth, WINDOW_HEIGHT, WINDOW_WIDTH + blockerWidth * 2, blockerWidth);
		bbe::PhysRectangle leftBlocker   = bbe::PhysRectangle(this, -blockerWidth, -blockerWidth, blockerWidth, WINDOW_HEIGHT + blockerWidth * 2);
		bbe::PhysRectangle rightBlocker  = bbe::PhysRectangle(this, WINDOW_WIDTH , -blockerWidth, blockerWidth, WINDOW_HEIGHT + blockerWidth * 2);
		topBlocker   .freeze();
		bottomBlocker.freeze();
		leftBlocker  .freeze();
		rightBlocker .freeze();

		getPhysWorld()->setGravity({ 0, 0 });

		reinitialize();
	}
	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << "FPS: " << (1.f / timeSinceLastFrame) << std::endl;
		if (isKeyPressed(bbe::Key::SPACE))
		{
			reinitialize();
		}
		flowField.reset();
		for (bbe::PhysCircle& c : leftToRightCircles)
		{
			flowField.addElement(c.getPos() + bbe::Vector2(c.getRadius(), c.getRadius()) / 2, { 1, 0 });
		}
		for (bbe::PhysCircle& c : rightToLeftCircles)
		{
			flowField.addElement(c.getPos() + bbe::Vector2(c.getRadius(), c.getRadius()) / 2, { -1, 0 });
		}

		constexpr float flowFieldObedience = 1.0;
		for (bbe::PhysCircle& c : leftToRightCircles)
		{
			auto speed = bbe::Vector2(100 * (1 - flowFieldObedience), 0) + flowField.getFlow(c.getPos() + bbe::Vector2(c.getRadius(), c.getRadius()) / 2, { 1, 0 }) * 100 * flowFieldObedience;
			speed = speed.normalize() * 100;
			c.setSpeed(speed);
		}
		for (bbe::PhysCircle& c : rightToLeftCircles)
		{
			auto speed = bbe::Vector2(-100 * (1 - flowFieldObedience), 0) + flowField.getFlow(c.getPos() + bbe::Vector2(c.getRadius(), c.getRadius()) / 2, { -1, 0 }) * 100 * flowFieldObedience;
			speed = speed.normalize() * 100;
			c.setSpeed(speed);
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		brush.setColorRGB(1, 1, 1);
		brush.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		brush.setColorRGB(1, 0, 0);
		for (const bbe::PhysCircle& c : leftToRightCircles)
		{
			brush.fillCircle(c);
		}
		brush.setColorRGB(0, 0, 1);
		for (const bbe::PhysCircle& c : rightToLeftCircles)
		{
			brush.fillCircle(c);
		}

		brush.setColorRGB(0, 1, 0);
		flowField.draw(brush);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "FlowField!");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}


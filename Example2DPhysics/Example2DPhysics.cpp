#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <set>

constexpr float WINDOW_WIDTH  = 1280.f;
constexpr float WINDOW_HEIGHT =  720.f;

class MyGame : public bbe::Game
{
	struct PhysCircle
	{
		bbe::Circle circle;
		bbe::Vector2 speed;
		float mass = 1;
		size_t gridX = 0;
		size_t gridY = 0;
		float hue = 0;
		float value = 0;
	};

	bbe::List<PhysCircle> circles;
	bbe::Random rand;
	
	constexpr static float MAX_RADIUS = 10.f;
	constexpr static size_t GRID_WIDTH  = size_t(WINDOW_WIDTH  / (MAX_RADIUS * 2)) + 1;
	constexpr static size_t GRID_HEIGHT = size_t(WINDOW_HEIGHT / (MAX_RADIUS * 2)) + 1;
	bbe::List<size_t> grid[GRID_WIDTH][GRID_HEIGHT];

	void prepareGrid()
	{
		for (size_t i = 0; i < GRID_WIDTH; i++)
		{
			for (size_t k = 0; k < GRID_HEIGHT; k++)
			{
				grid[i][k].clear();
			}
		}
		for (size_t i = 0; i < circles.getLength(); i++)
		{
			const size_t x = size_t(bbe::Math::clamp(circles[i].circle.getX(), 0, WINDOW_WIDTH ) / (MAX_RADIUS * 2));
			const size_t y = size_t(bbe::Math::clamp(circles[i].circle.getY(), 0, WINDOW_HEIGHT) / (MAX_RADIUS * 2));
			grid[x][y].add(i);
			circles[i].gridX = x;
			circles[i].gridY = y;
		}
	}

	bool checkForCollisionsInGridCell(size_t i, size_t gridX, size_t gridY)
	{
		bool foundCollision = false;
		if (gridX >= GRID_WIDTH || gridY >= GRID_HEIGHT) return false;
		for (size_t k = 0; k < grid[gridX][gridY].getLength(); k++)
		{
			if (grid[gridX][gridY][k] == i) continue;
			if (circles[i].circle.resolveIntersection(circles[grid[gridX][gridY][k]].circle, circles[i].mass, circles[grid[gridX][gridY][k]].mass))
			{
				collisionReaction(circles[i], circles[grid[gridX][gridY][k]]);
				foundCollision = true;
			}
		}
		return foundCollision;
	}

	bool checkForCollisions(size_t i)
	{
		bool foundCollision = false;
		foundCollision |= checkForCollisionsInGridCell(i, circles[i].gridX - 1, circles[i].gridY - 1);
		foundCollision |= checkForCollisionsInGridCell(i, circles[i].gridX    , circles[i].gridY - 1);
		foundCollision |= checkForCollisionsInGridCell(i, circles[i].gridX + 1, circles[i].gridY - 1);
		foundCollision |= checkForCollisionsInGridCell(i, circles[i].gridX - 1, circles[i].gridY    );
		foundCollision |= checkForCollisionsInGridCell(i, circles[i].gridX    , circles[i].gridY    );
		foundCollision |= checkForCollisionsInGridCell(i, circles[i].gridX + 1, circles[i].gridY    );
		foundCollision |= checkForCollisionsInGridCell(i, circles[i].gridX - 1, circles[i].gridY + 1);
		foundCollision |= checkForCollisionsInGridCell(i, circles[i].gridX    , circles[i].gridY + 1);
		foundCollision |= checkForCollisionsInGridCell(i, circles[i].gridX + 1, circles[i].gridY + 1);
		return foundCollision;
	}

	void collisionReaction(PhysCircle& c1, PhysCircle& c2)
	{
		constexpr float elasticity = 0.0f;

		const bbe::Vector2 n = (c1.circle.getMiddle() - c2.circle.getMiddle()).normalize();
		
		const float counter = -(1.f + elasticity) * ((c1.speed - c2.speed) * n);
		const float nominal = (1.f / c1.mass) + (1 / c2.mass);
		const float f = counter / nominal;
		
		c1.speed += n * (f / c1.mass);
		c2.speed -= n * (f / c2.mass);
	}

	void clampToLevel(PhysCircle& c)
	{
		if (c.circle.getX() < 0)
		{
			c.circle.setX(-c.circle.getX());
			c.speed.x *= -0.5f;
		}
		if (c.circle.getX() > WINDOW_WIDTH - c.circle.getWidth())
		{
			c.circle.setX((WINDOW_WIDTH - c.circle.getWidth()) * 2 - c.circle.getX());
			c.speed.x *= -0.5f;
		}
		if (c.circle.getY() < 0)
		{
			c.circle.setY(-c.circle.getY());
			c.speed.y *= -0.5f;
		}
		if (c.circle.getY() > WINDOW_HEIGHT - c.circle.getHeight())
		{
			c.circle.setY((WINDOW_HEIGHT - c.circle.getHeight()) * 2 - c.circle.getY());
			c.speed.y *= -0.5f;
		}
	}

	void pushFromEdges(PhysCircle& c, float timeSinceLastFrame)
	{
		constexpr float edgeSize = 100;
		constexpr float edgeMultiplier = 0.00001f;
		if (c.circle.getX() < edgeSize)
		{
			c.speed += bbe::Vector2(edgeSize - c.circle.getX(), 0) * edgeMultiplier;
		}
		if (c.circle.getX() > WINDOW_WIDTH - c.circle.getWidth() - edgeSize)
		{
			c.speed -= bbe::Vector2(c.circle.getX() - (WINDOW_WIDTH - c.circle.getWidth() - edgeSize), 0) * edgeMultiplier;
		}
		if (c.circle.getY() < edgeSize)
		{
			c.speed += bbe::Vector2(0, edgeSize - c.circle.getY()) * edgeMultiplier;
		}
		if (c.circle.getY() > WINDOW_HEIGHT - c.circle.getHeight() - edgeSize)
		{
			c.speed -= bbe::Vector2(0, c.circle.getY() - (WINDOW_HEIGHT - c.circle.getHeight() - edgeSize)) * edgeMultiplier;
		}
	}

	virtual void onStart() override
	{
		PhysCircle playerCircle{
				bbe::Circle(
					rand.randomFloat(WINDOW_WIDTH  - MAX_RADIUS * 2),
					rand.randomFloat(WINDOW_HEIGHT - MAX_RADIUS * 2),
					MAX_RADIUS * 2,
					MAX_RADIUS * 2
				),
				rand.randomVector2InUnitSphere() * 100,
				1000,
		};
		circles.add(playerCircle);

		for (uint32_t i = 0; i < 1024 * 8; i++)
		{
			const float radius = MAX_RADIUS * 0.25f;
			circles.add(PhysCircle{
				bbe::Circle(
					rand.randomFloat(WINDOW_WIDTH - radius * 2),
					rand.randomFloat(WINDOW_HEIGHT - radius * 2),
					radius * 2,
					radius * 2
				),
				rand.randomVector2InUnitSphere() * 100
			});
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << "FPS: " << (1.f / timeSinceLastFrame) << std::endl;
		if (isKeyPressed(bbe::Key::SPACE))
		{
			for (PhysCircle& circle : circles)
			{
				circle.speed += rand.randomVector2InUnitSphere() * 100;
			}
		}
		if (isKeyPressed(bbe::Key::Q))
		{
			for (PhysCircle& circle : circles)
			{
				circle.hue   = circle.circle.getX() / WINDOW_WIDTH * 360.0f;
				circle.value = circle.circle.getY() / WINDOW_HEIGHT;
			}
		}
		constexpr float CONTROL_SPEED = 100.f;
		float controlSpeed = CONTROL_SPEED * timeSinceLastFrame;
		if (isKeyDown(bbe::Key::W))
		{
			circles[0].speed -= bbe::Vector2(0, controlSpeed);
		}
		if (isKeyDown(bbe::Key::S))
		{
			circles[0].speed += bbe::Vector2(0, controlSpeed);
		}
		if (isKeyDown(bbe::Key::A))
		{
			circles[0].speed -= bbe::Vector2(controlSpeed, 0);
		}
		if (isKeyDown(bbe::Key::D))
		{
			circles[0].speed += bbe::Vector2(controlSpeed, 0);
		}

		for (PhysCircle& circle : circles)
		{
			circle.circle.setX(circle.circle.getX() + circle.speed.x * timeSinceLastFrame);
			circle.circle.setY(circle.circle.getY() + circle.speed.y * timeSinceLastFrame);
			circle.speed = circle.speed * 0.999f;
			clampToLevel(circle);
			pushFromEdges(circle, timeSinceLastFrame);
		}

		std::set<size_t> requireUpdate;
		for (size_t i = 0; i < circles.getLength(); i++)
		{
			requireUpdate.insert(i);
		}

		while (requireUpdate.size() > 0)
		{
			prepareGrid();
			std::set<size_t> requireUpdateNew;
			for (auto it = requireUpdate.begin(); it != requireUpdate.end(); it++)
			{
				size_t i = *it;
				if (checkForCollisions(i))
				{
					requireUpdateNew.insert(i);
				}
			}
			requireUpdate = requireUpdateNew;
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		brush.setColorRGB(1, 1, 1);
		brush.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		brush.setColorRGB(0, 0, 0);
		for (const PhysCircle& circle : circles)
		{
			brush.setColorHSV(circle.hue, 1, circle.value / 2.f + 0.5f);
			brush.fillCircle(circle.circle);
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "2D Physics!");

    return 0;
}


#include "BBE/BrotBoxEngine.h"
#include <iostream>

enum class Direction
{
	UP, DOWN, LEFT, RIGHT
};

struct BodyPart
{
	int x;
	int y;

	bool operator==(BodyPart &other)
	{
		return (other.x == x && other.y == y);
	}
};

class MyGame : public bbe::Game
{
public:
	constexpr static int GRIDWIDTH  = 20;
	constexpr static int GRIDHEIGHT = 20;
	constexpr static int CELLSIZE   = 10;
	constexpr static float TICKTIME = 0.25f;

	bbe::Cube cube;
	bbe::PointLight light;

	Direction dir     = Direction::LEFT;
	Direction nextDir = Direction::LEFT;

	bbe::List<BodyPart> bodyParts;
	BodyPart food;

	float timeSinceLastTick = 0;

	bbe::Random rand;

	bool gameOver = false;

	float rot = 0;

	virtual void onStart() override
	{
		light.setPosition(bbe::Vector3(-1, -1, 1));
		light.setLightStrength(5);

		bodyParts.add({ GRIDWIDTH / 2, GRIDHEIGHT / 2 });
		bodyParts.add({ -1, -1 });
		bodyParts.add({ -1, -1 });

		placeFood();
	}

	virtual void update(float timeSinceLastFrame) override
	{
		rot += timeSinceLastFrame;
		cube.set(bbe::Vector3(0, 2, bbe::Math::sin(rot) * 1), bbe::Vector3(1, 1, 1), bbe::Vector3(0, 0, 1), rot);

		input();
		timeSinceLastTick += timeSinceLastFrame;
		if (timeSinceLastTick > TICKTIME)
		{
			timeSinceLastTick -= TICKTIME;
			tick();
		}
	}

	void tick()
	{
		if (gameOver)
		{
			return;
		}

		dir = nextDir;

		for (int i = bodyParts.getLength() - 1; i >= 1; i--)
		{
			bodyParts[i] = bodyParts[i - 1];
		}
		switch (dir)
		{
		case Direction::UP:
			bodyParts[0].y--;
			break;
		case Direction::DOWN:
			bodyParts[0].y++;
			break;
		case Direction::LEFT:
			bodyParts[0].x--;
			break;
		case Direction::RIGHT:
			bodyParts[0].x++;
			break;
		default:
			throw bbe::IllegalStateException();
		}

		if (bodyParts[0].x < 0)
		{
			bodyParts[0].x = GRIDWIDTH - 1;
		}
		if (bodyParts[0].y < 0)
		{
			bodyParts[0].y = GRIDHEIGHT - 1;
		}
		if (bodyParts[0].x >= GRIDWIDTH)
		{
			bodyParts[0].x = 0;
		}
		if (bodyParts[0].y >= GRIDHEIGHT)
		{
			bodyParts[0].y = 0;
		}


		if (bodyParts[0].x == food.x && bodyParts[0].y == food.y)
		{
			bodyParts.add({ -1, -1 });
			placeFood();
		}

		checkGameOver();
	}

	void input()
	{
		if (dir == Direction::LEFT || dir == Direction::RIGHT)
		{
			if (isKeyDown(bbe::Key::W) || isKeyDown(bbe::Key::UP))
			{
				nextDir = Direction::UP;
			}
			else if (isKeyDown(bbe::Key::S) || isKeyDown(bbe::Key::DOWN))
			{
				nextDir = Direction::DOWN;
			}
		}
		else if (dir == Direction::UP || dir == Direction::DOWN)
		{
			if (isKeyDown(bbe::Key::A) || isKeyDown(bbe::Key::LEFT))
			{
				nextDir = Direction::LEFT;
			}
			else if (isKeyDown(bbe::Key::D) || isKeyDown(bbe::Key::RIGHT))
			{
				nextDir = Direction::RIGHT;
			}
		}
	}

	void checkGameOver()
	{
		for (size_t i = 1; i < bodyParts.getLength(); i++)
		{
			if (bodyParts[i] == bodyParts[0])
			{
				gameOver = true;
			}
		}
	}

	void placeFood()
	{
		start:
		int randX = rand.randomInt(GRIDWIDTH);
		int randY = rand.randomInt(GRIDHEIGHT);
		for (BodyPart bp : bodyParts)
		{
			if (bp.x == randX && bp.y == randY)
			{
				goto start; //I also like to live dangerously.
			}
		}
		food.x = randX;
		food.y = randY;
	}

	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		brush.setColorRGB(1, 1, 1, 0.5f);
		for (BodyPart bp : bodyParts)
		{
			brush.fillRect(bp.x * CELLSIZE, bp.y * CELLSIZE, CELLSIZE, CELLSIZE);
		}

		brush.setColorRGB(0.5f, 1, 0.5f, 0.5f);
		brush.fillCircle(food.x * CELLSIZE, food.y * CELLSIZE, CELLSIZE, CELLSIZE);
	}

	virtual void onEnd() override
	{
	}

	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.setCamera(bbe::Vector3(0, 0, 1), bbe::Vector3(0, 2, 0));
		brush.setColor(0.5f, 0.3f, 0.2f);
		brush.fillCube(cube);
	}
};


int main()
{
	MyGame mg;
	mg.start(mg.CELLSIZE * mg.GRIDWIDTH, mg.CELLSIZE * mg.GRIDHEIGHT, "Snake!");

    return 0;
}


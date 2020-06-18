#include "BBE/BrotBoxEngine.h"
#include <iostream>

enum class Direction
{
	UP, DOWN, LEFT, RIGHT, FRONT, BACK
};

struct BodyPart
{
	int x;
	int y;
	int z;

	bool operator==(BodyPart &other)
	{
		return (other.x == x && other.y == y && other.z == z);
	}
};

class MyGame : public bbe::Game
{
	//TODO there is a visible flicker some times that needs to be fixed.
public:
	constexpr static int GRIDDIMENSIONSIZE = 10;
	const float TICKTIME = 0.25f;

	bbe::PointLight light;

	Direction dir = Direction::LEFT;
	Direction nextDir = Direction::LEFT;

	bbe::List<BodyPart> bodyParts;
	BodyPart food;

	float timeSinceLastTick = 0;

	bbe::Random rand;

	bbe::Vector3 camPos = bbe::Vector3(-10, -10, 10);

	bool gameOver = false;

	virtual void onStart() override
	{
		light.setPosition(camPos);
		light.setLightStrength(10);

		bodyParts.add({ GRIDDIMENSIONSIZE / 2, GRIDDIMENSIONSIZE / 2, GRIDDIMENSIONSIZE / 2 });
		bodyParts.add({ -1, -1, -1 });
		bodyParts.add({ -1, -1, -1 });

		placeFood();
	}

	virtual void update(float timeSinceLastFrame) override
	{

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
		case Direction::FRONT:
			bodyParts[0].z++;
			break;
		case Direction::BACK:
			bodyParts[0].z--;
			break;
		default:
			throw bbe::IllegalStateException();
		}

		if (bodyParts[0].x < 0)
		{
			bodyParts[0].x = GRIDDIMENSIONSIZE - 1;
		}
		if (bodyParts[0].y < 0)
		{
			bodyParts[0].y = GRIDDIMENSIONSIZE - 1;
		}
		if (bodyParts[0].z < 0)
		{
			bodyParts[0].z = GRIDDIMENSIONSIZE - 1;
		}
		if (bodyParts[0].x >= GRIDDIMENSIONSIZE)
		{
			bodyParts[0].x = 0;
		}
		if (bodyParts[0].y >= GRIDDIMENSIONSIZE)
		{
			bodyParts[0].y = 0;
		}
		if (bodyParts[0].z >= GRIDDIMENSIONSIZE)
		{
			bodyParts[0].z = 0;
		}


		if (bodyParts[0].x == food.x && bodyParts[0].y == food.y && bodyParts[0].z == food.z)
		{
			bodyParts.add({ -1, -1 });
			placeFood();
		}

		checkGameOver();
	}

	void input()
	{
		if (isKeyDown(bbe::Key::W) && dir != Direction::BACK)
		{
			nextDir = Direction::FRONT;
		}
		else if (isKeyDown(bbe::Key::S) && dir != Direction::FRONT)
		{
			nextDir = Direction::BACK;
		}
		else if (isKeyDown(bbe::Key::SPACE) && dir != Direction::DOWN)
		{
			nextDir = Direction::UP;
		}
		else if (isKeyDown(bbe::Key::C) && dir != Direction::UP)
		{
			nextDir = Direction::DOWN;
		}
		else if (isKeyDown(bbe::Key::A) && dir != Direction::RIGHT)
		{
			nextDir = Direction::LEFT;
		}
		else if (isKeyDown(bbe::Key::D) && dir != Direction::LEFT)
		{
			nextDir = Direction::RIGHT;
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
		int randX = rand.randomInt(GRIDDIMENSIONSIZE);
		int randY = rand.randomInt(GRIDDIMENSIONSIZE);
		int randZ = rand.randomInt(GRIDDIMENSIONSIZE);
		for (BodyPart bp : bodyParts)
		{
			if (bp.x == randX && bp.y == randY && bp.z == randZ)
			{
				goto start; //I also like to live dangerously.
			}
		}
		food.x = randX;
		food.y = randY;
		food.z = randZ;
	}

	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{

	}

	virtual void onEnd() override
	{
	}

	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.setCamera(camPos, bbe::Vector3(GRIDDIMENSIONSIZE / 2, GRIDDIMENSIONSIZE / 2, GRIDDIMENSIONSIZE / 2));
		brush.setColor(1, 1, 1, 0.5f); //TODO: Alpha has no effect.
		for (const BodyPart& bp : bodyParts)
		{
			if (bp.x != -1 && bp.y != -1 && bp.z != -1)
			{
				//TODO there should be a different possible render mode that has smooth normals
				brush.fillCube(bbe::Cube(bbe::Vector3(bp.x, bp.y, bp.z)));
			}
		}
		brush.setColor(0, 1, 0);
		brush.fillCube(bbe::Cube(bbe::Vector3(food.x, food.y, food.z)));
	}
};


int main()
{
	MyGame mg;
	mg.start(1280, 720, "Snake 3D!");

	return 0;
}


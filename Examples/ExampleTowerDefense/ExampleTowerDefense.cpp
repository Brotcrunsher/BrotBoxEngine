#include "BBE/BrotBoxEngine.h"
#include <iostream>

//TODO: bbe::Random still uses std stuff! Bad for checkability of true rounds.
//TODO: Endless Grid might have a lot of allocations that require rethinking to optimize for performance.

enum class Direction
{
	UP,
	LEFT,
	DOWN,
	RIGHT,
	UNKNOWN,
};

bbe::Color heightToColor(int32_t height)
{
	if (height == 0)
	{
		return bbe::Color(1, 0, 0);
	}
	else if (height == -1)
	{
		return bbe::Color(1, 1, 0);
	}
	else if (height == 1)
	{
		return bbe::Color(0, 1, 0);
	}
	else
	{
		return bbe::Color(0, 0, 0);
	}
}

int32_t mergeTiles(int32_t a, int32_t b)
{
	if (a == -1 || b == -1) return -1;
	if (a == 0 || b == 0) return 0;
	return a > b ? a : b;
}

bbe::List<bbe::Vector2i> path;

class Tile
{
public:
	struct Entrance
	{
		bbe::Vector2i position;
		Direction dir;
	};
public:
	static constexpr size_t gridSize = 13;

private:
	bbe::Grid<int32_t> grid;


	int32_t getCrossCheck(size_t x, size_t y) const
	{
		int32_t retVal = 0;
		if (grid[x][y] == 0) retVal++;
		if (x > 0 && grid[x - 1][y] == 0) retVal++;
		if (y > 0 && grid[x][y - 1] == 0) retVal++;
		if (x < gridSize - 1 && grid[x + 1][y] == 0) retVal++;
		if (y < gridSize - 1 && grid[x][y + 1] == 0) retVal++;
		return retVal;
	}

public:
	Tile()
	{
		grid = bbe::Grid<int32_t>(gridSize, gridSize);
		grid.fill(1);
		for (int32_t i = gridSize / 2; i < gridSize; i++)
		{
			grid[gridSize / 2][i] = 0;
		}
		grid[gridSize / 2][gridSize / 2] = -1;
	}

	Tile(bbe::Random& rand)
	{
		int32_t t = 0;
		int32_t deepest = 0;
	start:
		int32_t depth = 0;
		t++;
		BBELOGLN("Try: " << t << " Depth: " << deepest);
		grid = bbe::Grid<int32_t>(gridSize, gridSize);
		grid.fill(1);
		grid[gridSize / 2][0] = 0;
		size_t currentX = gridSize / 2;
		size_t currentY = 1;
		while (true)
		{
			depth++;
			if (depth > deepest) deepest = depth;
			grid[currentX][currentY] = 0;
			if (currentX == gridSize / 2 && currentY == gridSize - 2)
			{
				grid[currentX][currentY + 1] = 0;
				goto success;
			}
			if (currentX == gridSize - 2 && currentY == gridSize / 2)
			{
				grid[currentX + 1][currentY] = 0;
				goto success;
			}
			if (currentX == 1 && currentY == gridSize / 2)
			{
				grid[currentX - 1][currentY] = 0;
				goto success;
			}

			bbe::List<Direction> possibleDirections;
			
			if (currentX > 1            && getCrossCheck(currentX - 1, currentY) == 1) possibleDirections.add(Direction::LEFT);
			if (currentX < gridSize - 2 && getCrossCheck(currentX + 1, currentY) == 1) possibleDirections.add(Direction::RIGHT);
			if (currentY > 1            && getCrossCheck(currentX, currentY - 1) == 1) possibleDirections.add(Direction::UP);
			if (currentY < gridSize - 2 && getCrossCheck(currentX, currentY + 1) == 1) possibleDirections.add(Direction::DOWN);

			if (possibleDirections.getLength() == 0) goto start;

			const Direction dir = possibleDirections[rand.randomInt(possibleDirections.getLength())];
			if (dir == Direction::UP)
			{
				currentY--;
			}
			else if (dir == Direction::DOWN)
			{
				currentY++;
			}
			else if (dir == Direction::LEFT)
			{
				currentX--;
			}
			else if (dir == Direction::RIGHT)
			{
				currentX++;
			}
			else
			{
				bbe::Crash(bbe::Error::IllegalState, "Illegal Direction");
			}
		}
	success:
		return;
	}

	int32_t getHeight(int32_t x, int32_t y) const
	{
		return grid[x][y];
	}

	int32_t getQuality() const
	{
		return grid.count(0);
	}

	bool exitsAlign(Entrance& exit)
	{
		if (exit.dir == Direction::UP)    return grid[gridSize / 2][gridSize - 1] == 0;
		if (exit.dir == Direction::DOWN)  return grid[gridSize / 2][0] == 0;
		if (exit.dir == Direction::LEFT)  return grid[gridSize - 1][gridSize / 2] == 0;
		if (exit.dir == Direction::RIGHT) return grid[0][gridSize / 2] == 0;
		bbe::Crash(bbe::Error::IllegalState, "Illegal Direction in exitsAlign");
	}

	void rotate(Entrance& exit)
	{
		do
		{
			grid = grid.rotated();
		} while (!exitsAlign(exit));
	}

	void draw(bbe::PrimitiveBrush2D& brush, const bbe::EndlessGrid<int32_t>& map, const bbe::Vector2i& tileOffset, float baseX, float baseY)
	{
		for (int32_t x = 0; x < Tile::gridSize; x++)
		{
			for (int32_t y = 0; y < Tile::gridSize; y++)
			{
				brush.setColorRGB(heightToColor(mergeTiles(getHeight(x, y), map.observe(tileOffset.x + x, tileOffset.y + y))));
				brush.fillRect(x * 25 + baseX, y * 25 + baseY, 25, 25);
			}
		}
	}

	static Tile newTile(bbe::Random& rand, int32_t repeats)
	{
		Tile candidate(rand);
		for (int32_t i = 0; i < repeats; i++)
		{
			Tile contender(rand);
			if (contender.getQuality() > candidate.getQuality())
			{
				candidate = contender;
			}
		}
		return candidate;
	}

	bbe::Grid<int32_t>& getGrid()
	{
		return grid;
	}

	Entrance getEntrance(Entrance& exit)
	{
		if (grid[0           ][gridSize / 2] == 0 && exit.dir != Direction::RIGHT) return Entrance{ {0,            gridSize / 2}, Direction::LEFT };
		if (grid[gridSize - 1][gridSize / 2] == 0 && exit.dir != Direction::LEFT)  return Entrance{ {gridSize - 1, gridSize / 2}, Direction::RIGHT };
		if (grid[gridSize / 2][0           ] == 0 && exit.dir != Direction::DOWN)  return Entrance{ {gridSize / 2, 0,          }, Direction::UP };
		if (grid[gridSize / 2][gridSize - 1] == 0 && exit.dir != Direction::UP)    return Entrance{ {gridSize / 2, gridSize - 1}, Direction::DOWN };
		bbe::Crash(bbe::Error::IllegalState, "Entrance mismatch!");
	}

	void addToPath(const bbe::Vector2i& base, Entrance& entrance)
	{
		bbe::Vector2i pos;
		if (entrance.dir == Direction::UNKNOWN) pos = bbe::Vector2i(gridSize / 2, gridSize / 2);
		if (entrance.dir == Direction::UP   ) pos = bbe::Vector2i(gridSize / 2, gridSize - 1);
		if (entrance.dir == Direction::DOWN) pos = bbe::Vector2i(gridSize / 2, 0);
		if (entrance.dir == Direction::RIGHT   ) pos = bbe::Vector2i(0           , gridSize / 2);
		if (entrance.dir == Direction::LEFT) pos = bbe::Vector2i(gridSize - 1, gridSize / 2);

		bbe::Grid<bool> visited(gridSize, gridSize);
		while (true)
		{
			visited[pos.x][pos.y] = true;
			path.add(pos + base);

			     if (pos.x > 0            && grid[pos.x - 1][pos.y] == 0 && !visited[pos.x - 1][pos.y]) pos.x--;
			else if (pos.x < gridSize - 1 && grid[pos.x + 1][pos.y] == 0 && !visited[pos.x + 1][pos.y]) pos.x++;
			else if (pos.y > 0            && grid[pos.x][pos.y - 1] == 0 && !visited[pos.x][pos.y - 1]) pos.y--;
			else if (pos.y < gridSize - 1 && grid[pos.x][pos.y + 1] == 0 && !visited[pos.x][pos.y + 1]) pos.y++;
			else break;
		}
	}
};

class Enemy
{
public:
	int32_t distanceToGoal = 0;

	Enemy(int32_t distanceToGoal) : distanceToGoal(distanceToGoal - 1) {}

	bbe::Vector2i getPosition() const
	{
		const int32_t baseIndex = distanceToGoal / 1000;
		const int32_t subIndex  = distanceToGoal - baseIndex * 1000;

		const bbe::Vector2i p1 = path[baseIndex];
		const bbe::Vector2i p2 = path[baseIndex + 1];
		const bbe::Vector2i d = p1 - p2;

		return p1 * 1000 + d * subIndex;
	}

	void tick()
	{
		distanceToGoal -= 15;
		if (distanceToGoal < 0) distanceToGoal = 0;
	}
};

class MyGame : public bbe::Game
{
	bbe::EndlessGrid<int32_t> map;
	Tile currentTile;
	bbe::Random rand;
	bbe::Vector2i tileOffset;
	Tile::Entrance previousEntrance = Tile::Entrance{ {}, Direction::UNKNOWN };
	bbe::Vector2 cameraOffset;

	void createTile()
	{
		currentTile = Tile::newTile(rand, 4);
		currentTile.rotate(previousEntrance);
	}

	void applyTile()
	{
		currentTile.addToPath(tileOffset, previousEntrance);
		for (int32_t x = 0; x < Tile::gridSize; x++)
		{
			for (int32_t y = 0; y < Tile::gridSize; y++)
			{
				const int32_t oldValue = map.observe(x + tileOffset.x, y + tileOffset.y);
				map[x + tileOffset.x][y + tileOffset.y] = mergeTiles(oldValue, currentTile.getGrid()[x][y]);
			}
		}
		previousEntrance = currentTile.getEntrance(previousEntrance);
		tileOffset += previousEntrance.position;
		     if (previousEntrance.dir == Direction::RIGHT) { tileOffset.x += 1;                       tileOffset.y += -Tile::gridSize / 2 + 1; }
		else if (previousEntrance.dir == Direction::LEFT)  { tileOffset.x += -Tile::gridSize;         tileOffset.y += -Tile::gridSize / 2 + 1; }
		else if (previousEntrance.dir == Direction::DOWN)  { tileOffset.x += -Tile::gridSize / 2 + 1; tileOffset.y += 1; }
		else if (previousEntrance.dir == Direction::UP)    { tileOffset.x += -Tile::gridSize / 2 + 1; tileOffset.y += -Tile::gridSize; }
		else { bbe::Crash(bbe::Error::IllegalState, "Illegal Entrance in applyTile"); }
	}

	virtual void onStart() override
	{
		map.setDefaultValue(-1000);
		currentTile = Tile();
		applyTile();
		createTile();
	}
	virtual void update(float timeSinceLastFrame) override
	{
		if (isKeyTyped(bbe::Key::SPACE))
		{
			applyTile();
			createTile();
		}
		if (getMouseScrollY() != 0)
		{
			currentTile.rotate(previousEntrance);
		}

		constexpr float camSpeed = 1000;

		if (isKeyDown(bbe::Key::W)) cameraOffset.y += camSpeed * timeSinceLastFrame;
		if (isKeyDown(bbe::Key::S)) cameraOffset.y -= camSpeed * timeSinceLastFrame;
		if (isKeyDown(bbe::Key::A)) cameraOffset.x += camSpeed * timeSinceLastFrame;
		if (isKeyDown(bbe::Key::D)) cameraOffset.x -= camSpeed * timeSinceLastFrame;
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		for (int32_t i = -100; i < 100; i++)
		{
			for (int32_t k = -100; k < 100; k++)
			{
				brush.setColorRGB(heightToColor(map.observe(i, k)));
				brush.fillRect(i * 25 + cameraOffset.x, k * 25 + cameraOffset.y, 25, 25);
			}
		}
		currentTile.draw(brush, map, tileOffset, tileOffset.x * 25 + cameraOffset.x, tileOffset.y * 25 + cameraOffset.y);

		brush.setColorRGB(1, 1, 1);
		for (size_t i = 1; i < path.getLength(); i++)
		{
			brush.fillLine(path[i].as<float>() * 25 + cameraOffset + bbe::Vector2(12.5f), path[i - 1].as<float>() * 25 + cameraOffset + bbe::Vector2(12.5f));
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "ExampleTowerDefense");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
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

class Block
{
private:
	int32_t height = -1000;

public:
	Block() = default;
	explicit Block(int32_t height) : height(height) {}

	int32_t getHeight() const
	{
		return height;
	}
};

int32_t mergeHeights(int32_t a, int32_t b)
{
	if (a == -1 || b == -1) return -1;
	if (a == 0 || b == 0) return 0;
	return a > b ? a : b;
}

Block mergeBlocks(const Block& a, const Block& b)
{
	return Block(mergeHeights(a.getHeight(), b.getHeight()));
}

struct PathElement
{
	bbe::Vector2i coords;
	size_t nextElement = 0;
};
bbe::List<PathElement> path;

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

	bool isBorderCoord(size_t x, size_t y) const
	{
		if (x == 0) return true;
		if (y == 0) return true;
		if (x == gridSize - 1) return true;
		if (y == gridSize - 1) return true;

		return false;
	}

	bool isExitCoord(size_t x, size_t y) const
	{
		if (!isBorderCoord(x, y)) return false;
		if (x == 0 && y == gridSize / 2) return true;
		if (y == 0 && x == gridSize / 2) return true;
		if (x == gridSize - 1 && y == gridSize / 2) return true;
		if (y == gridSize - 1 && x == gridSize / 2) return true;
		return false;
	}

	int32_t getPathNeighbors(size_t x, size_t y) const
	{
		int32_t neighbors = 0;
		if (x > 0 && grid[x - 1][y] == 0) neighbors++;
		if (y > 0 && grid[x][y - 1] == 0) neighbors++;
		if (x < gridSize - 1 && grid[x + 1][y] == 0) neighbors++;
		if (y < gridSize - 1 && grid[x][y + 1] == 0) neighbors++;
		return neighbors;
	}
	
	bool isPath(size_t x, size_t y) const
	{
		return grid[x][y] == 0;
	}

	bool isValidCoord(size_t x, size_t y) const
	{
		return x < gridSize && y < gridSize;
	}

	bool isCandidateForPath(size_t x, size_t y) const
	{
		if (!isValidCoord(x, y)) return false;
		if (isPath(x, y)) return false;
		if (getPathNeighbors(x, y) != 1) return false;
		if (isExitCoord(x, y)) return true;
		if (isBorderCoord(x, y)) return false;
		return true;
	}

	bool expandPathFrom(bbe::Random& rand, size_t x, size_t y)
	{
		bbe::Vector2i walkerPos(x, y);
		while (true)
		{
			grid[walkerPos] = 0;
			if (isExitCoord(walkerPos.x, walkerPos.y))
			{
				// Success!
				return true;
			}

			bbe::List<Direction> validDirections;
			if (isCandidateForPath(walkerPos.x - 1, walkerPos.y)) validDirections.add(Direction::LEFT);
			if (isCandidateForPath(walkerPos.x + 1, walkerPos.y)) validDirections.add(Direction::RIGHT);
			if (isCandidateForPath(walkerPos.x, walkerPos.y - 1)) validDirections.add(Direction::UP);
			if (isCandidateForPath(walkerPos.x, walkerPos.y + 1)) validDirections.add(Direction::DOWN);

			if (validDirections.getLength() == 0)
			{
				// We failed. There is nowhere to go from here.
				return false;
			}

			const Direction dir = validDirections[rand.randomInt(validDirections.getLength())];
			if (dir == Direction::UP)
			{
				walkerPos.y--;
			}
			else if (dir == Direction::DOWN)
			{
				walkerPos.y++;
			}
			else if (dir == Direction::LEFT)
			{
				walkerPos.x--;
			}
			else if (dir == Direction::RIGHT)
			{
				walkerPos.x++;
			}
			else
			{
				bbe::Crash(bbe::Error::IllegalState, "Illegal Direction");
			}
		}
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
		grid = bbe::Grid<int32_t>(gridSize, gridSize);
		while (true) // Keep making new grids until we found a good one.
		{
			grid.fill(1);
			grid[gridSize / 2][0] = 0;
			if (expandPathFrom(rand, gridSize / 2, 1))
			{
				bool twoExits = true;
				if (twoExits)
				{
					const auto startingGrid = grid; // Make a copy of the grid we had so far so that we can always roll back.
					
					// Some grids might be so full that we can't put another path, so we only try for a few times before giving up and trying with another tile.
					for (int32_t breakOut = 0; breakOut < 10; breakOut++)
					{
						// Figure out a random starting location that has one neighbor.
						size_t posX = 0;
						size_t posY = 0;
						while (true)
						{
							posX = 1 + rand.randomInt(gridSize - 2);
							posY = 1 + rand.randomInt(gridSize - 2);
							if (getPathNeighbors(posX, posY) == 1) break;
						}

						// Expand from that location.
						if (expandPathFrom(rand, posX, posY))
						{
							// Success!
							return;
						}
						else
						{
							// Fail :( Rollback
							grid = startingGrid;
						}
					}
				}
				else
				{
					// Success!
					return;
				}
			}
		}
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

	void draw(bbe::PrimitiveBrush2D& brush, const bbe::EndlessGrid<Block>& map, const bbe::Vector2i& tileOffset, float baseX, float baseY)
	{
		for (int32_t x = 0; x < Tile::gridSize; x++)
		{
			for (int32_t y = 0; y < Tile::gridSize; y++)
			{
				brush.setColorRGB(heightToColor(mergeHeights(getHeight(x, y), map.observe(tileOffset.x + x, tileOffset.y + y).getHeight())));
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
		if (entrance.dir == Direction::UP     ) pos = bbe::Vector2i(gridSize / 2, gridSize - 1);
		if (entrance.dir == Direction::DOWN   ) pos = bbe::Vector2i(gridSize / 2, 0);
		if (entrance.dir == Direction::RIGHT  ) pos = bbe::Vector2i(0           , gridSize / 2);
		if (entrance.dir == Direction::LEFT   ) pos = bbe::Vector2i(gridSize - 1, gridSize / 2);

		bbe::Grid<bool> visited(gridSize, gridSize);
		while (true)
		{
			visited[pos.x][pos.y] = true;
			PathElement newElement;
			newElement.coords = pos + base;
			newElement.nextElement = path.getLength() > 0 ? path.getLength() - 1 : 0;
			path.add(newElement);

			     if (pos.x > 0            && grid[pos.x - 1][pos.y] == 0 && !visited[pos.x - 1][pos.y]) pos.x--;
			else if (pos.x < gridSize - 1 && grid[pos.x + 1][pos.y] == 0 && !visited[pos.x + 1][pos.y]) pos.x++;
			else if (pos.y > 0            && grid[pos.x][pos.y - 1] == 0 && !visited[pos.x][pos.y - 1]) pos.y--;
			else if (pos.y < gridSize - 1 && grid[pos.x][pos.y + 1] == 0 && !visited[pos.x][pos.y + 1]) pos.y++;
			else break;
		}
	}
};

constexpr int32_t UNITS_PER_GRID_TILE = 1000;

class Enemy
{
public:
	int32_t currentPathIndex = 0;
	int32_t pathElementTraveled = 0;
	int32_t hue = 0;

	Enemy() : currentPathIndex(path.getLength() - 1)
	{
		static int32_t nextHue = 0;
		hue = nextHue;
		nextHue += 90;
	}

	bbe::Vector2i getPosition() const
	{
		if (currentPathIndex == 0 && pathElementTraveled >= UNITS_PER_GRID_TILE)
		{
			return path[0].coords;
		}

		const bbe::Vector2i p1 = path[currentPathIndex].coords;
		const bbe::Vector2i p2 = path[path[currentPathIndex].nextElement].coords;
		const bbe::Vector2i d = p2 - p1;

		return p1 * UNITS_PER_GRID_TILE + d * pathElementTraveled;
	}

	void tick()
	{
		pathElementTraveled += 15;
		while (currentPathIndex != 0 && pathElementTraveled >= UNITS_PER_GRID_TILE)
		{
			currentPathIndex = path[currentPathIndex].nextElement;
			pathElementTraveled -= UNITS_PER_GRID_TILE;
		}
	}

	bool shouldDelete()
	{
		return currentPathIndex == 0 && pathElementTraveled >= UNITS_PER_GRID_TILE;
	}
};

class MyGame : public bbe::Game
{
	bbe::EndlessGrid<Block> map;
	Tile currentTile;
	bbe::Random rand;
	bbe::Vector2i tileOffset;
	Tile::Entrance previousEntrance = Tile::Entrance{ {}, Direction::UNKNOWN };
	bbe::Vector2 cameraOffset;

	bbe::List<Enemy> enemies;
	int32_t ticksSinceLastEnemySpawn = 0;

	float timeSinceLastTick = 0.0f;
	int tickMultiplier = 1;

	void tick()
	{
		ticksSinceLastEnemySpawn++;
		if (ticksSinceLastEnemySpawn > 120)
		{
			ticksSinceLastEnemySpawn = 0;
			enemies.add(Enemy());
		}

		for (size_t i = 0; i < enemies.getLength(); i++)
		{
			enemies[i].tick();
		}

		for (size_t i = 0; i < enemies.getLength(); i++)
		{
			if (enemies[i].shouldDelete())
			{
				enemies.removeIndex(i);
				i--;
			}
		}
	}

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
				const Block oldValue = map.observe(x + tileOffset.x, y + tileOffset.y);
				map[x + tileOffset.x][y + tileOffset.y] = mergeBlocks(oldValue, Block(currentTile.getGrid()[x][y]));
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
		map.setDefaultValue(Block(-1000));
		currentTile = Tile();
		applyTile();
		createTile();
	}
	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << "FPS: " << (1 / timeSinceLastFrame) << std::endl;
		timeSinceLastTick += timeSinceLastFrame;
		constexpr float tickTime = 1.0f / 120.f;
		while (timeSinceLastTick > tickTime)
		{
			timeSinceLastTick -= tickTime;
			for(int i = 0; i < tickMultiplier; i++) tick();
		}

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
				brush.setColorRGB(heightToColor(map.observe(i, k).getHeight()));
				brush.fillRect(i * 25 + cameraOffset.x, k * 25 + cameraOffset.y, 25, 25);
			}
		}
		currentTile.draw(brush, map, tileOffset, tileOffset.x * 25 + cameraOffset.x, tileOffset.y * 25 + cameraOffset.y);

		brush.setColorRGB(1, 1, 1);
		for (size_t i = 1; i < path.getLength(); i++)
		{
			brush.fillLine(path[i].coords.as<float>() * 25 + cameraOffset + bbe::Vector2(12.5f), path[path[i].nextElement].coords.as<float>() * 25 + cameraOffset + bbe::Vector2(12.5f));
		}

		for (size_t i = 0; i < enemies.getLength(); i++)
		{
			brush.setColorHSV(enemies[i].hue, 0.5f, 1.0f);
			bbe::Vector2 pos = enemies[i].getPosition().as<float>() / UNITS_PER_GRID_TILE * 25;
			brush.fillRect(pos + cameraOffset - bbe::Vector2(5) + bbe::Vector2(25 / 2.f), 10, 10);
		}

		ImGui::InputInt("Tick Multiplier", &tickMultiplier);
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
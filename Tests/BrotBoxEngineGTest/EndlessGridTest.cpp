#include "gtest/gtest.h"
#include "TestUtils.h"
#include "BBE/EndlessGrid.h"
#include "BBE/Random.h"

TEST(EndlessGrid, SimpleChecks)
{
	bbe::EndlessGrid<int32_t> grid;
	ASSERT_EQ(grid[0][0], 0);
	grid[0][0] = 1;
	ASSERT_EQ(grid[0][0], 1);
	grid[ 1][ 0] = 2;
	grid[ 1][ 1] = 3;
	grid[ 0][ 1] = 4;
	grid[-1][ 1] = 5;
	grid[-1][ 0] = 6;
	grid[-1][-1] = 7;
	grid[ 0][-1] = 8;
	grid[ 1][-1] = 9;

	ASSERT_EQ(grid[ 0][ 0], 1);
	ASSERT_EQ(grid[ 1][ 0], 2);
	ASSERT_EQ(grid[ 1][ 1], 3);
	ASSERT_EQ(grid[ 0][ 1], 4);
	ASSERT_EQ(grid[-1][ 1], 5);
	ASSERT_EQ(grid[-1][ 0], 6);
	ASSERT_EQ(grid[-1][-1], 7);
	ASSERT_EQ(grid[ 0][-1], 8);
	ASSERT_EQ(grid[ 1][-1], 9);
}

TEST(EndlessGrid, RandomChecks)
{
	bbe::Random rand;
	rand.setSeed(17); // Repeatability
	for (int repeats = 0; repeats < 16; repeats++)
	{
		// TODO: Sloooow!
		constexpr int64_t size = 1024;
		bbe::Grid<int32_t> checkGrid(size, size);
		bbe::EndlessGrid<int32_t> grid;

		for (int i = 0; i < 16; i++)
		{
			const int64_t x = rand.randomInt(size) - size / 2;
			const int64_t y = rand.randomInt(size) - size / 2;
			checkGrid[x + size / 2][y + size / 2] = i + 1;
			grid[x][y] = i + 1;
		}

		for (int64_t x = 0; x < size; x++)
		{
			for (int64_t y = 0; y < size; y++)
			{
				if (checkGrid[x][y] != 0)
				{
					ASSERT_EQ(checkGrid[x][y], grid[x - size / 2][y - size / 2]);
				}
			}
		}
	}
}

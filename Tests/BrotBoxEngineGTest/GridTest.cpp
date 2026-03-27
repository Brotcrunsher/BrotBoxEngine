#include "gtest/gtest.h"
#include "BBE/BrotBoxEngine.h"
#include <limits>

TEST(Grid, NegativeDimensionsDie)
{
	ASSERT_DEATH((void)bbe::Grid<int>(bbe::Vector2i(-1, 4)), ".*");
	ASSERT_DEATH((void)bbe::Grid<int>(bbe::Vector2i(4, -1)), ".*");
}

TEST(Grid, OversizedDimensionsDie)
{
	ASSERT_DEATH((void)bbe::Grid<int>(std::numeric_limits<size_t>::max(), 2), ".*");
}

TEST(Grid, EmptyInitializerListCreatesEmptyGrid)
{
	bbe::Grid<int> grid({});
	ASSERT_EQ(grid.getWidth(), 0u);
	ASSERT_EQ(grid.getHeight(), 0u);
}

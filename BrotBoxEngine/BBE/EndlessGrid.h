#pragma once

#include "../BBE/Grid.h"
#include "../BBE/Vector2.h"

namespace bbe
{
	template<typename T> class EndlessGrid;


	template<typename T>
	class EndlessGridColProxy
	{
	private:
		bbe::EndlessGrid<T>& m_parent;
		int64_t m_col = 0;

	public:
		EndlessGridColProxy(bbe::EndlessGrid<T>& parent, int64_t col)
			: m_parent(parent), m_col(col)
		{}

		T& operator[](int64_t y)
		{
			return m_parent.get(m_col, y);
		}
	};

	template<typename T>
	class EndlessGrid
	{
	private:
		bbe::Vector2i64 m_offset;
		bbe::Grid<T> m_grid;

		void growIfNeeded(int64_t x_, int64_t y_)
		{
			const auto startCoord = toNativeCoord(x_, y_);
			if(    startCoord.x >= 0 
				&& startCoord.y >= 0 
				&& startCoord.x < m_grid.getWidth() 
				&& startCoord.y < m_grid.getHeight())
			{
				// No need to grow!
				return;
			}
			// TODO: Double size to minimize allocations. Careful though! It should extend the range in the negative indizes roughly (or exactly?) equally.
			const int64_t growWidth  = bbe::Math::distanceToRange<int64_t>(0, m_grid.getWidth (), startCoord.x);
			const int64_t growHeight = bbe::Math::distanceToRange<int64_t>(0, m_grid.getHeight(), startCoord.y);

			const int64_t newGridOffsetX = startCoord.x < 0 ? startCoord.x : 0;
			const int64_t newGridOffsetY = startCoord.y < 0 ? startCoord.y : 0;

			bbe::Grid<T> newGrid = bbe::Grid<T>(m_grid.getWidth() + growWidth, m_grid.getHeight() + growHeight);
			for (int64_t x = 0; x < m_grid.getWidth(); x++)
			{
				for (int64_t y = 0; y < m_grid.getHeight(); y++)
				{
					newGrid[x - newGridOffsetX][y - newGridOffsetY] = m_grid[x][y];
				}
			}
			m_grid = std::move(newGrid);

			m_offset.x += newGridOffsetX;
			m_offset.y += newGridOffsetY;
		}

		bbe::Vector2i64 toNativeCoord(int64_t x, int64_t y) const
		{
			return { x - m_offset.x, y - m_offset.y };
		}

	public:
		EndlessGrid() = default;

		T& get(int64_t x, int64_t y)
		{
			growIfNeeded(x, y);
			auto index = toNativeCoord(x, y);
			return m_grid[index.x][index.y];
		}

		EndlessGridColProxy<T> operator[](int64_t x)
		{
			return EndlessGridColProxy<T>(*this, x);
		}
	};
}

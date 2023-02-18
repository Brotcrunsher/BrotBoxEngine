#pragma once

#include <initializer_list>
#include "../BBE/List.h"
#include "../BBE/Span.h"
#include "../BBE/Rectangle.h"
#include "../BBE/Vector2.h"

namespace bbe
{
	template<typename T>
	class Grid
	{
	private:
		size_t m_width;
		size_t m_height;
		bbe::List<T> m_pdata;


	public:
		Grid() :
			m_width(0),
			m_height(0),
			m_pdata()
		{}

		Grid(size_t width, size_t height) :
			m_width(width),
			m_height(height),
			m_pdata()
		{
			m_pdata.resizeCapacityAndLength(m_width * m_height);
		}

		Grid(const bbe::Vector2i& dim) :
			m_width(dim.x),
			m_height(dim.y),
			m_pdata()
		{
			m_pdata.resizeCapacityAndLength(m_width * m_height);
		}

		/*nonexplicit*/ Grid(const std::initializer_list<std::initializer_list<T>>& il)
		{
			m_width = il.begin()->size();
			m_height = il.size();
			m_pdata.resizeCapacityAndLength(m_width * m_height);

			size_t x = 0;
			size_t y = 0;
			for (auto iter = il.begin(); iter != il.end(); iter++, y++)
			{
				auto il2 = *iter;
				if (il2.size() != m_width)
				{
					// One of the rows seems to have a different length than the others.
					debugBreak();
				}
				x = 0;
				for (auto iter2 = il2.begin(); iter2 != il2.end(); iter2++, x++)
				{
					operator[](x)[y] = *iter2;
				}
			}
		}

		bbe::Span<T> operator[](size_t x)
		{
			if (x >= m_width)
			{
				debugBreak();
			}
			return bbe::Span(m_pdata.getRaw() + x * m_height, m_height);
		}

		bbe::Span<const T> operator[](size_t x) const
		{
			if (x >= m_width)
			{
				debugBreak();
			}
			return bbe::Span(m_pdata.getRaw() + x * m_height, m_height);
		}

		T& operator[](const bbe::Vector2i& access)
		{
			return operator[](access.x)[access.y];
		}

		const T& operator[](const bbe::Vector2i& access) const
		{
			return operator[](access.x)[access.y];
		}

		size_t getWidth() const
		{
			return m_width;
		}

		size_t getHeight() const
		{
			return m_height;
		}

		T* getRaw()
		{
			return m_pdata;
		}

		const T* getRaw() const
		{
			return m_pdata;
		}

		bbe::Rectanglei getBiggestRect(const T& value) const
		{
			bbe::Rectanglei retVal;
			bbe::List<size_t> histogram;
			histogram.resizeCapacityAndLength(m_height);

			for (size_t i = 0; i < m_width; i++)
			{
				for (size_t k = 0; k < m_height; k++)
				{
					if (operator[](i)[k] == value)
					{
						histogram[k]++;
					}
					else
					{
						histogram[k] = 0;
					}
				}

				// TODO: This can get a speed up from O(N²) to O(N) using a stack.
				//       See "Largest rectangle in Histogram"
				for (size_t k = 0; k < m_height; k++)
				{
					if (histogram[k] > 0)
					{
						size_t width = 1;
						size_t up = k;

						for (size_t i = k + 1; i < m_height; i++)
						{
							if (histogram[i] >= histogram[k])
							{
								width++;
							}
							else
							{
								break;
							}
						}
						for (size_t i = k - 1; i != (size_t)-1; i--)
						{
							if (histogram[i] >= histogram[k])
							{
								width++;
								up = i;
							}
							else
							{
								break;
							}
						}

						size_t area = histogram[k] * width;
						if (area > retVal.getArea())
						{
							retVal.x = i - histogram[k] + 1;
							retVal.y = up;
							retVal.width  = histogram[k];
							retVal.height = width;
						}
					}
				}
			}

			return retVal;
		}

		void setAll(const T& t)
		{
			for (size_t i = 0; i < m_pdata.getLength(); i++)
			{
				m_pdata[i] = t;
			}
		}

	private:
		void floodFill_(const bbe::Vector2i& pos, const T& from, const T& to, bool fillDiagonal)
		{
			if (pos.x < 0 || pos.x >= getWidth() || pos.y < 0 || pos.y >= getHeight()) return;
			T& found = operator[](pos);
			if (found != from || found == to) return;

			found = to;

			floodFill_({ pos.x + 1, pos.y }, from, to, fillDiagonal);
			floodFill_({ pos.x - 1, pos.y }, from, to, fillDiagonal);
			floodFill_({ pos.x, pos.y + 1 }, from, to, fillDiagonal);
			floodFill_({ pos.x, pos.y - 1 }, from, to, fillDiagonal);
			if (fillDiagonal)
			{
				floodFill_({ pos.x + 1, pos.y + 1 }, from, to, fillDiagonal);
				floodFill_({ pos.x - 1, pos.y + 1 }, from, to, fillDiagonal);
				floodFill_({ pos.x + 1, pos.y - 1 }, from, to, fillDiagonal);
				floodFill_({ pos.x - 1, pos.y - 1 }, from, to, fillDiagonal);
			}
		}
	public:

		void floodFill(const bbe::Vector2i& pos, const T& value, bool fillDiagonal)
		{
			floodFill_(pos, operator[](pos), value, fillDiagonal);
		}
	};
}

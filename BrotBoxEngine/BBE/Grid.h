#pragma once

#include <initializer_list>
#include "../BBE/List.h"
#include "../BBE/Span.h"
#include "../BBE/Rectangle.h"
#include "../BBE/Vector2.h"

namespace bbe
{
	template<typename T>
	class GridIterator_t
	{
	private:
		T current;
		T end;
		bool hasMore = true;

	public:
		GridIterator_t(const bbe::Vector2& current, const bbe::Vector2& end) : current(current), end(end) {}

		bool hasNext() const
		{
			return hasMore;
		}

		bbe::Vector2i next()
		{
			bbe::Vector2i retVal = current.template as<int32_t>();
			if (retVal == end.template as<int32_t>())
			{
				hasMore = false;
				return retVal;
			}
			T dir = (end - current);
			float dist = dir.getLength();
			if (dist <= 1.0f)
			{
				current = end;
				return retVal;
			}
			dir /= dist;
			current += dir;
			if (current.as<int32_t>() == retVal) current += dir;
			return retVal;
		}
	};
	using GridIterator = GridIterator_t<bbe::Vector2>;

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

		bbe::List<bbe::Rectanglei> getAllBiggestRects(const T& value) const
		{
			Grid<T> copy = *this;
			bbe::List<bbe::Rectanglei> retVal;
			for (size_t x = 0; x < getWidth(); x++)
			{
				for (size_t y = 0; y < getHeight(); y++)
				{
					if (copy[x][y] == value)
					{
						bbe::Rectanglei newElement(x, y, 1, 1);
						// Determine width of new Element
						for (size_t wDet = x + 1; wDet < getWidth(); wDet++)
						{
							if (copy[wDet][y] == value) newElement.width++;
							else break;
						}
						// Determine height of new Element
						for (size_t hDet = y + 1; hDet < getHeight(); hDet++)
						{
							for (size_t check = x; check < x + newElement.width; check++)
							{
								if (copy[check][hDet] != value)
								{
									goto outer;
								}
							}
							newElement.height++;
						}
					outer:
						// Remove covered blocks from copy
						for (int32_t x = newElement.x; x < newElement.x + newElement.width; x++)
						{
							for (int32_t y = newElement.y; y < newElement.y + newElement.height; y++)
							{
								copy[x][y] = !value;
							}
						}
						// Add new Element to return
						retVal.add(newElement);
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
	public:

		void floodFill(const bbe::Vector2i& pos, const T& to, bool fillDiagonal)
		{
			const T from = operator[](pos);
			if (from == to) return;
			bbe::List<bbe::Vector2i> posToCheck;
			posToCheck.add(pos);

			while (posToCheck.getLength() > 0)
			{
				const bbe::Vector2i pos = posToCheck.popBack();
				if (pos.x < 0 || pos.x >= getWidth() || pos.y < 0 || pos.y >= getHeight()) continue;
				if (operator[](pos) == from)
				{
					operator[](pos) = to;

					posToCheck.add( bbe::Vector2i(pos.x + 1, pos.y ));
					posToCheck.add( bbe::Vector2i(pos.x - 1, pos.y ));
					posToCheck.add( bbe::Vector2i(pos.x, pos.y + 1 ));
					posToCheck.add( bbe::Vector2i(pos.x, pos.y - 1 ));
					if (fillDiagonal)
					{
						posToCheck.add( bbe::Vector2i(pos.x + 1, pos.y + 1 ));
						posToCheck.add( bbe::Vector2i(pos.x - 1, pos.y + 1 ));
						posToCheck.add( bbe::Vector2i(pos.x + 1, pos.y - 1 ));
						posToCheck.add( bbe::Vector2i(pos.x - 1, pos.y - 1 ));
					}
				}
			}
		}
	};
}

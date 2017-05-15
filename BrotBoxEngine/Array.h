#pragma once

namespace bbe
{
	template <typename T, int SIZE>
	class Array
	{
	private:
		T m_data[SIZE];
	public:
		Array()
		{
			//do nothing
		}

		Array(const Array<T, SIZE>& other)
		{
			//UNTESTED
			for (size_t i = 0; i < SIZE; i++)
			{
				m_data[i] = other[i];
			}
		}

		Array(Array<T, SIZE>&& other)
		{
			//UNTESTED
			for (size_t i = 0; i < SIZE; i++)
			{
				m_data[i] = std::move(other[i]);
			}
		}

		Array& operator=(const Array<T, SIZE>& other)
		{
			//UNTESTED
			for (size_t i = 0; i < SIZE; i++)
			{
				m_data[i] = other[i];
			}
		}

		Array& operator=(Array<T, SIZE>&& other)
		{
			//UNTESTED
			for (size_t i = 0; i < SIZE; i++)
			{
				m_data[i] = std::move(other[i]);
			}
		}

		~Array()
		{
			//do nothing
		}

		T& operator[](size_t index)
		{
			//UNTESTED
			return m_data[index];
		}

		const T& operator[](size_t index) const
		{
			//UNTESTED
			return m_data[index]
		}

		constexpr size_t getLength() const
		{
			//UNTESTED
			return SIZE;
		}

		T* getRaw()
		{
			//UNTESTED
			return m_data;
		}

		const T* getRaw() const
		{
			//UNTESTED
			return m_data;
		}
	};
}
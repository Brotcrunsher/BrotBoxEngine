#pragma once

namespace bbe
{
	template <typename T, int SIZE>
	class Array
	{
	private:
		T data[SIZE];
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
				data[i] = other[i];
			}
		}

		Array(Array<T, SIZE>&& other)
		{
			//UNTESTED
			for (size_t i = 0; i < SIZE; i++)
			{
				data[i] = std::move(other[i]);
			}
		}

		Array& operator=(const Array<T, SIZE>& other)
		{
			//UNTESTED
			for (size_t i = 0; i < SIZE; i++)
			{
				data[i] = other[i];
			}
		}

		Array& operator=(Array<T, SIZE>&& other)
		{
			//UNTESTED
			for (size_t i = 0; i < SIZE; i++)
			{
				data[i] = std::move(other[i]);
			}
		}

		~Array()
		{
			//do nothing
		}

		T& operator[](size_t index)
		{
			//UNTESTED
			return data[index];
		}

		const T& operator[](size_t index) const
		{
			//UNTESTED
			return data[index]
		}

		constexpr size_t getLength() const
		{
			//UNTESTED
			return SIZE;
		}

		T* getRaw()
		{
			//UNTESTED
			return data;
		}

		const T* getRaw() const
		{
			//UNTESTED
			return data;
		}
	};
}
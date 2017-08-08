#pragma once

#include "../BBE/Hash.h"
#include <cassert>
#include <initializer_list>

namespace bbe
{
	template <typename T, int LENGTH>
	class Array
	{
	private:
		T m_data[LENGTH];
	public:
		Array()
		{
			//do nothing
		}

		Array(const std::initializer_list<T> &il)
		{
			//UNTESTED
			assert((il.end() - il.begin()) == LENGTH);
			size_t i = 0;
			for (auto iter = il.begin(); iter != il.end(); iter++) {
				m_data[i] = *iter;
				i++;
			}
		}

		Array(const Array<T, LENGTH>& other)
		{
			for (size_t i = 0; i < LENGTH; i++)
			{
				m_data[i] = other[i];
			}
		}

		Array(Array<T, LENGTH>&& other)
		{
			for (size_t i = 0; i < LENGTH; i++)
			{
				m_data[i] = std::move(other[i]);
			}
		}

		Array& operator=(const Array<T, LENGTH>& other)
		{
			for (size_t i = 0; i < LENGTH; i++)
			{
				m_data[i] = other[i];
			}

			return *this;
		}

		Array& operator=(Array<T, LENGTH>&& other)
		{
			for (size_t i = 0; i < LENGTH; i++)
			{
				m_data[i] = std::move(other[i]);
			}

			return *this;
		}

		~Array()
		{
			//do nothing
		}

		T& operator[](size_t index)
		{
			return m_data[index];
		}

		const T& operator[](size_t index) const
		{
			return m_data[index];
		}

		constexpr size_t getLength() const
		{
			return LENGTH;
		}

		T* getRaw()
		{
			return m_data;
		}

		const T* getRaw() const
		{
			return m_data;
		}
	};

	template<typename T, int LENGTH>
	uint32_t hash(const Array<T, LENGTH> &t)
	{
		size_t length = t.getLength();
		if (length > 16)
		{
			length = 16;
		}

		uint32_t _hash = 0;

		for (int i = 0; i < length; i++)
		{
			_hash += hash(t[i]);
		}

		return _hash;
	}

}

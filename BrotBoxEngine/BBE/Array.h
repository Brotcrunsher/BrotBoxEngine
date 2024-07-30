#pragma once

#include "../BBE/Hash.h"
#include <cassert>
#include <initializer_list>
#include <utility>

namespace bbe
{
	template <typename T, int LENGTH>
	class Array
	{
	private:
		T m_pdata[LENGTH] = {};
	public:
		Array()
		{
			//do nothing
		}

		/*nonexplicit*/ Array(const std::initializer_list<T> &il)
		{
			//UNTESTED
			assert((il.end() - il.begin()) == LENGTH);
			std::size_t i = 0;
			for (auto iter = il.begin(); iter != il.end(); iter++) {
				m_pdata[i] = *iter;
				i++;
			}
		}

		Array(const Array<T, LENGTH>& other)
		{
			for (std::size_t i = 0; i < LENGTH; i++)
			{
				m_pdata[i] = other[i];
			}
		}

		Array(Array<T, LENGTH>&& other)
		{
			for (std::size_t i = 0; i < LENGTH; i++)
			{
				m_pdata[i] = std::move(other[i]);
			}
		}

		Array& operator=(const Array<T, LENGTH>& other)
		{
			for (std::size_t i = 0; i < LENGTH; i++)
			{
				m_pdata[i] = other[i];
			}

			return *this;
		}

		Array& operator=(Array<T, LENGTH>&& other)
		{
			for (std::size_t i = 0; i < LENGTH; i++)
			{
				m_pdata[i] = std::move(other[i]);
			}

			return *this;
		}

		~Array()
		{
			//do nothing
		}

		T& operator[](std::size_t index)
		{
			return m_pdata[index];
		}

		const T& operator[](std::size_t index) const
		{
			return m_pdata[index];
		}

		constexpr std::size_t getLength() const
		{
			return LENGTH;
		}

		T* getRaw()
		{
			return m_pdata;
		}

		const T* getRaw() const
		{
			return m_pdata;
		}

		T* begin()
		{
			//UNTESTED
			return &(this->m_pdata[0]);
		}

		const T* begin() const
		{
			//UNTESTED
			return &(this->m_pdata[0]);
		}

		T* end()
		{
			//UNTESTED
			return &(this->m_pdata[getLength()]);
		}

		const T* end() const
		{
			//UNTESTED
			return &(this->m_pdata[getLength()]);
		}
	};

	template<typename T, int LENGTH>
	uint32_t hash(const Array<T, LENGTH> &t)
	{
		std::size_t length = t.getLength();
		if (length > 16)
		{
			length = 16;
		}

		uint32_t _hash = 0;

		for (std::size_t i = 0; i < length; i++)
		{
			_hash += hash(t[i]);
		}

		return _hash;
	}

}

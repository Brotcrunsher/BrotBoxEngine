#pragma once

#include "../BBE/Array.h"
#include "../BBE/Hash.h"
#include "../BBE/Error.h"
#include "../BBE/AllocBlock.h"
#include <type_traits>
#include <stddef.h>
#include <initializer_list>

namespace bbe
{
	template<typename T>
	class List;

	// TODO: I think Dynamic Array should be removed. The only advantage compared to a List is that it has no capacity. The disadvantage is that code gets more messy when
	//       parts take a list and parts take a dynamic array.
	template <typename T>
	class DynamicArray
	{
	private:
		T*         m_pdata;
		size_t     m_length;
		AllocBlock m_allocBlock;

		void createArray(std::size_t size)
		{
			m_length = size;
			m_allocBlock = bbe::allocateBlock(sizeof(T) * size);
			m_pdata = new (m_allocBlock.data) T[size];
		}

		void deleteArray()
		{
			for (size_t i = 0; i < m_length; i++)
			{
				m_pdata[i].~T();
			}
			bbe::freeBlock(m_allocBlock);
			m_length = 0;
		}

	public:
		DynamicArray() :
			m_pdata(nullptr),
			m_length(0)
		{

		}

		DynamicArray(std::size_t size)
			: m_length(size)
		{
			createArray(size);
		}

		template <typename U, int size>
		DynamicArray(const Array<U, size> &arr)
			: m_length(size)
		{
			createArray(arr.getLength());
			for (int i = 0; i < size; i++)
			{
				m_pdata[i] = arr[i];
			}
		}

		DynamicArray(const List<T> &list)
			: m_length(list.getLength())
		{
			createArray(list.getLength());
			for (std::size_t i = 0; i < m_length; i++)
			{
				m_pdata[i] = list[i];
			}
		}

		explicit DynamicArray(const std::initializer_list<T> &il)
		{
			createArray(il.end() - il.begin());
			std::size_t i = 0;
			for (auto iter = il.begin(); iter != il.end(); iter++) {
				m_pdata[i] = *iter;
				i++;
			}
		}

		~DynamicArray()
		{
			deleteArray();
		}

		DynamicArray(const DynamicArray&  other) //Copy Constructor
			: m_length(other.m_length)
		{
			createArray(other.m_length);
			for (std::size_t i = 0; i < m_length; i++)
			{
				m_pdata[i] = other[i];
			}
		}
		DynamicArray(DynamicArray&& other) noexcept //Move Constructor
			: m_pdata(other.m_pdata)
		{
			m_length = other.m_length;
			m_allocBlock = other.m_allocBlock;
			other.m_pdata = nullptr;
			other.m_length = 0;
			other.m_allocBlock = {};
		}
		DynamicArray& operator=(const DynamicArray&  other)  //Copy Assignment
		{
			deleteArray();

			createArray(other.m_length);
			for (std::size_t i = 0; i < m_length; i++)
			{
				m_pdata[i] = other[i];
			}

			return *this;
		}
		DynamicArray& operator=(DynamicArray&& other) noexcept //Move Assignment
		{
			deleteArray();

			m_pdata = other.m_pdata;
			m_length = other.m_length;
			m_allocBlock = other.m_allocBlock;
			other.m_pdata = nullptr;
			other.m_length = 0;
			other.m_allocBlock = {};

			return *this;
		}

		T& operator[](std::size_t index)
		{
			if (index < 0 || index >= m_length)
			{
				bbe::Crash(bbe::Error::IllegalIndex);
			}
			return m_pdata[index];
		}

		const T& operator[](std::size_t index) const
		{
			if (index < 0 || index >= m_length)
			{
				bbe::Crash(bbe::Error::IllegalIndex);
			}
			return m_pdata[index];
		}

		std::size_t getLength() const
		{
			return m_length;
		}

		T* getRaw()
		{
			return m_pdata;
		}

		const T* getRaw() const
		{
			return m_pdata;
		}

		T* begin() { return m_pdata; };
		const T* begin() const { return m_pdata; };
		T* end() { return &m_pdata[m_length]; };
		const T* end() const { return &m_pdata[m_length]; };



		T& first()
		{
			return (m_pdata[0]);
		}

		const T& first() const
		{
			return (m_pdata[0]);
		}

		T& last()
		{
			return (m_pdata[m_length - 1]);
		}

		const T& last() const
		{
			return (m_pdata[m_length - 1]);
		}
	};

	template<typename T>
	uint32_t hash(const DynamicArray<T> &t)
	{
		uint32_t _hash = 0;

		for (std::size_t i = 0; i < t.getLength(); i++)
		{
			_hash += hash(t[i]);
		}

		return _hash;
	}
}

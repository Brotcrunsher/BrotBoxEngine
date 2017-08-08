#pragma once

#include "../BBE/Array.h"
#include "../BBE/Hash.h"
#include "../BBE/NewDeleteAllocator.h"
#include <type_traits>
#include <cassert>
#include <initializer_list>

namespace bbe
{
	template<typename T, bool keepSorted>
	class List;

	template <typename T, typename Allocator = NewDeleteAllocator, typename PointerType = T*>
	class DynamicArray
	{
	private:

		PointerType m_data;
		size_t m_length;
		Allocator* m_pparentAllocator = nullptr;

		void createArray(size_t size, Allocator* parentAllocator)
		{
			m_length = size;
			if (std::is_same<Allocator, NewDeleteAllocator>::value)
			{
				m_data = new T[size];
			}
			else
			{
				assert(parentAllocator != nullptr);
				m_data = parentAllocator->allocateObjects<T>(size);
				m_pparentAllocator = parentAllocator;
			}
		}

		void deleteArray()
		{
			if (m_data != nullptr)
			{
				if (std::is_same<Allocator, NewDeleteAllocator>::value)
				{
					delete[] m_data;
				}
				else
				{
					m_pparentAllocator->deallocate(m_data);
				}

				m_length = 0;
				m_pparentAllocator = nullptr;
			}
		}

	public:
		DynamicArray(size_t size, Allocator* parentAllocator = nullptr)
			: m_length(size)
		{
			createArray(size, parentAllocator);
		}

		template <typename U, int size>
		DynamicArray(const Array<U, size> &arr, Allocator* parentAllocator = nullptr)
			: m_length(size)
		{
			createArray(arr.getLength(), parentAllocator);
			for (int i = 0; i < size; i++)
			{
				m_data[i] = arr[i];
			}
		}

		DynamicArray(const List<T, true> &list, Allocator* parentAllocator = nullptr)
			: m_length(list.getLength())
		{
			createArray(list.getLength(), parentAllocator);
			for (int i = 0; i < m_length; i++)
			{
				m_data[i] = list[i];
			}
		}

		DynamicArray(const List<T, false> &list, Allocator* parentAllocator = nullptr)
			: m_length(list.getLength())
		{
			createArray(list.getLength(), parentAllocator);
			for (int i = 0; i < m_length; i++)
			{
				m_data[i] = list[i];
			}
		}

		DynamicArray(const std::initializer_list<T> &il)
		{
			createArray(il.end() - il.begin(), nullptr);
			size_t i = 0;
			for (auto iter = il.begin(); iter != il.end(); iter++) {
				m_data[i] = *iter;
				i++;
			}
		}

		~DynamicArray()
		{
			deleteArray();
		}

		DynamicArray(const DynamicArray&  other) //Copy Constructor
			: m_length(other.m_length), m_pparentAllocator(other.m_pparentAllocator)
		{
			createArray(other.m_length, other.m_pparentAllocator);
			for (size_t i = 0; i < m_length; i++)
			{
				m_data[i] = other[i];
			}
		}
		DynamicArray(DynamicArray&& other) //Move Constructor
		{
			m_data = other.m_data;
			m_length = other.m_length;
			m_pparentAllocator = other.m_pparentAllocator;
			other.m_data = nullptr;
			other.m_length = 0;
			other.m_pparentAllocator = nullptr;
		}
		DynamicArray& operator=(const DynamicArray&  other)  //Copy Assignment
		{
			deleteArray();

			createArray(other.m_length, other.m_pparentAllocator);
			for (size_t i = 0; i < m_length; i++)
			{
				m_data[i] = other[i];
			}

			return *this;
		}
		DynamicArray& operator=(DynamicArray&& other) //Move Assignment
		{
			deleteArray();

			m_data = other.m_data;
			m_length = other.m_length;
			m_pparentAllocator = other.m_pparentAllocator;
			other.m_data = nullptr;
			other.m_length = 0;

			return *this;
		}

		T& operator[](size_t index)
		{
			return m_data[index];
		}

		const T& operator[](size_t index) const
		{
			return m_data[index];
		}

		size_t getLength() const
		{
			return m_length;
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

	template<typename T>
	uint32_t hash(const DynamicArray<T> &t)
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

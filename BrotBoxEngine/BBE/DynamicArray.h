#pragma once

#include "../BBE/Array.h"
#include "../BBE/Hash.h"

namespace bbe
{
	template<typename T, bool keepSorted>
	class List;

	template <typename T>
	class DynamicArray
	{
		//TODO use Allocator
	private:

		T* m_data;
		size_t m_length;
	public:
		DynamicArray(size_t size)
			: m_length(size)
		{
			//UNTESTED
			m_data = new T[size];
		}

		template <typename U, int size>
		DynamicArray(Array<U, size> arr)
			: m_length(size)
		{
			//UNTESTED
			m_data = new T[m_length];
			for (int i = 0; i < size; i++)
			{
				m_data[i] = arr[i];
			}
		}

		DynamicArray(List<T, true> list)
			: m_length(list.getLength())
		{
			//UNTESTED
			m_data = new T[m_length];
			for (int i = 0; i < m_length; i++)
			{
				m_data[i] = list[i];
			}
		}

		DynamicArray(List<T, false> list)
			: m_length(list.getLength())
		{
			//UNTESTED
			//TODO basically this is a copy of above. Put into function!
			m_data = new T[m_length];
			for (int i = 0; i < m_length; i++)
			{
				m_data[i] = list[i];
			}
		}

		~DynamicArray()
		{
			//UNTESTED
			delete [] m_data;
		}

		DynamicArray(const DynamicArray&  other) //Copy Constructor
		{
			//UNTESTED
			m_data = new T[other.m_length];
			for (size_t i = 0; i < size; i++)
			{
				m_data[i] = other[i];
			}
		}
		DynamicArray(DynamicArray&& other) //Move Constructor
		{
			//UNTESTED
			m_data = other.m_data;
			m_length = other.m_length;
			other.m_data = nullptr;
			other.m_length = 0;
		}
		DynamicArray& operator=(const DynamicArray&  other)  //Copy Assignment
		{
			//UNTESTED
			delete[] m_data;

			m_data = new T[other.m_length];
			for (size_t i = 0; i < size; i++)
			{
				m_data[i] = other[i];
			}
		}
		DynamicArray& operator=(DynamicArray&& other) //Move Assignment
		{
			//UNTESTED
			delete[] m_data;

			m_data = other.m_data;
			m_length = other.m_length;
			other.m_data = nullptr;
			other.m_length = 0;
		}

		T& operator[](size_t index)
		{
			//UNTESTED
			return m_data[index];
		}

		const T& operator[](size_t index) const
		{
			//UNTESTED
			return m_data[index];
		}

		size_t getLength() const
		{
			//UNTESTED
			return m_length;
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

	template<typename T>
	uint32_t hash(const DynamicArray<T> &t)
	{
		//UNTESTED
		size_t length = t.getSize();
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

#pragma once

#include "Array.h"

namespace bbe {
	template<typename T, bool keepSorted = false>
	class List;

	template <typename T>
	class DynamicArray {
		//TODO use Allocator
	private:

		T* m_data;
		size_t m_size;
	public:
		DynamicArray(size_t size)
			: m_size(size)
		{
			//UNTESTED
			m_data = new T[size];
		}

		template <typename U, int size>
		DynamicArray(Array<U, size> arr)
			: m_size(size)
		{
			//UNTESTED
			m_data = new T[size];
			for (int i = 0; i < size; i++) {
				m_data[i] = arr[i];
			}
		}

		DynamicArray(List<T> list)
			: m_size(list.getLength())
		{
			//UNTESTED
			m_data = new T[size];
			for (int i = 0; i < size; i++) {
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
			m_data = new T[other.m_size];
			for (size_t i = 0; i < size; i++)
			{
				m_data[i] = other[i];
			}
		}
		DynamicArray(DynamicArray&& other) //Move Constructor
		{
			//UNTESTED
			m_data = other.m_data;
			m_size = other.m_size;
			other.m_data = nullptr;
			other.m_size = 0;
		}
		DynamicArray& operator=(const DynamicArray&  other)  //Copy Assignment
		{
			//UNTESTED
			delete[] m_data;

			m_data = new T[other.m_size];
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
			m_size = other.m_size;
			other.m_data = nullptr;
			other.m_size = 0;
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

		size_t getLength() const {
			//UNTESTED
			return m_size;
		}

		T* getRaw() {
			//UNTESTED
			return m_data;
		}

		const T* getRaw() const {
			//UNTESTED
			return m_data;
		}
	};
}
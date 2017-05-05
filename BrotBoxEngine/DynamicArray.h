#pragma once

namespace bbe {
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
			m_data = new T[size];
		}

		~DynamicArray()
		{
			delete [] m_data;
		}

		DynamicArray(const DynamicArray&  other) //Copy Constructor
		{
			m_data = new T[other.m_size];
			for (size_t i = 0; i < size; i++)
			{
				m_data[i] = other[i];
			}
		}
		DynamicArray(DynamicArray&& other) //Move Constructor
		{
			m_data = other.m_data;
			m_size = other.m_size;
			other.m_data = nullptr;
			other.m_size = 0;
		}
		DynamicArray& operator=(const DynamicArray&  other)  //Copy Assignment
		{
			delete[] m_data;

			m_data = new T[other.m_size];
			for (size_t i = 0; i < size; i++)
			{
				m_data[i] = other[i];
			}
		}
		DynamicArray& operator=(DynamicArray&& other) //Move Assignment
		{
			delete[] m_data;

			m_data = other.m_data;
			m_size = other.m_size;
			other.m_data = nullptr;
			other.m_size = 0;
		}

		T& operator[](size_t index)
		{
			return m_data[index];
		}

		size_t getLength() const {
			return m_size;
		}
	};
}
#pragma once
#include "../BBE/DynamicArray.h"
#include "../BBE/Math.h"

//TODO this template is far from complete!
//TODO use allocator
namespace bbe
{
	template<typename T>
	class RingArray
	{
	private:
		DynamicArray<T> m_data;

	public:
		explicit RingArray(size_t size)
			: m_data(size)
		{
			//do nothing
		}

		T& operator[](int index)
		{
			index = bbe::Math::mod(index, m_data.getLength());
			return m_data[index];
		}

		const T& operator[](int index) const
		{
			index = bbe::Math::mod(index, m_data.getLength());
			return m_data[index];
		}
	};
}
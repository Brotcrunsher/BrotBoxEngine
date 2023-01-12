#pragma once

#include <stddef.h>
#include "../BBE/UtilDebug.h"

namespace bbe
{
	template <typename T>
	class Span
	{
	private:
		T* m_pdata;
		size_t m_size;

	public:
		Span() :
			m_pdata(nullptr),
			m_size(0)
		{}

		Span(T* data, size_t size) :
			m_pdata(data),
			m_size(size)
		{}

		T* getRaw()
		{
			return m_pdata;
		}

		const T* getRaw() const
		{
			return m_pdata;
		}

		size_t getSize() const
		{
			return m_size;
		}
		
		T& operator[](size_t index)
		{
			if (index >= m_size)
			{
				debugBreak();
			}
			return m_pdata[index];
		}

		const T& operator[](size_t index) const
		{
			if (index >= m_size)
			{
				debugBreak();
			}
			return m_pdata[index];
		}
	};
}

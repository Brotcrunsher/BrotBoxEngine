#pragma once


#include "../BBE/DefaultDestroyer.h"
#include "../BBE/UtilDebug.h"
#include <utility>

namespace bbe
{
	template <typename T, typename Destroyer = bbe::INTERNAL::DefaultDestroyer>
	class UniquePointer
	{
	private:
		T *m_ptr;
		Destroyer m_destroyer;

	public:
		explicit UniquePointer(T *ptr)
			: m_ptr(ptr)
		{
			//do nothing
		}

		explicit UniquePointer(T *ptr, Destroyer destroyer)
			: m_ptr(ptr), m_destroyer(destroyer)
		{
			//do nothing
		}

		~UniquePointer()
		{
			if (m_ptr != nullptr)
			{
				m_destroyer.destroy(m_ptr);
				m_ptr = nullptr;
			}
		}

		UniquePointer(const UniquePointer& other) = delete;
		UniquePointer(UniquePointer&& other)
		{
			m_ptr = std::move(other.m_ptr);
			other.m_ptr = nullptr;
		}
		UniquePointer& operator= (const UniquePointer& other) = delete;
		UniquePointer& operator= (UniquePointer&& other)
		{
			if (m_ptr != nullptr)
			{
				Destroyer::destroy(m_ptr);
			}

			m_ptr = other.m_ptr;
			other.m_ptr = nullptr;
		}

		UniquePointer& operator= (T* ptr)
		{
			//UNTESTED
			if (m_ptr != nullptr)
			{
				Destroyer::destroy(m_ptr);
			}

			m_ptr = std::move(ptr);

		}

		
		T* operator ->()
		{
			return m_ptr;
		}

		const T* operator ->() const
		{
			//UNTESTED
			return m_ptr;
		}

		T& operator *()
		{
			//UNTESTED
			return *m_ptr;
		}

		const T& operator *() const
		{
			//UNTESTED
			return *m_ptr;
		}

		T* getRaw()
		{
			return m_ptr;
		}
	};
}

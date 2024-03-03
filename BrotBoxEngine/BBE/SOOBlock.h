#pragma once
#include <type_traits>
#include "AllocBlock.h"

namespace bbe
{
	template<typename T, size_t sooSize>
	class SOOBlock
	{
		static_assert(std::is_trivially_destructible_v<T>, "T is assumed to be trivially destructible! Currently does not call Destructors.");
	private:
		union
		{
			T* m_pdata;
			T m_sooData[sooSize];
		};
		size_t m_capacity = sooSize;
		bbe::AllocBlock ab;

	public:
		SOOBlock() 
		{
			m_sooData[0] = 0;
		}

		SOOBlock(const SOOBlock& other)
		{
			growIfNeeded(other.m_capacity, 0);
			T* thisPtr = get();
			const T* otherPtr = other.get();
			for (size_t i = 0; i < m_capacity; i++)
			{
				thisPtr[i] = otherPtr[i];
			}
		}

		SOOBlock(SOOBlock&& other) noexcept
		{
			m_capacity = other.m_capacity;
			if (isUsingSoo())
			{
				T* thisPtr = get();
				const T* otherPtr = other.get();
				for (size_t i = 0; i < m_capacity; i++)
				{
					thisPtr[i] = std::move(otherPtr[i]);
				}
			}
			else
			{
				ab = other.ab;
				m_pdata = other.m_pdata;
				other.m_capacity = sooSize;
			}
		}

		SOOBlock& operator=(const SOOBlock& other)
		{
			growIfNeeded(other.m_capacity, 0);
			T* thisPtr = get();
			const T* otherPtr = other.get();
			for (size_t i = 0; i < m_capacity; i++)
			{
				thisPtr[i] = otherPtr[i];
			}
			return *this;
		}

		SOOBlock& operator=(SOOBlock&& other) noexcept
		{
			if (!isUsingSoo())
			{
				bbe::freeBlock(ab);
			}
			m_capacity = other.m_capacity;
			if (isUsingSoo())
			{
				T* thisPtr = get();
				const T* otherPtr = other.get();
				for (size_t i = 0; i < m_capacity; i++)
				{
					thisPtr[i] = std::move(otherPtr[i]);
				}
			}
			else
			{
				ab = other.ab;
				m_pdata = other.m_pdata;
				other.m_capacity = sooSize;
			}
			return *this;
		}

		~SOOBlock()
		{
			if (!isUsingSoo())
			{
				bbe::freeBlock(ab);
			}
		}

		T* get()
		{
			if (isUsingSoo()) return m_sooData;
			return m_pdata;
		}

		const T* get() const
		{
			if (isUsingSoo()) return m_sooData;
			return m_pdata;
		}

		size_t getCapacity() const
		{
			return m_capacity;
		}

		bool isUsingSoo() const
		{
			return m_capacity == sooSize;
		}

		void growIfNeeded(size_t newCapacity, size_t copyUntil = (size_t)-1)
		{
			if (newCapacity > m_capacity)
			{
				if (newCapacity < 2 * m_capacity) newCapacity = 2 * m_capacity;
				if (copyUntil == (size_t)-1) copyUntil = m_capacity;

				AllocBlock ab = bbe::allocateBlock(newCapacity * sizeof(T));
				T* newData = new (ab.data) T[newCapacity];
				T* oldData = get();
				for (size_t i = 0; i < copyUntil; i++)
				{
					newData[i] = std::move(oldData[i]);
				}
				if (!isUsingSoo())
				{
					bbe::freeBlock(this->ab);
				}

				this->ab = ab;
				m_pdata = newData;
				m_capacity = ab.size / sizeof(T);
			}
		}
	};
}

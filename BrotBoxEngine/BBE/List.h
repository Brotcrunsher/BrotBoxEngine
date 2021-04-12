#pragma once

#include <functional>
#include "../BBE/STLCapsule.h"
#include "../BBE/Array.h"
#include "../BBE/Unconstructed.h"
#include "../BBE/UtilDebug.h"
#include "../BBE/Hash.h"
#include "../BBE/Exceptions.h"
#include <initializer_list>
#include <iostream>

namespace bbe
{
	template <typename T, bool keepSorted = false>
	class List
	{
		//TODO use own allocators
	private:
		size_t m_length;
		size_t m_capacity;
		INTERNAL::Unconstructed<T>* m_pdata;

		void growIfNeeded(size_t amountOfNewObjects)
		{
			if (m_capacity < m_length + amountOfNewObjects)
			{
				size_t newCapacity = m_length + amountOfNewObjects;
				if (newCapacity < m_capacity * 2)
				{
					newCapacity = m_capacity * 2;
				}

				INTERNAL::Unconstructed<T>* newData = new INTERNAL::Unconstructed<T>[newCapacity];

				for (size_t i = 0; i < m_length; i++)
				{
					new (bbe::addressOf(newData[i].m_value)) T(std::move(m_pdata[i].m_value));
					m_pdata[i].m_value.~T();
				}

				if (m_pdata != nullptr)
				{
					delete[] m_pdata;
				}
				m_pdata = newData;
				m_capacity = newCapacity;
			}
		}

	public:
		List()
			: m_length(0), m_capacity(0), m_pdata(nullptr)
		{
			//DO NOTHING
		}

		explicit List(size_t amountOfData)
			: m_length(0), m_capacity(amountOfData)
		{
			m_pdata = new INTERNAL::Unconstructed<T>[amountOfData];
		}

		template <typename... arguments>
		List(size_t amountOfObjects, arguments&&... args)
			: m_length(amountOfObjects), m_capacity(amountOfObjects)
		{
			m_pdata = new INTERNAL::Unconstructed<T>[amountOfObjects];
			const T copyVal = T(std::forward<arguments>(args)...);
			for (size_t i = 0; i < amountOfObjects; i++)
			{
				new (bbe::addressOf(m_pdata[i])) T(copyVal);
			}
		}

		List(const List<T, keepSorted>& other)
			: m_length(other.m_length), m_capacity(other.m_capacity)
		{
			m_pdata = new INTERNAL::Unconstructed<T>[m_capacity];
			for (size_t i = 0; i < m_length; i++)
			{
				new (bbe::addressOf(m_pdata[i])) T(other.m_pdata[i].m_value);
			}
		}

		List(List<T, keepSorted>&& other)
			: m_length(other.m_length), m_capacity(other.m_capacity), m_pdata(other.m_pdata)
		{
			other.m_pdata = nullptr;
			other.m_length = 0;
			other.m_capacity = 0;
		}

		/*nonexplicit*/ List(const std::initializer_list<T> &il)
			: m_length(0), m_capacity(il.size())
		{
			m_pdata = new INTERNAL::Unconstructed<T>[m_capacity];
			for (auto iter = il.begin(); iter != il.end(); iter++) {
				add(*iter);
			}
		}

		List& operator=(const List<T, keepSorted>& other)
		{
			clear();

			if (m_pdata != nullptr)
			{
				delete[] m_pdata;
			}

			m_length = other.m_length;
			m_capacity = other.m_capacity;
			m_pdata = new INTERNAL::Unconstructed<T>[m_capacity];
			for (size_t i = 0; i < m_capacity; i++)
			{
				new (bbe::addressOf(m_pdata[i])) T(other.m_pdata[i].m_value);
			}

			return *this;
		}

		List& operator=(List<T, keepSorted>&& other)
		{
			clear();

			if (m_pdata != nullptr)
			{
				delete[] m_pdata;
			}

			m_length = other.m_length;
			m_capacity = other.m_capacity;
			m_pdata = other.m_pdata;

			other.m_pdata = nullptr;
			other.m_length = 0;
			other.m_capacity = 0;

			return *this;
		}

		~List()
		{
			clear();

			if (m_pdata != nullptr)
			{
				delete[] m_pdata;
			}

			m_pdata = nullptr;
			m_length = 0;
			m_capacity = 0;
		}

		size_t getCapacity() const
		{
			return m_capacity;
		}

		size_t getLength() const
		{
			return m_length;
		}

		T* getRaw()
		{
			return reinterpret_cast<T*>(m_pdata);
		}

		const T* getRaw() const
		{
			//UNTESTED
			return reinterpret_cast<const T*>(m_pdata);
		}

		bool isEmpty() const
		{
			return m_length == 0;
		}

		T& operator[](size_t index)
		{
			if (index >= m_length)
			{
				debugBreak();
			}
			return m_pdata[index].m_value;
		}

		const T& operator[](size_t index) const
		{
			if (index >= m_length)
			{
				debugBreak();
			}
			return m_pdata[index].m_value;
		}

		List<T, keepSorted>& operator+=(List<T, keepSorted> other)
		{
			for (size_t i = 0; i < other.m_length; i++)
			{
				add(other.m_pdata[i].m_value);
			}
			return *this;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<dummyKeepSorted, void>::type add(const T& val, const int amount = 1)
		{
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");
			//TODO rewrite this method using size_t instead of int
			if (amount <= 0)
			{
				debugBreak();
			}
			growIfNeeded(amount);
			//UNTESTED
			int i = 0;
			for (i = (int)m_length + amount - 1; i >= 0; i--)
			{
				int lowerIndex = i - amount;
				if (lowerIndex >= 0 && val < m_pdata[lowerIndex].m_value)
				{
					if (i >= (int)m_length)
					{
						new (bbe::addressOf(m_pdata[i])) T(std::move(m_pdata[lowerIndex].m_value));
					}
					else
					{
						m_pdata[i].m_value = std::move(m_pdata[lowerIndex].m_value);
					}
				}
				else
				{
					break;
				}
			}
			int insertionIndex = i;
			for (; i >= insertionIndex - amount + 1 && i >= 0; i--)
			{
				if (i >= (int)m_length)
				{
					new (bbe::addressOf(m_pdata[i])) T(val);
				}
				else
				{
					m_pdata[i].m_value = val;
				}
			}
			m_length += amount;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<!dummyKeepSorted, void>::type add(const T& val, size_t amount = 1)
		{
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");
			growIfNeeded(amount);
			for (size_t i = 0; i < amount; i++)
			{
				new (bbe::addressOf(m_pdata[m_length + i])) T(val);
			}
			m_length += amount;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<dummyKeepSorted, void>::type add(T&& val, const int amount = 1)
		{
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");
			//TODO rewrite this method using size_t instead of int
			if (amount <= 0)
			{
				debugBreak();
			}
			growIfNeeded(amount);
			//UNTESTED
			int i = 0;
			for (i = (int)m_length + amount - 1; i >= 0; i--)
			{
				int lowerIndex = i - amount;
				if (lowerIndex >= 0 && val < m_pdata[lowerIndex].m_value)
				{
					if (i >= (int)m_length)
					{
						new (bbe::addressOf(m_pdata[i])) T(std::move(m_pdata[lowerIndex].m_value));
					}
					else
					{
						m_pdata[i].m_value = std::move(m_pdata[lowerIndex].m_value);
					}
				}
				else
				{
					break;
				}
			}
			int insertionIndex = i;
			for (; i >= insertionIndex - amount + 1 && i >= 0; i--)
			{
				if (i >= (int)m_length)
				{
					if (amount == 1)
					{
						new (bbe::addressOf(m_pdata[m_length])) T(std::move(val));
					}
					else
					{
						new (bbe::addressOf(m_pdata[i])) T(val);
					}
				}
				else
				{
					m_pdata[i].m_value = std::move(val);
				}
			}
			m_length += amount;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<!dummyKeepSorted, void>::type add(T&& val, size_t amount)
		{
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");
			growIfNeeded(amount);
			if (amount == 1)
			{
				new (bbe::addressOf(m_pdata[m_length])) T(std::move(val));
			}
			else
			{
				for (size_t i = 0; i < amount; i++)
				{
					new (bbe::addressOf(m_pdata[m_length + i])) T(val);
				}
			}

			m_length += amount;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<!dummyKeepSorted, void>::type add(T&& val)
		{
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");
			growIfNeeded(1);
			new (bbe::addressOf(m_pdata[m_length])) T(std::move(val));

			m_length += 1;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<!dummyKeepSorted, size_t>::type getIndexOnAdd(const T& val)
		{
			return m_length;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<dummyKeepSorted, size_t>::type getIndexOnAdd(const T& val)
		{
			//UNTESTED
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");

			if (m_length == 0)
			{
				return 0;
			}
			else if (m_length == 1)
			{
				if (m_pdata[0].m_value > val)
				{
					return 0;
				}
				else
				{
					return 1;
				}
			}
			if (val < m_pdata[0].m_value)
			{
				return 0;
			}
			if (val > m_pdata[m_length - 1].m_value)
			{
				return m_length;
			}

			size_t smallIndex = 0;
			size_t bigIndex = m_length - 1;

			while (smallIndex <= bigIndex)
			{
				size_t searchSpace = bigIndex - smallIndex;
				size_t middleIndex = smallIndex + searchSpace / 2;
				if (m_pdata[middleIndex].m_value == val)
				{
					return middleIndex;
				}
				else
				{
					if (m_pdata[middleIndex].m_value > val)
					{
						if (middleIndex == 0)
						{
							return 0;
						}
						bigIndex = middleIndex - 1;
					}
					else
					{
						if (middleIndex >= m_length)
						{
							return m_length;
						}
						smallIndex = middleIndex + 1;
					}
				}
			}
			return smallIndex;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<!dummyKeepSorted, void>::type getNeighbors(const T& val, T*& leftNeighbor, T*& rightNeighbor)
		{
			assert(false && "Only sorted Lists can getIndexWhenPushedBack!");
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<dummyKeepSorted, void>::type getNeighbors(const T& val, T*& leftNeighbor, T*& rightNeighbor)
		{
			//UNTESTED
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");

			if (m_length == 0)
			{
				leftNeighbor = nullptr;
				rightNeighbor = nullptr;
				return;
			}
			else if (m_length == 1)
			{
				if (m_pdata[0].m_value > val)
				{
					leftNeighbor = nullptr;
					rightNeighbor = reinterpret_cast<T*>(m_pdata);
					return;
				}
				else
				{
					leftNeighbor = reinterpret_cast<T*>(m_pdata);
					rightNeighbor = nullptr;
					return;
				}
			}

			size_t index = getIndexOnAdd(val);
			while (index < m_length - 1 && m_pdata[index].m_value == val)
			{
				index++;
			}

			if (index == 0)
			{
				leftNeighbor = nullptr;
				rightNeighbor = reinterpret_cast<T*>(m_pdata);
				return;
			}
			if (index >= m_length)
			{
				leftNeighbor = reinterpret_cast<T*>(m_pdata + m_length - 1);
				rightNeighbor = nullptr;
				return;
			}

			leftNeighbor = reinterpret_cast<T*>(m_pdata + index - 1);
			rightNeighbor = reinterpret_cast<T*>(m_pdata + index);

			return;
		}

		template <typename U>
		void addAll(U&& t)
		{
			add(std::forward<U>(t));
		}

		template<typename U, typename... arguments>
		void addAll(U&& t, arguments&&... args)
		{
			add(std::forward<U>(t));
			addAll(std::forward<arguments>(args)...);
		}

		void addArray(const T* data, size_t size)
		{
			for (size_t i = 0; i < size; i++)
			{
				add(data[i]);
			}
		}

		template<int size>
		void addArray(Array<T, size>& arr)
		{
			addArray(arr.getRaw(), size);
		}

		void popBack(size_t amount = 1)
		{
			if (amount > m_length)
			{
				debugBreak();
			}
			for (size_t i = 0; i < amount; i++)
			{
				m_pdata[m_length - 1 - i].m_value.~T();
			}
			m_length -= amount;
		}

		void clear()
		{
			if (!std::is_trivially_destructible_v<T>)
			{
				for (size_t i = 0; i < m_length; i++)
				{
					(&(m_pdata[i].m_value))->~T();
				}
			}
			m_length = 0;
		}
		
		bool shrink()
		{
			if (m_length == m_capacity)
			{
				return false;
			}
			m_capacity = m_length;

			if (m_length == 0)
			{
				if (m_pdata != nullptr)
				{
					delete[] m_pdata;
				}
				m_pdata = nullptr;
				return true;
			}
			INTERNAL::Unconstructed<T>* newList = new INTERNAL::Unconstructed<T>[m_length];
			for (size_t i = 0; i < m_length; i++)
			{
				new (bbe::addressOf(newList[i])) T(std::move(m_pdata[i].m_value));
			}
			if (m_pdata != nullptr)
			{
				for (size_t i = 0; i < m_length; i++)
				{
					bbe::addressOf(m_pdata[i].m_value)->~T();
				}
				delete[] m_pdata;
			}
			m_pdata = newList;
			return true;
		}

		template <typename dummyT = T>
		typename std::enable_if<std::is_default_constructible<dummyT>::value, void>::type
			resizeCapacityAndLength(size_t newCapacity)
		{
			//UNTESTED
			static_assert(std::is_same<dummyT, T>::value, "Do not specify dummyT!");
			resizeCapacity(newCapacity);
			if constexpr (std::is_trivially_constructible_v<T>)
			{
				m_length = newCapacity;
			}
			else
			{
				add(T(), newCapacity - m_length);
			}
		}

		void resizeCapacity(size_t newCapacity)
		{
			//UNTESTED
			if (newCapacity < m_length)
			{
				debugBreak();
				throw IllegalArgumentException();
			}

			if (newCapacity == m_capacity)
			{
				return;
			}

			INTERNAL::Unconstructed<T>* newList = new INTERNAL::Unconstructed<T>[newCapacity];
			for (size_t i = 0; i < m_length; i++)
			{
				new (bbe::addressOf(newList[i])) T(std::move(m_pdata[i].m_value));
			}
			if (m_pdata != nullptr) {
				for (size_t i = 0; i < m_length; i++) {
					bbe::addressOf(m_pdata[i].m_value)->~T();
				}
				delete[] m_pdata;
			}
			m_pdata = newList;
			m_capacity = newCapacity;
		}

		size_t removeAll(const T& remover)
		{
			return removeAll(
				[&](const T& other)
				{ 
					return other == remover;
				});
		}

		size_t removeAll(std::function<bool(const T&)> predicate)
		{
			size_t moveRange = 0;
			for (size_t i = 0; i < m_length; i++)
			{
				if (predicate(m_pdata[i].m_value))
				{
					bbe::addressOf(m_pdata[i].m_value)->~T();
					moveRange++;
				}
				else if (moveRange != 0)
				{
					m_pdata[i - moveRange].m_value = std::move(m_pdata[i].m_value);
				}
			}
			m_length -= moveRange;
			return moveRange;
		}

		bool removeIndex(size_t index) {
			if (index >= m_length) {
				return false;
			}

			m_pdata[index].m_value.~T();
			if (index != m_length - 1)
			{
				new (bbe::addressOf(m_pdata[index].m_value)) T(std::move(m_pdata[index + 1].m_value));

				for (size_t i = index + 1; i < m_length - 1; i++)
				{
					m_pdata[i].m_value = std::move(m_pdata[i + 1].m_value);
				}
			}


			m_length--;
			return true;
		}

		bool removeSingle(const T& remover)
		{
			return removeSingle(
				[&](const T& t)
				{
					return remover == t;
				});
		}

		bool removeSingle(std::function<bool(const T&)> predicate)
		{
			size_t index = 0;
			bool found = false;
			for (size_t i = 0; i < m_length; i++)
			{
				if (predicate(m_pdata[i].m_value))
				{
					index = i;
					found = true;
					break;
				}
			}
			if (!found) return false;

			return removeIndex(index);
		}

		size_t containsAmount(const T& t) const
		{
			return containsAmount(
				[&](const T& other)
				{
					return t == other;
				});
		}

		size_t containsAmount(std::function<bool(const T&)> predicate) const
		{
			size_t amount = 0;
			for (size_t i = 0; i < m_length; i++)
			{
				if (predicate(m_pdata[i].m_value))
				{
					amount++;
				}
			}
			return amount;
		}

		bool contains(const T& t) const
		{
			return contains(
				[&](const T& other)
				{
					return t == other;
				});
		}

		bool contains(std::function<bool(const T&)> predicate) const
		{
			for (size_t i = 0; i < m_length; i++)
			{
				if (predicate(m_pdata[i].m_value))
				{
					return true;
				}
			}
			return false;
		}

		bool containsUnique(const T& t) const
		{
			return containsAmount(t) == 1;
		}

		bool containsUnique(std::function<bool(const T&)> predicate) const
		{
			return containsAmount(predicate) == 1;
		}

		T* begin()
		{
			//UNTESTED
			return &(this->m_pdata[0].m_value);
		}

		const T* begin() const
		{
			//UNTESTED
			return &(this->m_pdata[0].m_value);
		}

		T* end()
		{
			//UNTESTED
			return &(this->m_pdata[getLength()].m_value);
		}

		const T* end() const
		{
			//UNTESTED
			return &(this->m_pdata[getLength()].m_value);
		}

		void sort()
		{
			sortSTL(reinterpret_cast<T*>(m_pdata), reinterpret_cast<T*>(m_pdata + m_length));
		}

		void sort(std::function<bool(const T&, const T&)> predicate)
		{
			sortSTL(reinterpret_cast<T*>(m_pdata), reinterpret_cast<T*>(m_pdata + m_length), predicate);
		}

		T& first()
		{
			//UNTESTED
			if (m_pdata == nullptr)
			{
				throw ContainerEmptyException();
			}

			return (m_pdata[0].m_value);
		}

		T& last()
		{
			//UNTESTED
			if (m_pdata == nullptr)
			{
				throw ContainerEmptyException();
			}

			return (m_pdata[m_length - 1].m_value);
		}

		T* find(const T& t)
		{
			return find(
				[&](const T& other)
				{
					return t == other;
				});
		}

		T* find(std::function<bool(const T&)> predicate)
		{
			for (size_t i = 0; i < m_length; i++)
			{
				if (predicate(m_pdata[i].m_value))
				{
					return reinterpret_cast<T*>(m_pdata + i);
				}
			}
			return nullptr;
		}

		T* findLast(const T& t)
		{
			return findLast(
				[&t](const T& other)
				{
					return t == other;
				});
		}

		T* findLast(std::function<bool(const T&)> predicate)
		{
			for (size_t i = m_length - 1; i >= 0 && i != std::numeric_limits<size_t>::max(); i--)
			{
				if (predicate(m_pdata[i].m_value))
				{
					return reinterpret_cast<T*>(m_pdata + i);
				}
			}
			return nullptr;
		}

		bool operator==(const List<T>& other)
		{
			if (m_length != other.m_length)
			{
				return false;
			}

			for (size_t i = 0; i < m_length; i++)
			{
				if (m_pdata[i].m_value != other.m_pdata[i].m_value)
				{
					return false;
				}
			}

			return true;
		}

		bool operator!=(const List<T>& other)
		{
			return !(operator==(other));
		}


	};

	template<typename T>
	uint32_t hash(const List<T> &t)
	{
		//UNTESTED
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

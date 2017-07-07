#pragma once

#include <functional>
#include "STLCapsule.h"
#include "Array.h"
#include "DynamicArray.h"
#include "Unconstructed.h"
#include <initializer_list>

namespace bbe
{

	

	template <typename T, bool keepSorted = false>
	class List
	{
		//TODO use own allocators
		//TODO make usable in foreach
	private:
		size_t m_length;
		size_t m_capacity;
		INTERNAL::Unconstructed<T>* m_data;

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
					new (bbe::addressOf(newData[i].value)) T(std::move(m_data[i].value));
					m_data[i].value.~T();
				}

				if (m_data != nullptr)
				{
					delete[] m_data;
				}
				m_data = newData;
				m_capacity = newCapacity;
			}
		}

	public:
		List()
			: m_length(0), m_capacity(0), m_data(nullptr)
		{
			//DO NOTHING
		}

		template <typename... arguments>
		List(size_t amountOfObjects, arguments&&... args)
			: m_length(amountOfObjects), m_capacity(amountOfObjects)
		{
			m_data = new INTERNAL::Unconstructed<T>[amountOfObjects];
			for (size_t i = 0; i < amountOfObjects; i++)
			{
				new (bbe::addressOf(m_data[i])) T(std::forward<arguments>(args)...);
			}
		}

		List(const List<T, keepSorted>& other)
			: m_length(other.m_length), m_capacity(other.m_capacity)
		{
			m_data = new INTERNAL::Unconstructed<T>[m_capacity];
			for (size_t i = 0; i < m_length; i++)
			{
				new (bbe::addressOf(m_data[i])) T(other.m_data[i].value);
			}
		}

		List(List<T, keepSorted>&& other)
			: m_length(other.m_length), m_capacity(other.m_capacity), m_data(other.m_data)
		{
			other.m_data = nullptr;
			other.m_length = 0;
			other.m_capacity = 0;
		}

		List(std::initializer_list<T> il) {
			//UNTESTED
			for (auto iter = il.begin(); iter != il.end(); iter++) {
				add(*iter);
			}
		}

		List& operator=(const List<T, keepSorted>& other)
		{
			if (m_data != nullptr)
			{
				delete[] m_data;
			}

			m_length = other.m_length;
			m_capacity = other.m_capacity;
			m_data = new INTERNAL::Unconstructed<T>[m_capacity];
			for (size_t i = 0; i < m_capacity; i++)
			{
				new (bbe::addressOf(m_data[i])) T(other.m_data[i].value);
			}

			return *this;
		}

		List& operator=(List<T, keepSorted>&& other)
		{
			if (m_data != nullptr)
			{
				delete[] m_data;
			}

			m_length = other.m_length;
			m_capacity = other.m_capacity;
			m_data = other.m_data;

			other.m_data = nullptr;
			other.m_length = 0;
			other.m_capacity = 0;

			return *this;
		}

		~List()
		{
			clear();

			if (m_data != nullptr)
			{
				delete[] m_data;
			}

			m_data = nullptr;
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
			return reinterpret_cast<T*>(m_data);
		}

		const T* getRaw() const
		{
			//UNTESTED
			return reinterpret_cast<const T*>(m_data);
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
			return m_data[index].value;
		}

		const T& operator[](size_t index) const
		{
			if (index >= m_length)
			{
				debugBreak();
			}
			return m_data[index].value;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<dummyKeepSorted, List<typename T, dummyKeepSorted>&>::type
		operator+=(List<T, dummyKeepSorted> other)
		{
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");
			//UNTESTED
			growIfNeeded(other.m_length);
			size_t newLength = m_length + other.m_length;
			size_t indexThis = m_length - 1;
			size_t indexOther = other.m_length - 1;
			m_length = newLength;
			for (size_t i = newLength - 1; i >= m_length; i++)
			{
				if (indexThis != std::numeric_limits<size_t>::max() && other[indexOther] >= m_data[indexThis])
				{
					new (bbe::addressOf(m_data[i])) T(other[indexOther]);
					indexOther--;
					if (indexOther == std::numeric_limits<size_t>::max())
					{
						return *this;
					}
				}
				else
				{
					new (bbe::addressOf(m_data[i])) T(std::move(m_data[indexThis]));
					indexThis--;
				}
			}
			for (size_t i = m_length - 1; i >= 0; i++)
			{
				if (indexThis != std::numeric_limits<size_t>::max() && other[indexOther] >= m_data[indexThis])
				{
					m_data[i] = other[indexOther];
					indexOther--;
					if (indexOther == std::numeric_limits<size_t>::max())
					{
						return *this;
					}
				}
				else
				{
					m_data[i] = std::move(m_data[indexThis]);
					indexThis--;
				}
			}
			return *this;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<!dummyKeepSorted, List&>::type
			operator+=(List<T, dummyKeepSorted> other)
		{
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");
			for (size_t i = 0; i < other.m_length; i++)
			{
				add(other.m_data[i].value);
			}
			return *this;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<dummyKeepSorted, void>::type add(const T& val, int amount = 1)
		{
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");
			//TODO rewrite this method using size_t instead of int
			growIfNeeded(amount);
			//UNTESTED
			int i = 0;
			for (i = (int)m_length + amount - 1; i >= 0; i--)
			{
				int lowerIndex = i - amount;
				if (lowerIndex >= 0 && val < m_data[lowerIndex].value)
				{
					new (bbe::addressOf(m_data[i])) T(std::move(m_data[lowerIndex].value));
					bbe::addressOf(m_data[lowerIndex].value)->~T();
				}
				else
				{
					break;
				}
			}
			int insertionIndex = i;
			for (; i >= insertionIndex - amount + 1 && i >= 0; i--)
			{
				if (i >= m_length)
				{
					new (bbe::addressOf(m_data[i])) T(val);
				}
				else
				{
					m_data[i].value = val;
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
				new (bbe::addressOf(m_data[m_length + i])) T(val);
			}
			m_length += amount;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<dummyKeepSorted, void>::type add(T&& val, int amount = 1)
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
				if (lowerIndex >= 0 && val < m_data[lowerIndex].value)
				{
					new (bbe::addressOf(m_data[i])) T(std::move(m_data[lowerIndex].value));
					bbe::addressOf(m_data[lowerIndex].value)->~T();
				}
				else
				{
					break;
				}
			}
			int insertionIndex = i;
			for (; i >= insertionIndex - amount + 1 && i >= 0; i--)
			{
				if (i >= m_length)
				{
					if (amount == 1)
					{
						new (bbe::addressOf(m_data[m_length])) T(std::move(val));
					}
					else
					{
						new (bbe::addressOf(m_data[i])) T(val);
					}
				}
				else
				{
					m_data[i].value = val;
				}
			}
			m_length += amount;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<!dummyKeepSorted, void>::type add(T&& val, size_t amount = 1)
		{
			static_assert(dummyKeepSorted == keepSorted, "Do not specify dummyKeepSorted!");
			growIfNeeded(amount);
			if (amount == 1)
			{
				new (bbe::addressOf(m_data[m_length])) T(std::move(val));
			}
			else
			{
				for (size_t i = 0; i < amount; i++)
				{
					new (bbe::addressOf(m_data[m_length + i])) T(val);
				}
			}

			m_length += amount;
		}

		template <bool dummyKeepSorted = keepSorted>
		typename std::enable_if<!dummyKeepSorted, size_t>::type getIndexOnAdd(const T& val)
		{
			static_assert(false, "Only sorted Lists can getIndexWhenPushedBack!");
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
				if (m_data[0].value > val)
				{
					return 0;
				}
				else
				{
					return 1;
				}
			}
			if (val < m_data[0].value)
			{
				return 0;
			}
			if (val > m_data[m_length - 1].value)
			{
				return m_length;
			}

			size_t smallIndex = 0;
			size_t bigIndex = m_length - 1;
			size_t middleIndex;

			while (smallIndex <= bigIndex)
			{
				size_t searchSpace = bigIndex - smallIndex;
				middleIndex = smallIndex + searchSpace / 2;
				if (m_data[middleIndex].value == val)
				{
					return middleIndex;
				}
				else
				{
					if (m_data[middleIndex].value > val)
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
			static_assert(false, "Only sorted Lists can getIndexWhenPushedBack!");
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
				if (m_data[0].value > val)
				{
					leftNeighbor = nullptr;
					rightNeighbor = reinterpret_cast<T*>(m_data);
					return;
				}
				else
				{
					leftNeighbor = reinterpret_cast<T*>(m_data);
					rightNeighbor = nullptr;
					return;
				}
			}

			size_t index = getIndexOnAdd(val);
			while (m_data[index].value == val && index < m_length - 1)
			{
				index++;
			}

			if (index == 0)
			{
				leftNeighbor = nullptr;
				rightNeighbor = reinterpret_cast<T*>(m_data);
				return;
			}
			if (index >= m_length)
			{
				leftNeighbor = reinterpret_cast<T*>(m_data + m_length - 1);
				rightNeighbor = nullptr;
				return;
			}

			leftNeighbor = reinterpret_cast<T*>(m_data + index - 1);
			rightNeighbor = reinterpret_cast<T*>(m_data + index);

			return;
		}

		template <typename T>
		void addAll(T&& t)
		{
			add(std::forward<T>(t));
		}

		template<typename T, typename... arguments>
		void addAll(T&& t, arguments&&... args)
		{
			add(std::forward<T>(t));
			addAll(std::forward<arguments>(args)...);
		}

		void addAll(T* data, size_t size)
		{
			//UNTESTED
			for (size_t i = 0; i < size; i++)
			{
				add(data[i]);
			}
		}

		template<int size>
		void addAll(Array<T, size>& arr)
		{
			//UNTESTED
			addAll(arr.getRaw(), size);
		}

		void addAll(DynamicArray<T>& arr)
		{
			//UNTESTED
			addAll(arr.getRaw(), arr.getLength());
		}

		void popBack(size_t amount = 1)
		{
			for (size_t i = 0; i < amount; i++)
			{
				m_data[m_length - 1 - i].value.~T();
			}
			m_length -= amount;
		}

		void clear()
		{
			for (size_t i = 0; i < m_length; i++)
			{
				(&(m_data[i].value))->~T();
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
				if (m_data != nullptr)
				{
					delete[] m_data;
				}
				m_data = nullptr;
				return true;
			}
			INTERNAL::Unconstructed<T>* newList = new INTERNAL::Unconstructed<T>[m_length];
			for (size_t i = 0; i < m_length; i++)
			{
				new (bbe::addressOf(newList[i])) T(std::move(m_data[i].value));
			}
			if (m_data != nullptr)
			{
				for (size_t i = 0; i < m_length; i++)
				{
					bbe::addressOf(m_data[i].value)->~T();
				}
				delete[] m_data;
			}
			m_data = newList;
			return true;
		}

		template <typename dummyT = T>
		typename std::enable_if<std::is_default_constructible<dummyT>::value, void>::type
			resizeCapacityAndLength(size_t newCapacity)
		{
			//UNTESTED
			static_assert(std::is_same<dummyT, T>::value, "Do not specify dummyT!");
			resizeCapacity(newCapacity);
			add(T(), newCapacity - m_length);
		}

		void resizeCapacity(size_t newCapacity)
		{
			//UNTESTED
			if (newCapacity < m_length)
			{
				//TODO add further error handling
				debugBreak();
				return;
			}

			if (newCapacity == m_capacity)
			{
				return;
			}

			INTERNAL::Unconstructed<T>* newList = new INTERNAL::Unconstructed<T>[newCapacity];
			for (size_t i = 0; i < m_length; i++)
			{
				new (bbe::addressOf(newList[i])) T(std::move(m_data[i].value));
			}
			if (m_data != nullptr) {
				for (size_t i = 0; i < m_length; i++) {
					bbe::addressOf(m_data[i].value)->~T();
				}
				delete[] m_data;
			}
			m_data = newList;
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
				if (predicate(m_data[i].value))
				{
					moveRange++;
				}
				else if (moveRange != 0)
				{
					m_data[i - moveRange].value = std::move(m_data[i].value);
				}
			}
			m_length -= moveRange;
			return moveRange;
		}

		bool removeIndex(size_t index) {
			if (index >= m_length) {
				return false;
			}

			m_data[index].value.~T();
			if (index != m_length - 1)
			{
				new (bbe::addressOf(m_data[index].value)) T(std::move(m_data[index + 1].value));

				for (size_t i = index + 1; i < m_length - 1; i++)
				{
					m_data[i].value = std::move(m_data[i + 1].value);
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
				if (predicate(m_data[i].value))
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
				if (predicate(m_data[i].value))
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
				if (predicate(m_data[i].value))
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

		void sort()
		{
			sortSTL(reinterpret_cast<T*>(m_data), reinterpret_cast<T*>(m_data + m_length));
		}

		void sort(std::function<bool(const T&, const T&)> predicate)
		{
			sortSTL(reinterpret_cast<T*>(m_data), reinterpret_cast<T*>(m_data + m_length), predicate);
		}

		T& first()
		{
			//UNTESTED
			if (m_data == nullptr)
			{
				//TODO error handling
				debugBreak();
			}

			return (m_data[0].value);
		}

		T& last()
		{
			//UNTESTED
			if (m_data == nullptr)
			{
				//TODO error handling
				debugBreak();
			}

			return (m_data[m_length - 1].value);
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
				if (predicate(m_data[i].value))
				{
					return reinterpret_cast<T*>(m_data + i);
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
				if (predicate(m_data[i].value))
				{
					return reinterpret_cast<T*>(m_data + i);
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
				if (m_data[i].value != other.m_data[i].value)
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
}
#pragma once

#include <functional>
#include "../BBE/STLCapsule.h"
#include "../BBE/Array.h"
#include "../BBE/Unconstructed.h"
#include "../BBE/UtilDebug.h"
#include "../BBE/Hash.h"
#include "../BBE/Exceptions.h"
#include "../BBE/MersenneTwister.h"
#include <initializer_list>
#include <iostream>
#include <cstring>
#include <limits>

namespace bbe
{
	template <typename T>
	class List
	{
	private:
		size_t m_length;
		size_t m_capacity;
		INTERNAL::Unconstructed<T>* m_pdata;

		INTERNAL::Unconstructed<T>* growIfNeeded(size_t amountOfNewObjects)
		{
			INTERNAL::Unconstructed<T>* retVal = nullptr;
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

				retVal = m_pdata;
				m_pdata = newData;
				m_capacity = newCapacity;
			}
			return retVal;
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

		List(const List<T>& other)
			: m_length(other.m_length), m_capacity(other.m_capacity)
		{
			m_pdata = new INTERNAL::Unconstructed<T>[m_capacity];
			for (size_t i = 0; i < m_length; i++)
			{
				new (bbe::addressOf(m_pdata[i])) T(other.m_pdata[i].m_value);
			}
		}

		List(List<T>&& other) noexcept
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

		List& operator=(const List<T>& other)
		{
			clear();

			if (m_pdata != nullptr)
			{
				delete[] m_pdata;
			}

			m_length = other.m_length;
			m_capacity = other.m_capacity;
			m_pdata = new INTERNAL::Unconstructed<T>[m_capacity];
			for (size_t i = 0; i < m_length; i++)
			{
				new (bbe::addressOf(m_pdata[i])) T(other.m_pdata[i].m_value);
			}

			return *this;
		}

		List& operator=(List<T>&& other) noexcept
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
			return getRaw()[index];
		}

		const T& operator[](size_t index) const
		{
			if (index >= m_length)
			{
				debugBreak();
			}
			return getRaw()[index];
		}

		List<T>& operator+=(const List<T>& other)
		{
			const T* optr = other.getRaw();
			for (size_t i = 0; i < other.m_length; i++)
			{
				add(optr[i]);
			}
			return *this;
		}

		void add(const T& val, size_t amount = 1)
		{
			auto delVal = growIfNeeded(amount);
			T* d = getRaw();
			for (size_t i = 0; i < amount; i++)
			{
				new (bbe::addressOf(d[m_length + i])) T(val);
			}
			m_length += amount;
			delete[] delVal;
		}

		void add(T&& val, size_t amount)
		{
			auto delVal = growIfNeeded(amount);
			T* d = getRaw();
			if (amount == 1)
			{
				new (bbe::addressOf(d[m_length])) T(std::move(val));
			}
			else
			{
				for (size_t i = 0; i < amount; i++)
				{
					new (bbe::addressOf(d[m_length + i])) T(val);
				}
			}

			m_length += amount;
			delete[] delVal;
		}

		void add(T&& val)
		{
			auto delVal = growIfNeeded(1);
			new (bbe::addressOf(getRaw()[m_length])) T(std::move(val));

			m_length += 1;
			delete[] delVal;
		}

		bool addUnique(const T& val)
		{
			if (!contains(val))
			{
				add(val);
				return true;
			}
			else
			{
				return false;
			}
		}

		template <typename U>
		void addAll(U&& t)
		{
			add(std::forward<U>(t));
		}
		
		void addList(const List<T>& other)
		{
			const size_t len = other.getLength();
			for (size_t i = 0; i < len; i++)
			{
				add(other[i]);
			}
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

		T popBack()
		{
			if (m_length < 1)
			{
				debugBreak();
			}
			T retVal = std::move(last());
			getRaw()[m_length - 1].~T();
			m_length--;
			return retVal;
		}

		void clear()
		{
			if (!std::is_trivially_destructible_v<T>)
			{
				T* d = getRaw();
				for (size_t i = 0; i < m_length; i++)
				{
					d[i].~T();
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
		typename std::enable_if<std::is_fundamental<dummyT>::value || std::is_pointer<dummyT>::value, void>::type
			resizeCapacityAndLengthUninit(size_t newCapacity)
		{
			//UNTESTED
			static_assert(std::is_same<dummyT, T>::value, "Do not specify dummyT!");
			if (newCapacity == m_length) return;

			delete[] m_pdata;
			m_pdata = new INTERNAL::Unconstructed<T>[newCapacity];
			m_length = newCapacity;
			m_capacity = newCapacity;
		}

		template <typename dummyT = T>
		typename std::enable_if<std::is_default_constructible<dummyT>::value, void>::type
			resizeCapacityAndLength(size_t newCapacity)
		{
			//UNTESTED
			static_assert(std::is_same<dummyT, T>::value, "Do not specify dummyT!");
			const size_t oldLength = getLength();
			resizeCapacity(newCapacity);
			if constexpr (std::is_trivially_constructible_v<T>)
			{
				m_length = newCapacity;
				if (oldLength != m_length)
				{
					std::memset(getRaw() + oldLength, 0, sizeof(T) * (m_length - oldLength));
				}
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

			if (newCapacity <= m_capacity)
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
			T* d = getRaw();
			for (size_t i = 0; i < m_length; i++)
			{
				if (predicate(d[i]))
				{
					d[i].~T();
					moveRange++;
				}
				else if (moveRange != 0)
				{
					d[i - moveRange] = std::move(d[i]);
				}
			}
			m_length -= moveRange;
			return moveRange;
		}

		bool removeIndex(size_t index) {
			if (index >= m_length) {
				return false;
			}

			T* d = getRaw();
			for (size_t i = index; i < m_length - 1; i++)
			{
				d[i] = std::move(d[i + 1]);
			}
			d[m_length - 1].~T();

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
			T* d = getRaw();
			for (size_t i = 0; i < m_length; i++)
			{
				if (predicate(d[i]))
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
			const T* d = getRaw();
			for (size_t i = 0; i < m_length; i++)
			{
				if (predicate(d[i]))
				{
					amount++;
				}
			}
			return amount;
		}

		size_t getBiggestIndex() const
		{
			if (getLength() == 0) return (size_t)-1;

			size_t biggestIndex = 0;

			for (size_t i = 1; i < getLength(); i++)
			{
				if (operator[](i) > operator[](biggestIndex))
				{
					biggestIndex = i;
				}
			}

			return biggestIndex;
		}

		bool swap(size_t a, size_t b)
		{
			if (a == b) return false;
			if (a >= m_length) return false;
			if (b >= m_length) return false;

			T temp = operator[](a);
			operator[](a) = operator[](b);
			operator[](b) = temp;

			return true;
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
			const T* d = getRaw();
			for (size_t i = 0; i < m_length; i++)
			{
				if (predicate(d[i]))
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

		bool any(std::function<bool(const T&)> predicate) const
		{
			return contains(predicate);
		}

		bool all(std::function<bool(const T&)> predicate) const
		{
			const T* d = getRaw();
			for (size_t i = 0; i < m_length; i++)
			{
				if (!predicate(d[i]))
				{
					return false;
				}
			}
			return true;
		}

		T* begin()
		{
			if (!m_length) return nullptr;
			return getRaw();
		}

		const T* begin() const
		{
			if (!m_length) return nullptr;
			return getRaw();
		}

		T* end()
		{
			if (!m_length) return nullptr;
			return getRaw() + getLength();
		}

		const T* end() const
		{
			if (!m_length) return nullptr;
			return getRaw() + getLength();
		}

		void sort()
		{
			sortSTL(begin(), end());
		}

		void sort(std::function<bool(const T&, const T&)> predicate)
		{
			sortSTL(begin(), end(), predicate);
		}

		void partition(std::function<bool(const T&)> predicate)
		{
			std::partition(begin(), end(), predicate);
		}

		void shuffle()
		{
			bbe::mt19937 twister;
			std::shuffle(begin(), end(), twister);
		}

		void shuffle(uint32_t seed)
		{
			bbe::mt19937 twister(seed);
			std::shuffle(begin(), end(), twister);
		}

		T& first()
		{
			//UNTESTED
			T* d = getRaw();
			if (!d)
			{
				throw ContainerEmptyException();
			}

			return *d;
		}

		const T& first() const
		{
			//UNTESTED
			const T* d = getRaw();
			if (!d)
			{
				throw ContainerEmptyException();
			}

			return *d;
		}

		T& last()
		{
			//UNTESTED
			T* d = getRaw();
			if (!d)
			{
				throw ContainerEmptyException();
			}

			return *(d + getLength() - 1);
		}

		const T& last() const
		{
			//UNTESTED
			const T* d = getRaw();
			if (!d)
			{
				throw ContainerEmptyException();
			}

			return *(d + getLength() - 1);
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
			T* d = getRaw();
			for (size_t i = 0; i < m_length; i++)
			{
				if (predicate(d[i]))
				{
					return d + i;
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
			T* d = getRaw();
			for (size_t i = m_length - 1; i >= 0 && i != std::numeric_limits<size_t>::max(); i--)
			{
				if (predicate(d[i]))
				{
					return d + i;
				}
			}
			return nullptr;
		}

		bool operator==(const List<T>& other) const
		{
			if (m_length != other.m_length)
			{
				return false;
			}

			const T* d = getRaw();
			const T* t = other.getRaw();
			for (size_t i = 0; i < m_length; i++)
			{
				if (d[i] != t[i])
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

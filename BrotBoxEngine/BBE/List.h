#pragma once

#include <functional>
#include "../BBE/STLCapsule.h"
#include "../BBE/Array.h"
#include "../BBE/Unconstructed.h"
#include "../BBE/UtilDebug.h"
#include "../BBE/Hash.h"
#include "../BBE/Error.h"
#include "../BBE/MersenneTwister.h"
#include "../BBE/AllocBlock.h"
#include <initializer_list>
#include <iostream>
#include <cstring>
#include <limits>
#include <mutex>
#include <utility>
#include <type_traits>

namespace bbe
{
	template <typename T>
	class List
	{
	public:
		using SubType = T;

	private:
		size_t m_length = 0;
		bbe::AllocBlock m_allocBlock;

		bbe::AllocBlock growIfNeeded(size_t amountOfNewObjects)
		{
			bbe::AllocBlock retVal;
			if (getCapacity() < m_length + amountOfNewObjects)
			{
				size_t newCapacity = m_length + amountOfNewObjects;
				if (newCapacity < getCapacity() * 2)
				{
					newCapacity = getCapacity() * 2;
				}

				bbe::AllocBlock newData = bbe::allocateBlock(newCapacity * sizeof(T));
				T* newDataPtr = (T*)newData.data;
				T* oldDataPtr = getRaw();

				for (size_t i = 0; i < m_length; i++)
				{
					new (bbe::addressOf(newDataPtr[i])) T(std::move(oldDataPtr[i]));
					oldDataPtr[i].~T();
				}

				retVal = m_allocBlock;
				m_allocBlock = newData;
			}
			return retVal;
		}

	public:
		List()
			: m_length(0), m_allocBlock()
		{
			//DO NOTHING
		}

		explicit List(size_t amountOfData)
			: m_length(0)
		{
			growIfNeeded(amountOfData);
		}

		template <typename... arguments>
		List(size_t amountOfObjects, arguments&&... args)
		{
			growIfNeeded(amountOfObjects);
			m_length = amountOfObjects;
			const T copyVal = T(std::forward<arguments>(args)...);
			for (size_t i = 0; i < amountOfObjects; i++)
			{
				new (bbe::addressOf(getRaw()[i])) T(copyVal);
			}
		}

		List(const List<T>& other)
		{
			growIfNeeded(other.m_length);
			m_length = other.m_length;
			for (size_t i = 0; i < m_length; i++)
			{
				new (bbe::addressOf(getRaw()[i])) T(other.getRaw()[i]);
			}
		}

		List(List<T>&& other) noexcept
			: m_length(other.m_length), m_allocBlock(std::move(other.m_allocBlock))
		{
			other.m_length = 0;
			other.m_allocBlock = {};
		}

		/*nonexplicit*/ List(const std::initializer_list<T> &il)
		{
			growIfNeeded(il.size());
			for (auto iter = il.begin(); iter != il.end(); iter++) {
				add(*iter);
			}
		}

		List& operator=(const List<T>& other)
		{
			clear();

			bbe::freeBlock(m_allocBlock);
			m_length = 0;
			growIfNeeded(other.getCapacity());
			m_length = other.m_length;

			for (size_t i = 0; i < m_length; i++)
			{
				new (bbe::addressOf(getRaw()[i])) T(other.getRaw()[i]);
			}

			return *this;
		}

		List& operator=(List<T>&& other) noexcept
		{
			clear();

			bbe::freeBlock(m_allocBlock);

			m_length = other.m_length;
			m_allocBlock = other.m_allocBlock;

			other.m_length = 0;
			other.m_allocBlock = {};

			return *this;
		}

		~List()
		{
			clear();

			bbe::freeBlock(m_allocBlock);
			m_length = 0;
		}

		size_t getCapacity() const
		{
			return m_allocBlock.size / sizeof(T);
		}

		size_t getLength() const
		{
			return m_length;
		}

		T* getRaw()
		{
			return reinterpret_cast<T*>(m_allocBlock.data);
		}

		const T* getRaw() const
		{
			//UNTESTED
			return reinterpret_cast<const T*>(m_allocBlock.data);
		}

		void* getVoidRaw()
		{
			return (void*)getRaw();
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
			bbe::freeBlock(delVal);
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
			bbe::freeBlock(delVal);
		}

		void add(T&& val)
		{
			auto delVal = growIfNeeded(1);
			new (bbe::addressOf(getRaw()[m_length])) T(std::move(val));

			m_length += 1;
			bbe::freeBlock(delVal);
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

		T* addRaw(size_t size)
		{
			auto delVal = growIfNeeded(size);
			T* retVal = getRaw() + m_length;
			for (size_t i = 0; i < size; i++)
			{
				new (bbe::addressOf(getRaw()[m_length])) T();
				m_length += 1;
			}
			bbe::freeBlock(delVal);
			return retVal;
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

		T popFront()
		{
			if (m_length < 1)
			{
				debugBreak();
			}
			T retVal = std::move(first());
			removeIndex(0);
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

		template <typename dummyT = T>
		typename std::enable_if<std::is_fundamental<dummyT>::value || std::is_pointer<dummyT>::value, void>::type
			resizeCapacityAndLengthUninit(size_t newCapacity)
		{
			//UNTESTED
			static_assert(std::is_same<dummyT, T>::value, "Do not specify dummyT!");
			if (newCapacity == m_length) return;

			bbe::freeBlock(m_allocBlock);
			growIfNeeded(newCapacity);
			m_length = newCapacity;
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
				debugBreak();
				bbe::Crash(bbe::Error::IllegalArgument);
			}

			if (newCapacity <= getCapacity())
			{
				return;
			}

			auto delVal = growIfNeeded(newCapacity - getCapacity());
			bbe::freeBlock(delVal);
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
				bbe::Crash(bbe::Error::ContainerEmpty);
			}

			return *d;
		}

		const T& first() const
		{
			//UNTESTED
			const T* d = getRaw();
			if (!d)
			{
				bbe::Crash(bbe::Error::ContainerEmpty);
			}

			return *d;
		}

		T& last()
		{
			//UNTESTED
			T* d = getRaw();
			if (!d)
			{
				bbe::Crash(bbe::Error::ContainerEmpty);
			}

			return *(d + getLength() - 1);
		}

		const T& last() const
		{
			//UNTESTED
			const T* d = getRaw();
			if (!d)
			{
				bbe::Crash(bbe::Error::ContainerEmpty);
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
			for (size_t i = m_length - 1; i != std::numeric_limits<size_t>::max(); i--)
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

		bool operator!=(const List<T>& other) const
		{
			return !(operator==(other));
		}


	};

	template<typename T>
	class ConcurrentList
	{
	private:
		bbe::List<T> m_data;
		mutable std::recursive_mutex m_mutex;
		mutable int64_t m_lockCount = 0;

		void enforceLocked(const char* msg) const
		{
			if (m_lockCount == 0)
			{
				bbe::Crash(bbe::Error::IllegalState, msg);
			}
			else if (m_lockCount < 0)
			{
				bbe::Crash(bbe::Error::IllegalState, "Negative m_lockCount detected!");
			}
		}

	public:
		ConcurrentList() = default;
		/*nonexplicit*/ ConcurrentList(const std::initializer_list<T>& il) :
			m_data(il)
		{}

		T& operator[](size_t index)
		{
			enforceLocked("ConcurrentList: operator[]");
			return m_data[index];
		}

		const T& operator[](size_t index) const
		{
			enforceLocked("ConcurrentList: operator[] const");
			return m_data[index];
		}

		T& getUnprotected(size_t index)
		{
			return m_data[index];
		}

		const T& getUnprotected(size_t index) const
		{
			return m_data[index];
		}

		void add(const T& t)
		{
			std::lock_guard lg(m_mutex);
			m_data.add(t);
		}

		void add(T&& t)
		{
			std::lock_guard lg(m_mutex);
			m_data.add(t);
		}

		size_t getLength() const
		{
			std::lock_guard lg(m_mutex);
			return m_data.getLength();
		}

		void lock() const
		{
			m_mutex.lock();
			++m_lockCount;
		}

		void unlock() const
		{
			--m_lockCount;
			m_mutex.unlock();
		}

		bbe::List<T>& getUnderlying()
		{
			enforceLocked("ConcurrentList: getUnderlying");
			return m_data;
		}

		T popFront()
		{
			std::lock_guard lg(m_mutex);
			return m_data.popFront();
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

	template<typename>
	struct IsList : std::false_type {};

	template<typename T>
	struct IsList<bbe::List<T>> : std::true_type {};
}

#pragma once

#include <memory>
#include <algorithm>

namespace bbe {
	template <typename T>
	union ListChunk
	{
		//this little hack prevents the constructor of T to be called
		//allows the use of new and its auto alignment features
		T value;

		ListChunk() {}
		~ListChunk() {}
	};

	template <typename T>
	class List {
		//TODO use own allocators
	private:
		size_t m_length;
		size_t m_capacity;
		ListChunk<T>* m_data;

		void growIfNeeded(size_t amountOfNewObjects)
		{
			if (m_capacity < m_length + amountOfNewObjects) {
				size_t newCapacity = m_length + amountOfNewObjects;
				if (newCapacity < m_capacity * 2) {
					newCapacity = m_capacity * 2;
				}

				ListChunk<T>* newData = new ListChunk<T>[newCapacity];

				for (size_t i = 0; i < m_length; i++) {
					newData[i].value = std::move(m_data[i].value);
					m_data[i].value.~T();
				}

				delete[] m_data;
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
			m_data = new ListChunk<T>[amountOfObjects];
			for (size_t i = 0; i < amountOfObjects; i++) {
				new (std::adressof(m_data[i])) T(std::forward<arguments>(args)...);
			}
		}

		List(const List<T>& other)
			: m_length(other.m_length), m_capacity(other.m_capacity)
		{
			m_data = new ListChunk<T>[m_capacity];
			for (size_t i = 0; i < m_capacity; i++) {
				m_data[i] = other.m_data[i];
			}
		}

		List(List<T>&& other)
			: m_length(other.m_length), m_capacity(other.m_capacity), m_data(other.m_data)
		{
			other.m_data = nullptr;
			other.m_length = 0;
			other.m_capacity = 0;
		}

		List& operator=(const List<T>& other) {
			if (m_data != nullptr) {
				delete[] m_data;
			}

			m_length = other.m_length;
			m_capacity = other.m_capacity;
			m_data = new ListChunk<T>[m_capacity];
			for (size_t i = 0; i < m_capacity; i++) {
				m_data[i] = other.m_data[i];
			}
		}

		List& operator=(List<T>&& other) {
			if (m_data != nullptr) {
				delete[] m_data;
			}

			m_length = other.m_length;
			m_capacity = other.m_capacity;
			m_data = other.m_data;

			other.m_data = nullptr;
			other.m_length = 0;
			other.m_capacity = 0;
		}

		~List() {
			clear();

			if (m_data != nullptr) {
				delete[] m_data;
			}

			m_data = nullptr;
			m_length = 0;
			m_capacity = 0;
		}

		size_t getCapacity() const {
			return m_capacity;
		}

		size_t getLength() const {
			return m_length;
		}

		T* getRaw() {
			return reinterpret_cast<T*>(m_data);
		}

		bool isEmpty() const {
			return m_length == 0;
		}

		T& operator[](size_t index) {
			return m_data[index].value;
		}

		const T& operator[](size_t index) const{
			return m_data[index].value;
		}

		List<T>& operator+=(List<T> other) {
			for (size_t i = 0; i < other.m_length; i++) {
				pushBack(other.m_data[i].value);
			}
		}

		void pushBack(const T& val, size_t amount = 1) {
			growIfNeeded(amount);
			for (size_t i = 0; i < amount; i++) {
				m_data[m_length + i].value = val;
			}
			m_length += amount;
		}

		void popBack(size_t amount = 1) {
			for (size_t i = 0; i < amount; i++) {
				m_data[m_length - 1 - i].value.~T();
			}
			m_length -= amount;
		}

		void clear() {
			for (size_t i = 0; i < m_length; i++) {
				m_data[i].~T();
			}
			m_length = 0;
		}
		
		bool shrink() {
			if (m_length == m_capacity) {
				return false;
			}
			List<T> newList = new List<T>[m_length];
			for (size_t i = 0; i < m_length; i++) {
				newList[i].value = std::move(m_data[i].value);
			}
			delete[] m_data;
			m_data = newList;
			return true;
		}

		size_t removeAll(const T& remover) {
			return removeAll([&remover](const &T other) { other == remover });
		}

		size_t removeAll(bool(*predicate)(T&)) {
			size_t moveRange = 0;
			for (size_t i = 0; i < m_length; i++) {
				if (predicate(m_data[i].value)) {
					moveRange++;
				}
				else if (moveRange != 0) {
					m_data[i - moveRange].value = std::move(m - data[i]);
				}
			}
			m_length -= moveRange;
		}

		bool removeSingle(const T& remover) {
			removeSingle([&remover](const T& t) {remover == t});
		}

		bool removeSingle(bool(*predicate)(T&)) {
			size_t index = 0;
			bool found = false;
			for (size_t i = 0; i < m_length; i++) {
				if (predicate(m_data[i].value)) {
					index = i;
					found = true;
					break;
				}
			}
			if (!found) return false;

			m_data[index].value.~T();
			for (size_t i = index; i < m_length - 1; i++) {
				m_data[index].value = std::move(m_data[index + 1].value);
			}
		}

		size_t containsAmount(const T& t) const {
			return containsAmount([&t](const T& other) {t == other});
		}

		size_t containsAmount(bool(*predicate)(T&)) const {
			size_t amount = 0;
			for (size_t i = 0; i < m_length; i++) {
				if (predicate(m_data[i].value)) {
					amount++;
				}
			}
			return amount;
		}

		bool contains(const T& t) const {
			return contains([&t](const T& other) {t == other});
		}

		bool contains(bool(*predicate)(T&)) const {
			for (size_t i = 0; i < m_length; i++) {
				if (predicate(m_data[i].value)) {
					return true;
				}
			}
			return false;
		}

		bool containsUnique(const T& t) const {
			return containsAmount(t) == 1;
		}

		bool containsUnique(bool(*predicate)(T&)) const {
			return containsAmount(predicate) == 1;
		}

		void sort() {
			std::sort(reinterpret_cast<*T>(m_data), reinterpret_cast<*T>(m_data + m_length));
		}

		void sort(bool(*predicate)(T&, T&)) {
			std::sort(reinterpret_cast<*T>(m_data), reinterpret_cast<*T>(m_data + m_length), predicate);
		}

		T* find(const T& t) {
			return find([&t](const T& other) { t == other });
		}

		T* find(bool(*predicate)(T&)) {
			for (size_t i = 0; i < m_length; i++) {
				if (predicate(m_data[i].value)) {
					return reinterpret_cast<T*>(m_data + i);
				}
			}
		}

		T* findLast(const T& t) {
			return findLast([&t](const T& other) { t == other });
		}

		T* findLast(bool(*predicate)(T&)) {
			for (size_t i = m_length - 1; i >= 0; i--) {
				if (predicate(m_data[i].value)) {
					return reinterpret_cast<T*>(m_data + i);
				}
			}
		}

		bool operator==(const List<T>& other) {
			if (m_length != other.m_length) {
				return false;
			}

			for (size_t i = 0; i < m_length; i++) {
				if (m_data[i].value != other.m_data[i].value) {
					return false;
				}
			}

			return true;
		}

		bool operator!=(const List<T>& other) {
			return !(operator==(other));
		}


	};
}
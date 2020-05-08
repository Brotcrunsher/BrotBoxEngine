#pragma once

template<typename T>
class SomeClass
{
private:
	T* m_pdata = nullptr;
	size_t m_length = 0;

	void init(size_t length)
	{
		if (m_pdata != nullptr)
		{
			delete[] m_pdata;
			m_pdata = nullptr;
		}

		m_length = length;
		m_pdata = new T[m_length]{ 0 };
	}

	void copyDataFrom(const SomeClass<T>& other)
	{
		init(other.m_length);
		for (size_t i = 0; i < m_length; i++)
		{
			m_pdata[i] = other.m_pdata[i];
		}
	}

	void stealDataFrom(SomeClass<T>&& other)
	{
		m_pdata = other.m_pdata;
		m_length = other.m_length;

		other.m_pdata = nullptr;
		other.m_length = 0;
	}

	void cleanup()
	{
		if (m_pdata != nullptr)
		{
			delete[] m_pdata;
			m_pdata = nullptr;
			m_length = 0;
		}
	}

public:
	SomeClass()
	{
		init(10);
	}

	SomeClass(size_t length)
	{
		init(length);
	}

	SomeClass(const SomeClass<T>& other)
	{
		copyDataFrom(other);
	}

	SomeClass(SomeClass<T>&& other)
	{
		stealDataFrom(std::move(other));
	}

	SomeClass<T>& operator=(const SomeClass<T>& other)
	{
		cleanup();
		copyDataFrom(other);
		return *this;
	}

	SomeClass<T>& operator=(SomeClass<T>&& other)
	{
		cleanup();
		stealDataFrom(std::move(other));
		return *this;
	}

	bool operator<(const SomeClass<T>& other) const
	{
		return m_length < other.m_length;
	}

	bool operator>(const SomeClass<T>& other) const
	{
		return m_length > other.m_length;
	}

	bool operator<=(const SomeClass<T>& other) const
	{
		return m_length <= other.m_length;
	}

	bool operator>=(const SomeClass<T>& other) const
	{
		return m_length >= other.m_length;
	}

	bool operator==(const SomeClass<T>& other) const
	{
		return m_length == other.m_length;
	}

	bool operator!=(const SomeClass<T>& other) const
	{
		return m_length != other.m_length;
	}

	size_t getLength() const
	{
		return m_length;
	}

	void resize(size_t length)
	{
		init(length);
	}

	~SomeClass()
	{
		cleanup();
	}
};

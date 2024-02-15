#include "BBE/Utf8Iterator.h"
#include "BBE/Utf8Helpers.h"

bbe::Utf8Iterator::Utf8Iterator()
	: m_pdata(nullptr)
{
}

bbe::Utf8Iterator::Utf8Iterator(const char* ptr)
	: m_pdata(ptr)
{
}

bbe::Utf8Iterator& bbe::Utf8Iterator::operator++()
{
	m_pdata = bbe::utf8GetNextChar(m_pdata);
	return *this;
}

bbe::Utf8Iterator bbe::Utf8Iterator::operator++(int)
{
	const auto copy = *this;
	++(*this);
	return copy;
}

bbe::Utf8Iterator& bbe::Utf8Iterator::operator--()
{
	m_pdata--;
	m_pdata = bbe::utf8GetStartAddrOfCodePoint(m_pdata);
	return *this;
}

bbe::Utf8Iterator bbe::Utf8Iterator::operator--(int)
{
	const auto copy = *this;
	--(*this);
	return copy;
}

bbe::Utf8Iterator bbe::Utf8Iterator::operator+(int32_t advancement) const
{
	if (advancement < 0) return operator-(-advancement);
	auto retVal = *this;
	for (int32_t i = 0; i < advancement; i++)
	{
		++retVal;
	}
	return retVal;
}

bbe::Utf8Iterator& bbe::Utf8Iterator::operator+=(int32_t advancement)
{
	if (advancement < 0) return operator-=(-advancement);
	for (int32_t i = 0; i < advancement; i++)
	{
		++(*this);
	}
	return *this;
}

bbe::Utf8Iterator bbe::Utf8Iterator::operator-(int32_t advancement) const
{
	if (advancement < 0) return operator+(-advancement);
	auto retVal = *this;
	for (int32_t i = 0; i < advancement; i++)
	{
		--retVal;
	}
	return retVal;
}

bbe::Utf8Iterator& bbe::Utf8Iterator::operator-=(int32_t advancement)
{
	if (advancement < 0) return operator+=(-advancement);
	for (int32_t i = 0; i < advancement; i++)
	{
		--(*this);
	}
	return *this;
}

int32_t bbe::Utf8Iterator::operator-(const bbe::Utf8Iterator& other) const
{
	if (m_pdata > other.m_pdata)
	{
		return -other.operator-(*this);
	}

	auto copy = *this;
	int32_t dist = 0;
	while (copy != other)
	{
		dist--;
		copy++;
	}
	return dist;
}

int32_t bbe::Utf8Iterator::getCodepoint() const
{
	return utf8CharToCodePoint(m_pdata);
}

bool bbe::Utf8Iterator::valid() const
{
	return getCodepoint() != 0;
}

bool bbe::Utf8Iterator::operator==(const bbe::Utf8Iterator& other) const
{
	return m_pdata == other.m_pdata;
}

bool bbe::Utf8Iterator::operator!=(const bbe::Utf8Iterator& other) const
{
	return m_pdata != other.m_pdata;
}

bool bbe::Utf8Iterator::operator<=(const bbe::Utf8Iterator& other) const
{
	return m_pdata <= other.m_pdata;
}

bool bbe::Utf8Iterator::operator>=(const bbe::Utf8Iterator& other) const
{
	return m_pdata >= other.m_pdata;
}

bool bbe::Utf8Iterator::operator<(const bbe::Utf8Iterator& other) const
{
	return m_pdata < other.m_pdata;
}

bool bbe::Utf8Iterator::operator>(const bbe::Utf8Iterator& other) const
{
	return m_pdata > other.m_pdata;
}

bbe::Utf8Iterator::operator const char* () const
{
	return m_pdata;
}

bool bbe::Utf8Iterator::operator==(const char* other) const
{
	return m_pdata == other;
}

bool bbe::Utf8Iterator::operator!=(const char* other) const
{
	return m_pdata != other;
}

bool bbe::Utf8Iterator::operator<=(const char* other) const
{
	return m_pdata <= other;
}

bool bbe::Utf8Iterator::operator>=(const char* other) const
{
	return m_pdata >= other;
}

bool bbe::Utf8Iterator::operator<(const char* other) const
{
	return m_pdata < other;
}

bool bbe::Utf8Iterator::operator>(const char* other) const
{
	return m_pdata > other;
}

const char& bbe::Utf8Iterator::operator*() const
{
	return *m_pdata;
}

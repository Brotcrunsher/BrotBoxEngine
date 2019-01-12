#include "stdafx.h"
#include "BBE/String.h"
#include "BBE/DataType.h"
#include "BBE/Exceptions.h"
#include <string>

void bbe::Utf8String::growIfNeeded(std::size_t newSize)
{
	if(getCapacity() < newSize)
	{
		size_t newCapa = newSize * 2;
		char *newData = new char[newCapa];
		memcpy(newData, getRaw(), getLengthBytes() + 1);

		if(!m_usesSSO)
		{
			delete[] getRaw();
		}
		else
		{
			m_usesSSO = false;
		}

		m_capacity = newCapa;
		m_UNION.m_pdata = newData;
	}
}

void bbe::Utf8String::initializeFromCharArr(const char* data)
{
	m_length = utf8len(data);

	if(m_length < BBE_UTF8STRING_SSOSIZE - 1)
	{
		memcpy(m_UNION.m_ssoData, data, m_length + 1);
		m_usesSSO = true;
		m_capacity = BBE_UTF8STRING_SSOSIZE;
	}
	else
	{
		m_UNION.m_pdata = new char[m_length + 1];
		memcpy(m_UNION.m_pdata, data, m_length + 1);
		m_usesSSO = false;
		m_capacity = m_length + 1;
	}
}

bbe::Utf8String::Utf8String()
{
	//UNTESTED
	initializeFromCharArr(u8"");
}

bbe::Utf8String::Utf8String(const char* data)
{
	//UNTESTED
	initializeFromCharArr(data);
}

bbe::Utf8String::Utf8String(double number)
{
	//UNTESTED
	initializeFromCharArr(std::to_string(number).c_str());
}

bbe::Utf8String::Utf8String(int number)
{
	//UNTESTED
	initializeFromCharArr(std::to_string(number).c_str());
}

bbe::Utf8String::Utf8String(long long number)
{
	//UNTESTED
	initializeFromCharArr(std::to_string(number).c_str());
}

bbe::Utf8String::Utf8String(long double number)
{
	//UNTESTED
	initializeFromCharArr(std::to_string(number).c_str());
}

bbe::Utf8String::Utf8String(float number)
{
	//UNTESTED
	initializeFromCharArr(std::to_string(number).c_str());
}

bbe::Utf8String::Utf8String(unsigned long long number)
{
	//UNTESTED
	initializeFromCharArr(std::to_string(number).c_str());
}

bbe::Utf8String::Utf8String(unsigned long number)
{
	//UNTESTED
	initializeFromCharArr(std::to_string(number).c_str());
}

bbe::Utf8String::Utf8String(long number)
{
	//UNTESTED
	initializeFromCharArr(std::to_string(number).c_str());
}

bbe::Utf8String::Utf8String(unsigned int number)
{
	//UNTESTED
	initializeFromCharArr(std::to_string(number).c_str());
}

bbe::Utf8String::Utf8String(const Utf8String& other)//Copy Constructor
{ 
	//UNTESTED
	initializeFromCharArr(other.getRaw());
}

bbe::Utf8String::Utf8String(Utf8String&& other) //Move Constructor
{
	//UNTESTED
	m_length = other.m_length;
	m_usesSSO = other.m_usesSSO;
	if (m_usesSSO)
	{
		memcpy(m_UNION.m_ssoData, other.m_UNION.m_ssoData, BBE_UTF8STRING_SSOSIZE);
		m_capacity = BBE_UTF8STRING_SSOSIZE;
	}
	else
	{
		m_UNION.m_pdata = other.m_UNION.m_pdata;
		m_capacity = other.m_capacity;
		other.m_UNION.m_pdata = nullptr;
	}
	other.m_length = 0;
}

bbe::Utf8String& bbe::Utf8String::operator=(const Utf8String &other)//Copy Assignment
{
	//UNTESTED
	if (!m_usesSSO && m_UNION.m_pdata != nullptr)
	{
		delete[] m_UNION.m_pdata;
	}

	m_length = other.getLength();
	initializeFromCharArr(other.getRaw());
	return *this;
}

bbe::Utf8String& bbe::Utf8String::operator=(Utf8String &&other)//Move Assignment
{
	//UNTESTED
	if (!m_usesSSO && m_UNION.m_pdata != nullptr)
	{
		delete[] m_UNION.m_pdata;
	}
	
	m_length = other.m_length;
	m_usesSSO = other.m_usesSSO;
	if (m_usesSSO)
	{
		memcpy(m_UNION.m_ssoData, other.m_UNION.m_ssoData, BBE_UTF8STRING_SSOSIZE);
		m_capacity = BBE_UTF8STRING_SSOSIZE;
	}
	else
	{
		m_UNION.m_pdata = other.m_UNION.m_pdata;
		m_capacity = other.m_capacity;
		other.m_UNION.m_pdata = nullptr;
	}
	other.m_length = 0;
	return *this;
}

bbe::Utf8String::~Utf8String()
{
	//UNTESTED
	if (!m_usesSSO && m_UNION.m_pdata != nullptr)
	{
		delete[] m_UNION.m_pdata;
		m_UNION.m_pdata = nullptr;
	}
}

bool bbe::Utf8String::operator==(const Utf8String& other) const
{
	return strcmp(getRaw(), other.getRaw()) == 0;
}
bool bbe::Utf8String::operator==(const char*       other) const
{
	return strcmp(getRaw(), other) == 0;
}
bool bbe::operator==(const char* arr, const bbe::Utf8String& string)
{
	return strcmp(arr, string.getRaw()) == 0;
}
bool bbe::Utf8String::operator!=(const Utf8String& other) const
{
	return strcmp(getRaw(), other.getRaw()) != 0;
}
bool bbe::Utf8String::operator!=(const char*       other) const
{
	return strcmp(getRaw(), other) != 0;
}
bool bbe::operator!=(const char* arr, const bbe::Utf8String& string)
{
	return strcmp(arr, string.getRaw()) != 0;
}

std::ostream& bbe::operator<<(std::ostream &os, const bbe::Utf8String &string)
{
	return os << string.getRaw();
}

bbe::Utf8String bbe::Utf8String::operator+(const Utf8String& other) const
{
	//PO
	//UNTESTED
	size_t totalLength = m_length + other.m_length;
	Utf8String retVal;
	retVal.m_length = totalLength;

	if (totalLength + 1 < BBE_UTF8STRING_SSOSIZE)
	{
		memcpy(retVal.m_UNION.m_ssoData, getRaw(), m_length);
		memcpy(retVal.m_UNION.m_ssoData + m_length, other.getRaw(), other.m_length);
		retVal.m_UNION.m_ssoData[totalLength + 1] = '\0';
	}
	else
	{
		char *newData = new char[totalLength + 1];
		memcpy(newData, getRaw(), m_length);
		memcpy(newData + m_length, other.getRaw(), other.m_length);
		newData[totalLength] = 0;

		retVal.m_usesSSO = false;
		retVal.m_UNION.m_pdata = newData;
	}
	return retVal;
}

bbe::Utf8String bbe::Utf8String::operator+(const char* other) const
{
	//UNTESTED
	return operator+(Utf8String(other));
}

bbe::Utf8String bbe::Utf8String::operator+(double number) const
{
	//UNTESTED
	return operator+(Utf8String(number));
}

bbe::Utf8String bbe::Utf8String::operator+(int number) const
{
	//UNTESTED
	return operator+(Utf8String(number));
}

bbe::Utf8String bbe::Utf8String::operator+(long long number) const
{
	//UNTESTED
	return operator+(Utf8String(number));
}

bbe::Utf8String bbe::Utf8String::operator+(long double number) const
{
	//UNTESTED
	return operator+(Utf8String(number));
}

bbe::Utf8String bbe::Utf8String::operator+(float number) const
{
	//UNTESTED
	return operator+(Utf8String(number));
}

bbe::Utf8String bbe::Utf8String::operator+(unsigned long long number) const
{
	//UNTESTED
	return operator+(Utf8String(number));
}

bbe::Utf8String bbe::Utf8String::operator+(unsigned long number) const
{
	//UNTESTED
	return operator+(Utf8String(number));
}

bbe::Utf8String bbe::Utf8String::operator+(long number) const
{
	//UNTESTED
	return operator+(Utf8String(number));
}

bbe::Utf8String bbe::Utf8String::operator+(unsigned int number) const
{
	//UNTESTED
	return operator+(Utf8String(number));
}

bbe::Utf8String bbe::operator+(const char* other, const bbe::Utf8String& string)
{
	//UNTESTED
	return bbe::Utf8String(other) + string;
}

bbe::Utf8String bbe::operator+(double number, const bbe::Utf8String& string)
{
	//UNTESTED
	return bbe::Utf8String(number) + string;
}

bbe::Utf8String bbe::operator+(int number, const bbe::Utf8String& string)
{
	//UNTESTED
	return bbe::Utf8String(number) + string;
}

bbe::Utf8String bbe::operator+(long long number, const bbe::Utf8String& string)
{
	//UNTESTED
	return bbe::Utf8String(number) + string;
}

bbe::Utf8String bbe::operator+(long double number, const bbe::Utf8String& string)
{
	//UNTESTED
	return bbe::Utf8String(number) + string;
}

bbe::Utf8String bbe::operator+(float number, const bbe::Utf8String& string)
{
	//UNTESTED
	return bbe::Utf8String(number) + string;
}

bbe::Utf8String bbe::operator+(unsigned long long number, const bbe::Utf8String& string)
{
	//UNTESTED
	return bbe::Utf8String(number) + string;
}

bbe::Utf8String bbe::operator+(unsigned long number, const bbe::Utf8String& string)
{
	//UNTESTED
	return bbe::Utf8String(number) + string;
}

bbe::Utf8String bbe::operator+(long number, const bbe::Utf8String& string)
{
	//UNTESTED
	return bbe::Utf8String(number) + string;
}

bbe::Utf8String bbe::operator+(unsigned int number, const bbe::Utf8String& string)
{
	//UNTESTED
	return bbe::Utf8String(number) + string;
}

bbe::Utf8String& bbe::Utf8String::operator+=(const bbe::Utf8String& other)
{
	//UNTESTED
	size_t totalLength = m_length + other.m_length;
	size_t oldLength = m_length;
	m_length = totalLength;

	if(m_usesSSO && totalLength + 1 >= BBE_UTF8STRING_SSOSIZE)
	{
		char *newData = new char[totalLength + 1];
		memcpy(newData, getRaw(), oldLength);
		m_UNION.m_pdata = newData;
		m_usesSSO = false;
	}

	if (m_usesSSO) {
		memcpy(m_UNION.m_ssoData + oldLength, other.getRaw(), other.m_length);
		m_UNION.m_ssoData[totalLength] = 0;
	}
	else {
		growIfNeeded(totalLength + 1);
		memcpy(getRaw() + oldLength, other.getRaw(), sizeof(wchar_t) * other.m_length);
		m_UNION.m_pdata[totalLength] = 0;
	}
	return *this;
}

bbe::Utf8String& bbe::Utf8String::operator+=(const char* other)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(other));
}

bbe::Utf8String& bbe::Utf8String::operator+=(double number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String& bbe::Utf8String::operator+=(int number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String& bbe::Utf8String::operator+=(long long number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String& bbe::Utf8String::operator+=(long double number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String& bbe::Utf8String::operator+=(float number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String& bbe::Utf8String::operator+=(unsigned long long number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String& bbe::Utf8String::operator+=(unsigned long number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String& bbe::Utf8String::operator+=(long number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String& bbe::Utf8String::operator+=(unsigned int number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String bbe::Utf8String::trim() const
{
	//UNTESTED
	bbe::Utf8String copy = *this;
	copy.trimInPlace();
	return copy;
}

void bbe::Utf8String::trimInPlace()
{
	//UNTESTED
	size_t start = 0;
	size_t end = m_length;
	const char* raw = getRaw();

	while (utf8IsWhitespace(&raw[start]))
	{
		start ++;
	}
	while (utf8IsWhitespace(&raw[end]) && end != 0)
	{
		end--;
	}

	substringInPlace(start, end);
}

bbe::Utf8String bbe::Utf8String::substring(std::size_t start, std::size_t end) const
{
	//UNTESTED
	bbe::Utf8String copy = *this;
	copy.substringInPlace(start, end);
	return copy;
}

void bbe::Utf8String::substringInPlace(size_t start, size_t end)
{
	//UNTESTED
	if(end == (size_t)-1)
	{
		end = m_length;
	}
	auto raw = getRaw();
	if (start != 0 || end != m_length)
	{
		if (end == 0) //Special Case, if the string only contains whitespace
		{ 
			m_length = 0;
			raw[m_length] = 0;
		}
		else
		{
			m_length = end - start + 1;
			memmove(raw, &raw[start], m_length);
			raw[m_length] = 0;
		}
	}
}

size_t bbe::Utf8String::count(const Utf8String& countand) const
{
	//UNTESTED
	size_t countandLength = countand.getLength();
	if (countandLength == 0)
	{
		return 0;
	}
	size_t amount = 0;
	auto readHead = getRaw();

	while ((readHead = strstr(readHead, countand.getRaw())) != nullptr)
	{
		amount++;
		readHead += countandLength;
	}
	return amount;
}

size_t bbe::Utf8String::count(const char* countand) const
{
	//UNTESTED
	return count(bbe::Utf8String(countand));
}

bbe::DynamicArray<bbe::Utf8String> bbe::Utf8String::split(const bbe::Utf8String& splitAt) const
{
	//UNTESTED
	//TODO this method is a little mess. Clean it up!
	size_t counted = count(splitAt);
	if (counted == 0)
	{
		DynamicArray<Utf8String> retVal(1);
		retVal[0] = getRaw();
		return retVal;
	}
	DynamicArray<Utf8String> retVal(counted + 1);
	auto previousFinding = getRaw();
	for (size_t i = 0; i < retVal.getLength() - 1; i++)
	{
		const char *currentFinding = strstr(previousFinding, splitAt.getRaw());
		Utf8String currentString;
		size_t currentStringLength = currentFinding - previousFinding;
		currentString.m_usesSSO = false; //TODO make this better! current string could use SSO!
		currentString.m_UNION.m_pdata = new char[currentStringLength + 1];
		memcpy(currentString.m_UNION.m_pdata, previousFinding, currentStringLength);
		currentString.m_UNION.m_pdata[currentStringLength] = 0;
		currentString.m_length = currentStringLength;

		retVal[i] = currentString;

		previousFinding = currentFinding + splitAt.getLength();
	}

	Utf8String currentString;
	size_t currentStringLength = m_UNION.m_pdata + m_length - previousFinding;
	currentString.m_usesSSO = false; //TODO make this better! current string could use SSO!
	currentString.m_UNION.m_pdata = new char[currentStringLength + 1];
	memcpy(currentString.m_UNION.m_pdata, previousFinding, currentStringLength);
	currentString.m_UNION.m_pdata[currentStringLength] = 0;
	currentString.m_length = currentStringLength;
	retVal[retVal.getLength() - 1] = currentString;

	return retVal;
}

bbe::DynamicArray<bbe::Utf8String> bbe::Utf8String::split(const char* splitAt) const
{
	//UNTESTED
	return split(bbe::Utf8String(splitAt));
}

bool bbe::Utf8String::contains(const char* string) const
{
	//UNTESTED
	return strstr(getRaw(), string) != nullptr;
}

bool bbe::Utf8String::contains(const Utf8String &string) const
{
	//UNTESTED
	return contains(string.getRaw());
}

int64_t bbe::Utf8String::search(const char* string) const
{
	//UNTESTED
	const char* firstOcc = strstr(getRaw(), string);
	if(firstOcc == nullptr)
	{
		return -1;
	}

	return utf8Distance(getRaw(), firstOcc);
}

int64_t bbe::Utf8String::search(const Utf8String &string) const
{
	return bbe::Utf8String::search(string.getRaw());
}

long bbe::Utf8String::toLong(int base) const
{
	//UNTESTED
	return strtol(getRaw(), nullptr, base);
}

double bbe::Utf8String::toDouble() const
{
	//UNTESTED
	return strtod(getRaw(), nullptr);
}

float bbe::Utf8String::toFloat() const
{
	//UNTESTED
	return strtof(getRaw(), nullptr);
}

const char& bbe::Utf8String::operator[](std::size_t index) const
{
	const char* ptr = getRaw();
	for(size_t i = 0; i<index; i++)
	{
		ptr = bbe::utf8GetNextChar(ptr);
	}
	return *ptr;
}

char* bbe::Utf8String::getRaw()
{
	//UNTESTED
	if(m_usesSSO)
	{
		return m_UNION.m_ssoData;
	}
	else
	{
		return m_UNION.m_pdata;
	}
}

const char* bbe::Utf8String::getRaw() const
{
	//UNTESTED
	if(m_usesSSO)
	{
		return m_UNION.m_ssoData;
	}
	else
	{
		return m_UNION.m_pdata;
	}
}

bbe::Utf8String bbe::Utf8String::toUpperCase() const
{
	//UNTESTED
	bbe::Utf8String copy = *this;
	copy.toUpperCaseInPlace();
	return copy;
}

bbe::Utf8String bbe::Utf8String::toLowerCase() const
{
	//UNTESTED
	bbe::Utf8String copy = *this;
	copy.toLowerCaseInPlace();
	return copy;
}

void bbe::Utf8String::toUpperCaseInPlace()
{
	auto raw = getRaw();
	for(std::size_t i = 0; i < m_length; i++)
	{
		raw[i] = toupper(raw[i]);
	}
}

void bbe::Utf8String::toLowerCaseInPlace()
{
	auto raw = getRaw();
	for(std::size_t i = 0; i < m_length; i++)
	{
		raw[i] = tolower(raw[i]);
	}
}

std::size_t bbe::Utf8String::getLength() const
{
	return m_length;
}

std::size_t bbe::Utf8String::getLengthBytes() const
{
	return strlen(getRaw());
}

std::size_t bbe::Utf8String::getCapacity() const
{
	return m_capacity;
}






template<>
uint32_t bbe::hash(const bbe::String & t)
{
	//UNTESTED
	//This function is based on the djb2 hashing algorithm by Dan Bernstein
	//See: http://www.cse.yorku.ca/~oz/hash.html
	//Changes to the original algorithm:
	//  1. A 32 bit hash is used instead of a 64 bit hash
	//  2. Only the first 128 chars are used for hashing
	//  3. wchar_t is used instead of char
	uint32_t _hash = 5381;
	size_t length = t.getLength();
	if (length > 128)
	{
		length = 128;
	}

	for (size_t i = 0; i < length; i++)
	{
		_hash = ((_hash << 5) + _hash) + t[i];
	}

	return _hash;
}

std::size_t bbe::utf8len(const char* ptr)
{
	if(ptr == nullptr)
	{
		throw NullPointerException();
	}
	std::size_t len = 0;
	const byte* bptr = reinterpret_cast<const byte*>(ptr);
	while(*bptr != (byte)0b00000000)
	{
		if(((*bptr) & (byte)0b11000000) != (byte)0b10000000)
		{
			len++;
		}
		bptr++;
	}
	return len;
}

std::size_t bbe::utf8charlen(const char* ptr)
{
	if(ptr == nullptr)
	{
		throw NullPointerException();
	}
	const byte* bptr = reinterpret_cast<const byte*>(ptr);

	if(((*bptr) & (byte)0b10000000) == (byte)0b00000000) return 1;
	if(((*bptr) & (byte)0b11100000) == (byte)0b11000000) return 2;
	if(((*bptr) & (byte)0b11110000) == (byte)0b11100000) return 3;
	if(((*bptr) & (byte)0b11111000) == (byte)0b11110000) return 4;

	throw NotStartOfUtf8Exception();
}

bool bbe::utf8IsStartOfChar(const char* ptr)
{
	//UNTESTED
	if(ptr == nullptr)
	{
		throw NullPointerException();
	}
	const byte* bptr = reinterpret_cast<const byte*>(ptr);

	if(((*bptr) & (byte)0b10000000) == (byte)0b00000000) return true;
	if(((*bptr) & (byte)0b11100000) == (byte)0b11000000) return true;
	if(((*bptr) & (byte)0b11110000) == (byte)0b11100000) return true;
	if(((*bptr) & (byte)0b11111000) == (byte)0b11110000) return true;

	return false;
}

const char* bbe::utf8GetStartAddrOfCodePoint(const char* ptr)
{
	//UNTESTED
	if(ptr == nullptr)
	{
		throw NullPointerException();
	}

	if(*ptr == '\0')
	{
		throw UnexpectedEndOfStringException();
	}

	for(int i = 0; i<4; i++)
	{
		if(ptr[-i] == '\0') 
		{
			throw NotAUtf8CharException();
		}
		if(bbe::utf8IsStartOfChar(&ptr[-i])) 
		{
			return &ptr[-i];
		}
	}

	throw NotAUtf8CharException();
}

const char* bbe::utf8GetNextChar(const char* ptr)
{
	//UNTESTED
	if(*ptr == '\0')
	{
		throw UnexpectedEndOfStringException();
	}

	std::size_t length = bbe::utf8charlen(ptr);

	return ptr + length;
}

bool bbe::utf8IsSameChar(const char* ptr1, const char* ptr2)
{
	if(ptr1 == nullptr || ptr2 == nullptr) throw NullPointerException();
	if(ptr1 == ptr2) return true;

	auto len1 = bbe::utf8charlen(ptr1);
	auto len2 = bbe::utf8charlen(ptr2);

	if(len1 != len2) return false;

	for(std::size_t i = 0; i<len1; i++)
	{
		if(ptr1[i] != ptr2[i])
		{
			return false;
		}
	}

	return true;
}

bool bbe::utf8IsWhitespace(const char* ptr)
{
	if(ptr == nullptr)
	{
		throw NullPointerException();
	}

	if(bbe::utf8IsSameChar("\u0009", ptr)) return true; // CHARACTER TABULATION
	if(bbe::utf8IsSameChar("\u000A", ptr)) return true; // LINE FEED
	if(bbe::utf8IsSameChar("\u000B", ptr)) return true; // LINE TABULATION
	if(bbe::utf8IsSameChar("\u000C", ptr)) return true; // FORM FEED
	if(bbe::utf8IsSameChar("\u000D", ptr)) return true; // CARRIAGE RETURN
	if(bbe::utf8IsSameChar("\u0020", ptr)) return true; // SPACE
	if(bbe::utf8IsSameChar("\u0085", ptr)) return true; // NEXT LINE
	if(bbe::utf8IsSameChar("\u00A0", ptr)) return true; // NO-BREAK SPACE
	if(bbe::utf8IsSameChar("\u1680", ptr)) return true; // OGHAM SPACE MARK
	if(bbe::utf8IsSameChar("\u180E", ptr)) return true; // MONGOLIAN VOWEL SEPARATOR
	if(bbe::utf8IsSameChar("\u2000", ptr)) return true; // EN QUAD
	if(bbe::utf8IsSameChar("\u2001", ptr)) return true; // EM QUAD
	if(bbe::utf8IsSameChar("\u2002", ptr)) return true; // EN SPACE
	if(bbe::utf8IsSameChar("\u2003", ptr)) return true; // EM SPACE
	if(bbe::utf8IsSameChar("\u2004", ptr)) return true; // THREE-PER-EM SPACE
	if(bbe::utf8IsSameChar("\u2005", ptr)) return true; // FOUR-PER-EM SPACE
	if(bbe::utf8IsSameChar("\u2006", ptr)) return true; // SIX-PER-EM SPACE
	if(bbe::utf8IsSameChar("\u2007", ptr)) return true; // FIGURE SPACE
	if(bbe::utf8IsSameChar("\u2008", ptr)) return true; // PUNCTUATION SPACE
	if(bbe::utf8IsSameChar("\u2009", ptr)) return true; // THIN SPACE
	if(bbe::utf8IsSameChar("\u200A", ptr)) return true; // HAIR SPACE
	if(bbe::utf8IsSameChar("\u200B", ptr)) return true; // ZERO WIDTH SPACE
	if(bbe::utf8IsSameChar("\u200C", ptr)) return true; // ZERO WIDTH NON-JOINER
	if(bbe::utf8IsSameChar("\u200D", ptr)) return true; // ZERO WIDTH JOINER
	if(bbe::utf8IsSameChar("\u2028", ptr)) return true; // LINE SEPARATOR
	if(bbe::utf8IsSameChar("\u2029", ptr)) return true; // PARAGRAPH SEPARATOR
	if(bbe::utf8IsSameChar("\u202F", ptr)) return true; // NARROW NO-BREAK SPACE
	if(bbe::utf8IsSameChar("\u205F", ptr)) return true; // MEDIUM MATHEMATICAL SPACE
	if(bbe::utf8IsSameChar("\u2060", ptr)) return true; // WORD JOINER
	if(bbe::utf8IsSameChar("\u3000", ptr)) return true; // IDEOGRAPHIC SPACE
	if(bbe::utf8IsSameChar("\uFEFF", ptr)) return true; // ZERO WIDTH NON-BREAKING SPACE

	return false;
}

bool bbe::utf8IsLatinChar(const char* ptr)
{
	//UNTESTED
	if(ptr == nullptr)
	{
		throw NullPointerException();
	}

	return (*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z');
}

bool bbe::utf8IsDigitChar(const char* ptr)
{
	//UNTESTED
	if(ptr == nullptr)
	{
		throw NullPointerException();
	}

	return (*ptr >= '0' && *ptr <= '9');
}

bool bbe::utf8IsAsciiChar(const char* ptr)
{
	//UNTESTED
	if(ptr == nullptr)
	{
		throw NullPointerException();
	}

	const byte* bptr = reinterpret_cast<const byte*>(ptr);

	return ((*bptr) & (byte)0b10000000) == (byte)0;
}

bool bbe::utf8IsSmallerCodePoint(const char* ptr1, const char* ptr2)
{
	//UNTESTED
	if(ptr1 == nullptr || ptr2 == nullptr)
	{
		throw NullPointerException();
	}

	auto size1 = bbe::utf8charlen(ptr1);
	auto size2 = bbe::utf8charlen(ptr2);

	if(size1 < size2) return true;
	if(size1 > size2) return false;

	for(std::size_t i = 0; i<size1; i++)
	{
		if(ptr1[i] < ptr2[i]) return true;
		if(ptr1[i] > ptr2[i]) return false;
	}

	return false;
}

int bbe::utf8Distance(const char* ptr1, const char* ptr2)
{
	//UNTESTED
	bool negative = false;
	if(ptr2 < ptr1)
	{
		const char* temp = ptr1;
		ptr1 = ptr2;
		ptr2 = temp;
		negative = true;
	}

	int distance = 0;
	while(ptr1 < ptr2)
	{
		ptr1 = utf8GetNextChar(ptr1);
		distance++;
	}

	if(negative)
	{
		distance *= -1;
	}

	return distance;
}

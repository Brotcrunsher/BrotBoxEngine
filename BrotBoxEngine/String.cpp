#include "BBE/String.h"
#include "BBE/DataType.h"
#include "BBE/Exceptions.h"
#include "BBE/Math.h"
#include <string>
#include "BBE/Utf8Helpers.h"

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
	auto amountOfByte = strlen(data);

	if(amountOfByte < BBE_UTF8STRING_SSOSIZE - 1)
	{
		memcpy(m_UNION.m_ssoData, data, amountOfByte + 1);
		m_usesSSO = true;
		m_capacity = BBE_UTF8STRING_SSOSIZE;
	}
	else
	{
		m_UNION.m_pdata = new char[amountOfByte + 1];
		memcpy(m_UNION.m_pdata, data, amountOfByte + 1);
		m_usesSSO = false;
		m_capacity = amountOfByte + 1;
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

bbe::Utf8String::Utf8String(char c)
{
	char arr[] = { c, 0 };
	initializeFromCharArr(arr);
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

bbe::Utf8String::Utf8String(const std::initializer_list<char>& il)
{
	if (il.size() == 0)
	{
		initializeFromCharArr(u8"");
	}
	else
	{
		if (*(il.end() - 1) != '\0')
		{
			throw IllegalArgumentException("End of il wasn't nul.");
		}
		initializeFromCharArr(il.begin());
	}
}

void bbe::Utf8String::serialize(bbe::ByteBuffer& buffer) const
{
	buffer.writeNullString(getRaw());
}

bbe::Utf8String bbe::Utf8String::deserialize(bbe::ByteBufferSpan& buffer)
{
	return bbe::Utf8String(buffer.readNullString());
}

bbe::Utf8String::Utf8String(const Utf8String& other)//Copy Constructor
{ 
	//UNTESTED
	initializeFromCharArr(other.getRaw());
}

bbe::Utf8String::Utf8String(Utf8String&& other) noexcept //Move Constructor
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

bbe::Utf8String& bbe::Utf8String::operator=(Utf8String &&other) noexcept//Move Assignment
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

bbe::Utf8String bbe::Utf8String::fromCodePoint(int32_t codePoint)
{
	const std::size_t length = utf8codePointLen(codePoint);

	char c[5] = {};

	switch (length)
	{
	case 0:
		return "";
	case 1:
		return bbe::Utf8String((char)codePoint);
	case 2:
		c[0] = ((codePoint >> 6) & 0b00011111) | 0b11000000;
		c[1] =  (codePoint       & 0b00111111) | 0b10000000;
	}

	return bbe::Utf8String(c);
}

bbe::Utf8String bbe::Utf8String::toHex(uint32_t value)
{
	// TODO Test
	// TODO this implementation is probably quite wasteful.
	bbe::Utf8String retVal = "";

	constexpr const char* hexDigits = "0123456789ABCDEF";

	while (value > 0)
	{
		const uint32_t lowValue = value & 0x0F;
		retVal = fromCodePoint(hexDigits[lowValue]) + retVal;
		value >>= 4;
	}

	return retVal;
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
namespace bbe
{
	bool operator==(const char* arr, const bbe::Utf8String& string)
	{
		return strcmp(arr, string.getRaw()) == 0;
	}
}

bbe::Utf8StringView::Utf8StringView()
{
}
bbe::Utf8StringView::Utf8StringView(const Utf8String& string, std::size_t m_start, std::size_t m_end)
	:m_pstring(&string), m_start(m_start), m_end(m_end)
{
}
std::size_t bbe::Utf8StringView::getEnd() const
{
	if (m_end == (std::size_t) - 1)
	{
		return m_pstring->getLength();
	}
	return m_end;
}

std::size_t bbe::Utf8StringView::getLength() const
{
	return getEnd() - m_start;
}

std::size_t bbe::Utf8StringView::getLengthBytes() const
{
	const std::size_t end = getEnd();
	std::size_t retVal = 0;
	const char* cPtr = &(*m_pstring)[m_start];
	for (size_t i = m_start; i < end; i++)
	{
		// TODO: Use iterator.
		const size_t charLen = utf8charlen(cPtr);
		retVal += charLen;
		cPtr += charLen;
	}
	return retVal;
}

bool bbe::Utf8String::operator!=(const Utf8String& other) const
{
	return strcmp(getRaw(), other.getRaw()) != 0;
}
bool bbe::Utf8String::operator!=(const char*       other) const
{
	return strcmp(getRaw(), other) != 0;
}
namespace bbe{
	bool operator!=(const char* arr, const bbe::Utf8String& string)
	{
		return strcmp(arr, string.getRaw()) != 0;
	}
}

namespace bbe{
	std::ostream& operator<<(std::ostream &os, const bbe::Utf8String &string)
	{
		return os << string.getRaw();
	}
}

bbe::Utf8String bbe::Utf8String::operator+(const Utf8String& other) const
{
	//UNTESTED
	Utf8String retVal = *this;
	retVal += other;
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

namespace bbe{
	bbe::Utf8String operator+(const char* other, const bbe::Utf8String& string)
	{
		//UNTESTED
		return bbe::Utf8String(other) + string;
	}

	bbe::Utf8String operator+(double number, const bbe::Utf8String& string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(int number, const bbe::Utf8String& string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(long long number, const bbe::Utf8String& string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(long double number, const bbe::Utf8String& string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(float number, const bbe::Utf8String& string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(unsigned long long number, const bbe::Utf8String& string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(unsigned long number, const bbe::Utf8String& string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(long number, const bbe::Utf8String& string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(unsigned int number, const bbe::Utf8String& string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}
}

bbe::Utf8String& bbe::Utf8String::operator+=(const bbe::Utf8String& other)
{
	//UNTESTED
	const size_t totalLength = getLengthBytes() + other.getLengthBytes();
	const size_t oldLength = getLengthBytes();
	m_length = getLength() + other.getLength();
	growIfNeeded(totalLength + 1);
	memcpy(getRaw() + oldLength, other.getRaw(), other.getLengthBytes());
	getRaw()[totalLength] = 0;

	return *this;
}

bbe::Utf8String& bbe::Utf8String::operator+=(const bbe::Utf8StringView& other)
{
	//UNTESTED
	const size_t oldLength = getLengthBytes();
	const size_t otherLength = other.getLengthBytes();
	const size_t totalLength = oldLength + otherLength;
	const char* otherRaw = other.m_pstring->getRaw();
	m_length = getLength() + utf8len(&((*other.m_pstring)[other.m_start]), &((*other.m_pstring)[other.getEnd()]));
	growIfNeeded(totalLength + 1);
	memcpy(getRaw() + oldLength, &((*other.m_pstring)[other.m_start]), otherLength);
	getRaw()[totalLength] = 0;

	return *this;
}

bbe::Utf8String& bbe::Utf8String::operator+=(const char* other)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(other));
}

bbe::Utf8String& bbe::Utf8String::operator+=(char c)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(c));
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
	if (m_length == 0) return;

	size_t start = 0;
	size_t end = m_length - 1;

	while (utf8IsWhitespace(&(*this)[start]) && start != m_length - 1)
	{
		start++;
	}
	while (utf8IsWhitespace(&(*this)[end]) && end != 0)
	{
		end--;
	}

	substringInPlace(start, end + 1);
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
	if(end > m_length)
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
			std::size_t sizeOfSubstringInByte = 0;
			for(std::size_t i = start; i<end; i++)
			{
				sizeOfSubstringInByte += utf8charlen(&(*this)[i]);
			}
			memmove(raw, &raw[start], sizeOfSubstringInByte);
			raw[sizeOfSubstringInByte] = 0;
			m_length = end - start;
		}
	}
}

bbe::Utf8StringView bbe::Utf8String::substringView(std::size_t start, std::size_t end) const
{
	return bbe::Utf8StringView(*this, start, end);
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

bbe::DynamicArray<bbe::Utf8String> bbe::Utf8String::split(const bbe::Utf8String& splitAt, bool addEmpty) const
{
	//UNTESTED
	//TODO this method is a little mess. Clean it up!
	if (!addEmpty)
	{
		auto splitWith = split(splitAt, true);
		size_t empties = 0;
		for (const bbe::String& s : splitWith)
		{
			if (s == "") empties++;
		}
		if (empties == 0) return splitWith;

		DynamicArray<Utf8String> retVal(splitWith.getLength() - empties);
		size_t accessIndex = 0;
		for (const bbe::String& s : splitWith)
		{
			if (s != "")
			{
				retVal[accessIndex] = s;
				accessIndex++;
			}
		}
		return retVal;
	}

	size_t counted = count(splitAt);
	DynamicArray<Utf8String> retVal(counted + 1);
	if (counted == 0)
	{
		retVal[0] = getRaw();
		return retVal;
	}
	auto previousFinding = getRaw();
	if (!previousFinding)
	{
		throw IllegalStateException();
	}
	for (size_t i = 0; i < retVal.getLength() - 1; i++)
	{
		const char *currentFinding = strstr(previousFinding, splitAt.getRaw());
		if (!currentFinding)
		{
			return retVal;
		}
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
	size_t currentStringLength = getRaw() + getLengthBytes() - previousFinding;
	currentString.m_usesSSO = false; //TODO make this better! current string could use SSO!
	currentString.m_UNION.m_pdata = new char[currentStringLength + 1];
	memcpy(currentString.m_UNION.m_pdata, previousFinding, currentStringLength);
	currentString.m_UNION.m_pdata[currentStringLength] = 0;
	currentString.m_length = currentStringLength;
	retVal[retVal.getLength() - 1] = currentString;

	return retVal;
}

bbe::DynamicArray<bbe::Utf8String> bbe::Utf8String::split(const char* splitAt, bool addEmpty) const
{
	//UNTESTED
	return split(bbe::Utf8String(splitAt), addEmpty);
}

bbe::DynamicArray<bbe::Utf8String> bbe::Utf8String::lines(bool addEmpty) const
{
	auto lines = split("\n", addEmpty);
	for (size_t i = 0; i < lines.getLength(); i++)
	{
		if (lines[i].endsWith("\r"))
		{
			lines[i].substringInPlace(0, lines[i].getLength() - 1);
		}
	}

	return lines;
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

bool bbe::Utf8String::isTextAtLocation(const char* string, size_t index) const
{
	while (*string)
	{
		if (index >= m_length || *string != operator[](index))
		{
			return false;
		}
		index++;
		string++;
	}
	return true;
}

bool bbe::Utf8String::startsWith(const char* string) const
{
	return isTextAtLocation(string, 0);
}

bool bbe::Utf8String::endsWith(const char* string) const
{
	const size_t sLen = strlen(string);
	if (sLen > m_length)
	{
		return false;
	}
	return isTextAtLocation(string, m_length - sLen);
}

int64_t bbe::Utf8String::search(const char* string, int64_t startIndex) const
{
	//UNTESTED
	const char* firstOcc = strstr(&this->operator[](startIndex), string);
	if(firstOcc == nullptr)
	{
		return -1;
	}

	return utf8Distance(getRaw(), firstOcc);
}

int64_t bbe::Utf8String::search(const Utf8String &string, int64_t startIndex) const
{
	return bbe::Utf8String::search(string.getRaw(), startIndex);
}

int64_t bbe::Utf8String::searchLast(const char* string) const
{
	// TODO quite inefficient, use Knuth-Morris-Pratt!

	const size_t sLen = strlen(string);

	if (sLen > m_length || sLen == 0) return -1;

	for (size_t i = m_length - sLen; i != (size_t)-1; i--)
	{
		// TODO Urghs ... also inefficient, use iterators once we have them...
		const bbe::String temp = bbe::String(&operator[](i));
		if (temp.startsWith(string))
		{
			return i;
		}
	}

	return -1;
}

bool bbe::Utf8String::isNumber() const
{
	if (m_length == 0) return false;

	size_t i = 0;
	if (startsWith("+") || startsWith("-")) i++;

	for (; i < m_length; i++)
	{
		const char& c = operator[](i);
		if (c < '0' || c > '9')
		{
			return false;
		}
	}

	return true;
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
	if(index > m_length)
	{
		throw IllegalIndexException();
	}
	else if (index == m_length)
	{
		return *""; // Woah...
	}
	const char* ptr = getRaw();
	for(size_t i = 0; i<index; i++)
	{
		ptr = bbe::utf8GetNextChar(ptr);
	}
	return *ptr;
}

bool bbe::Utf8String::operator<(const bbe::Utf8String& other) const
{
	auto thisIterator = getIterator();
	auto otherIterator = other.getIterator();

	while (thisIterator.getCodepoint() == otherIterator.getCodepoint()
		&& *thisIterator != '\0'
		&& *otherIterator != '\0')
	{
		thisIterator++;
		otherIterator++;
	}

	return thisIterator.getCodepoint() < otherIterator.getCodepoint();
}

int32_t bbe::Utf8String::getCodepoint(size_t index) const
{
	return utf8CharToCodePoint(&operator[](index));
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

bbe::Utf8Iterator bbe::Utf8String::getIterator() const
{
	return Utf8Iterator(getRaw());
}

bbe::Utf8String bbe::Utf8String::replace(const Utf8String& searchString, const Utf8String& replaceString) const
{
	if (searchString == u8"") return *this;
	if (searchString == replaceString) return *this;

	bbe::Utf8String retVal = "";
	const size_t searchStringOccurences = count(searchString);
	retVal.growIfNeeded(getLengthBytes() + (replaceString.getLengthBytes() - searchString.getLengthBytes()) * searchStringOccurences );
	uint64_t currentFoundIndex = 0;
	uint64_t lastFoundIndex = 0;
	while ((currentFoundIndex = search(searchString, currentFoundIndex)) != (uint64_t)-1)
	{
		retVal += substringView(lastFoundIndex, currentFoundIndex);
		retVal += replaceString;
		
		currentFoundIndex += searchString.getLength();
		lastFoundIndex = currentFoundIndex;
	}
	retVal += substringView(lastFoundIndex, getLength());
	return retVal;
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

bbe::Utf8String bbe::Utf8String::leftFill(char c, size_t length)
{
	bbe::String retVal = "";
	if (length > getLength())
	{
		const size_t charsToPlace = length - getLength();
		for (size_t i = 0; i < charsToPlace; i++)
		{
			retVal += c;
		}
	}
	retVal += *this;
	return retVal;
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

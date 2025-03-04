#include "BBE/String.h"
#include "BBE/DataType.h"
#include "BBE/Error.h"
#include "BBE/Math.h"
#include <string>
#include <codecvt>
#include <locale>
#include <sstream>
#include "stdarg.h"
#include "BBE/Utf8Helpers.h"

void bbe::Utf8String::initializeFromCharArr(const char* data)
{
	auto amountOfByte = strlen(data);
	m_data.growIfNeeded(amountOfByte + 1);
	memcpy(m_data.get(), data, amountOfByte + 1);
}

bbe::Utf8String::Utf8String()
{
	//UNTESTED
	initializeFromCharArr("");
}

bbe::Utf8String::Utf8String(const char* data)
{
	//UNTESTED
	initializeFromCharArr(data);
}

bbe::Utf8String::Utf8String(const wchar_t* data)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	initializeFromCharArr(conv.to_bytes(data).c_str());
}

bbe::Utf8String::Utf8String(char c)
{
	const char arr[] = { c, 0 };
	initializeFromCharArr(arr);
}

bbe::Utf8String::Utf8String(double number, int32_t precision)
{
	//UNTESTED
	std::ostringstream stream;
	stream.precision(precision);
	stream << std::fixed << number;
	initializeFromCharArr(stream.str().c_str());
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

bbe::Utf8String::Utf8String(float number, int32_t precision)
{
	//UNTESTED
	std::ostringstream stream;
	stream.precision(precision);
	stream << std::fixed << number;
	initializeFromCharArr(stream.str().c_str());
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

bbe::Utf8String::Utf8String(const std::string& data)
{
	initializeFromCharArr(data.c_str());
}

bbe::Utf8String::Utf8String(const std::initializer_list<char>& il)
{
	if (il.size() == 0)
	{
		initializeFromCharArr("");
	}
	else
	{
		if (*(il.end() - 1) != '\0')
		{
			bbe::Crash(bbe::Error::IllegalArgument, "End of il wasn't nul.");
		}
		initializeFromCharArr(il.begin());
	}
}

bbe::Utf8String bbe::Utf8String::format(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	bbe::Utf8String retVal = formatVa(format, args);
	va_end(args);

	return retVal;
}

bbe::Utf8String bbe::Utf8String::formatVa(const char* format, va_list va)
{
	va_list va2;
	va_copy(va2, va);

	bbe::String retVal;

	auto amountOfByte = vsnprintf(nullptr, 0, format, va);
	retVal.m_data.growIfNeeded(amountOfByte + 1);
	vsnprintf(retVal.m_data.get(), amountOfByte + 1, format, va2);

	va_end(va2);
	return retVal;
}

void bbe::Utf8String::serialize(bbe::ByteBuffer& buffer) const
{
	buffer.writeNullString(getRaw());
}

bbe::Utf8String bbe::Utf8String::deserialize(bbe::ByteBufferSpan& buffer)
{
	return bbe::Utf8String(buffer.readNullString());
}

bbe::Utf8String bbe::Utf8String::fromCodePoint(int32_t codePoint)
{
	if (codePoint < 0 || codePoint > 0x10FFFF)
	{
		bbe::Crash(bbe::Error::IllegalArgument, "Invalid CodePoint");
	}

	char c[5] = {};

	if (codePoint <= 0x7F)
	{
		// 1-byte: 0xxxxxxx
		c[0] = static_cast<char>(codePoint);
	}
	else if (codePoint <= 0x7FF)
	{
		// 2-byte: 110xxxxx 10xxxxxx
		c[0] = static_cast<char>((codePoint >> 6)         | 0b11000000);
		c[1] = static_cast<char>((codePoint & 0b00111111) | 0b10000000);
	}
	else if (codePoint <= 0xFFFF)
	{
		// 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
		c[0] = static_cast<char>(( codePoint >> 12)               | 0b11100000);
		c[1] = static_cast<char>(((codePoint >>  6) & 0b00111111) | 0b10000000);
		c[2] = static_cast<char>(( codePoint        & 0b00111111) | 0b10000000);
	}
	else
	{
		// 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		c[0] = static_cast<char>(( codePoint >> 18)               | 0b11110000);
		c[1] = static_cast<char>(((codePoint >> 12) & 0b00111111) | 0b10000000);
		c[2] = static_cast<char>(((codePoint >>  6) & 0b00111111) | 0b10000000);
		c[3] = static_cast<char>(( codePoint        & 0b00111111) | 0b10000000);
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
bbe::Utf8StringView::Utf8StringView(const Utf8String& string, std::size_t start, std::size_t end)
	:m_pstring(&string), m_start(start), m_end(end)
{
	if (m_end == (size_t)-1)
	{
		m_end = utf8len(m_pstring->getRaw());
	}
}
std::size_t bbe::Utf8StringView::getEnd() const
{
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

bbe::Utf8String bbe::Utf8String::operator*(int32_t mult) const
{
	// TODO: This does a lot of unnecessary allocations. Calculate capacity ahead of time.
	bbe::String retVal;
	for (int32_t i = 0; i < mult; i++)
	{
		retVal += *this;
	}
	return retVal;
}

bbe::Utf8String& bbe::Utf8String::operator+=(const bbe::Utf8String& other)
{
	//UNTESTED
	const size_t totalLength = getLengthBytes() + other.getLengthBytes();
	const size_t oldLength = getLengthBytes();
	m_data.growIfNeeded(totalLength + 1);
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
	m_data.growIfNeeded(totalLength + 1);
	memcpy(getRaw() + oldLength, &((*other.m_pstring)[other.m_start]), otherLength);
	getRaw()[totalLength] = 0;

	return *this;
}

bbe::Utf8String& bbe::Utf8String::operator+=(const std::string& other)
{
	return operator+=(bbe::Utf8String(other));
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

bbe::Utf8String bbe::Utf8String::rounded(int32_t digitsAfterDot) const
{
	if (!isNumber()) bbe::Crash(bbe::Error::IllegalState, "Not a number, can't be rounded");

	auto dotLocation = search(".");
	if (dotLocation == -1) return *this;

	return substring(0, dotLocation + digitsAfterDot + 1);
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
	size_t end = utf8len(m_data.get()) - 1;
	if (end == 0) return;

	while (utf8IsWhitespace(&(*this)[start]) && start != end - 1)
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
	if(end == (size_t)-1)
	{
		end = utf8len(m_data.get());
	}

	std::size_t sizeOfSubstringInByte = 0;
	auto it = getIterator();
	for (size_t i = 0; i < start; i++) ++it;
	for(std::size_t i = start; i<end; i++)
	{
		sizeOfSubstringInByte += utf8codePointLen(it.getCodepoint());
		++it;
	}
	auto raw = m_data.get();
	memmove(raw, &raw[start], sizeOfSubstringInByte);
	raw[sizeOfSubstringInByte] = 0;
}

bbe::Utf8StringView bbe::Utf8String::substringView(std::size_t start, std::size_t end) const
{
	return bbe::Utf8StringView(*this, start, end);
}

size_t bbe::Utf8String::count(const Utf8String& countand) const
{
	//UNTESTED
	size_t countandLength = utf8len(countand.getRaw());
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
		bbe::Crash(bbe::Error::IllegalState);
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
		currentString.m_data.growIfNeeded(currentStringLength + 1);
		memcpy(currentString.m_data.get(), previousFinding, currentStringLength);
		currentString.m_data.get()[currentStringLength] = 0;

		retVal[i] = currentString;

		previousFinding = currentFinding + utf8len(splitAt.getRaw());
	}

	Utf8String currentString;
	size_t currentStringLength = getRaw() + getLengthBytes() - previousFinding;
	currentString.m_data.growIfNeeded(currentStringLength + 1);
	memcpy(currentString.m_data.get(), previousFinding, currentStringLength);
	currentString.m_data.get()[currentStringLength] = 0;
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
			lines[i].substringInPlace(0, utf8len(lines[i].getRaw()) - 1);
		}
	}

	return lines;
}

bool bbe::Utf8String::containsAny(const char* string) const
{
	for (auto it = getIterator(); it.valid(); ++it)
	{
		const char* s = string;
		while (*s)
		{
			if (it.getCodepoint() == *s)
			{
				return true;
			}
			s++;
		}
	}
	return false;
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

#ifdef _MSC_VER
#include <shlwapi.h>
#include <AtlBase.h>
#else
#endif

static bool platformIndependentStrStrI(const char* haystack, const char* needle)
{
#ifdef _MSC_VER
	return StrStrIA(haystack, needle) != nullptr;
#else
	return strcasestr(haystack, needle) != nullptr;
#endif
}

bool bbe::Utf8String::containsIgnoreCase(const char* string) const
{
	return platformIndependentStrStrI(getRaw(), string);
}

bool bbe::Utf8String::containsIgnoreCase(const Utf8String& string) const
{
	return containsIgnoreCase(string.getRaw());
}

bbe::Utf8String bbe::Utf8String::hardBreakEvery(int32_t x) const
{
	//TODO: This is a rather dumb implementation. We could easily determine the necessary amount of bytes ahead of time.
	bbe::Utf8String retVal;

	int32_t column = 0;
	for (auto it = getIterator(); it.valid(); ++it)
	{
		if (column == x)
		{
			retVal += "\n";
			column = 0;
		}
		column++;
		char buff[sizeof(int32_t) + 1] = {};
		auto cp = it.getCodepoint();
		memcpy(buff, &cp, sizeof(cp));
		retVal += buff;
	}

	return retVal;
}

bool bbe::Utf8String::isTextAtLocation(const char* string, size_t index) const
{
	while (*string)
	{
		// TODO:                           VVVVVVVVVVVVVVVVVVVVVVVVVV Not UTF8!
		if (index >= utf8len(getRaw()) || *string != operator[](index))
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

bool bbe::Utf8String::startsWith(const bbe::String& string) const
{
	return startsWith(string.getRaw());
}

bool bbe::Utf8String::endsWith(const char* string) const
{
	const size_t sLen = strlen(string);
	if (sLen > utf8len(getRaw()))
	{
		return false;
	}
	return isTextAtLocation(string, utf8len(getRaw()) - sLen);
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

	if (sLen > utf8len(getRaw()) || sLen == 0) return -1;

	for (size_t i = utf8len(getRaw()) - sLen; i != (size_t)-1; i--)
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
	if (utf8len(getRaw()) == 0) return false;

	size_t i = 0;
	if (startsWith("+") || startsWith("-")) i++;

	bool dotFound = false;
	for (; i < utf8len(getRaw()); i++)
	{
		const char& c = operator[](i);
		if (c == '.')
		{
			if (dotFound) return false;
			dotFound = true;
		}
		else if (c < '0' || c > '9')
		{
			return false;
		}
	}

	return true;
}

bool bbe::Utf8String::isEmpty() const
{
	return getRaw()[0] == 0;
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
	if(index > utf8len(getRaw()))
	{
		bbe::Crash(bbe::Error::IllegalIndex);
	}
	else if (index == utf8len(getRaw()))
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
		++thisIterator;
		++otherIterator;
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
	return m_data.get();
}

const char* bbe::Utf8String::getRaw() const
{
	//UNTESTED
	return m_data.get();
}

bbe::Utf8Iterator bbe::Utf8String::getIterator() const
{
	return Utf8Iterator(getRaw());
}

bbe::Utf8String bbe::Utf8String::replace(const Utf8String& searchString, const Utf8String& replaceString) const
{
	if (searchString == "") return *this;
	if (searchString == replaceString) return *this;

	bbe::Utf8String retVal = "";
	const size_t searchStringOccurences = count(searchString);
	retVal.m_data.growIfNeeded(getLengthBytes() + (replaceString.getLengthBytes() - searchString.getLengthBytes()) * searchStringOccurences );
	uint64_t currentFoundIndex = 0;
	uint64_t lastFoundIndex = 0;
	while ((currentFoundIndex = search(searchString, currentFoundIndex)) != (uint64_t)-1)
	{
		retVal += substringView(lastFoundIndex, currentFoundIndex);
		retVal += replaceString;
		
		currentFoundIndex += utf8len(searchString.getRaw());
		lastFoundIndex = currentFoundIndex;
	}
	retVal += substringView(lastFoundIndex, utf8len(getRaw()));
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
	for(std::size_t i = 0; i < utf8len(getRaw()); i++)
	{
		raw[i] = toupper(raw[i]);
	}
}

void bbe::Utf8String::toLowerCaseInPlace()
{
	auto raw = getRaw();
	for(std::size_t i = 0; i < utf8len(getRaw()); i++)
	{
		raw[i] = tolower(raw[i]);
	}
}

size_t bbe::Utf8String::getLength() const
{
	return utf8len(getRaw());
}

std::size_t bbe::Utf8String::getLengthBytes() const
{
	return strlen(getRaw());
}

std::size_t bbe::Utf8String::getCapacity() const
{
	return m_data.getCapacity();
}

void bbe::Utf8String::resizeCapacity(size_t newCapacity)
{
	m_data.growIfNeeded(newCapacity);
}

bbe::Utf8String bbe::Utf8String::leftFill(char c, size_t length)
{
	bbe::String retVal = "";
	if (length > utf8len(getRaw()))
	{
		const size_t charsToPlace = length - utf8len(getRaw());
		for (size_t i = 0; i < charsToPlace; i++)
		{
			retVal += c;
		}
	}
	retVal += *this;
	return retVal;
}

bbe::Utf8String& bbe::Utf8String::append(const bbe::Utf8String& other)
{
	size_t totalLength = getLengthBytes() + other.getLengthBytes();
	m_data.growIfNeeded(totalLength + 1); // +1 for null terminator
	memcpy(getRaw() + getLengthBytes(), other.getRaw(), other.getLengthBytes());
	getRaw()[totalLength] = '\0';
	return *this;
}

bbe::Utf8String& bbe::Utf8String::append(const bbe::Utf8String& other, size_t pos, size_t count)
{
	if (pos > other.getLength())
	{
		bbe::Crash(bbe::Error::IllegalArgument, "Position out of range in append.");
	}

#undef min
	size_t actualCount = (count == npos) ? (other.getLength() - pos) : bbe::Math::min(count, other.getLength() - pos);
	Utf8StringView view = other.substringView(pos, pos + actualCount);
	return append(view);
}

bbe::Utf8String& bbe::Utf8String::append(const char* s)
{
	size_t sLengthBytes = strlen(s);
	size_t totalLength = getLengthBytes() + sLengthBytes;
	m_data.growIfNeeded(totalLength + 1); // +1 for null terminator
	memcpy(getRaw() + getLengthBytes(), s, sLengthBytes);
	getRaw()[totalLength] = '\0';
	return *this;
}

bbe::Utf8String& bbe::Utf8String::append(const char* s, size_t count)
{
	size_t sLengthBytes = strlen(s);
	size_t actualCount = std::min(count, sLengthBytes);
	size_t totalLength = getLengthBytes() + actualCount;
	m_data.growIfNeeded(totalLength + 1); // +1 for null terminator
	memcpy(getRaw() + getLengthBytes(), s, actualCount);
	getRaw()[totalLength] = '\0';
	return *this;
}

bbe::Utf8String& bbe::Utf8String::append(size_t count, char c)
{
	size_t totalLength = getLengthBytes() + count;
	m_data.growIfNeeded(totalLength + 1); // +1 for null terminator
	memset(getRaw() + getLengthBytes(), c, count);
	getRaw()[totalLength] = '\0';
	return *this;
}

bbe::Utf8String& bbe::Utf8String::append(const bbe::Utf8StringView& view)
{
	size_t otherLengthBytes = view.getLengthBytes();
	size_t totalLength = getLengthBytes() + otherLengthBytes;
	m_data.growIfNeeded(totalLength + 1); // +1 for null terminator
	memcpy(getRaw() + getLengthBytes(), &((*view.m_pstring)[view.m_start]), otherLengthBytes);
	getRaw()[totalLength] = '\0';
	return *this;
}

std::string bbe::Utf8String::toStdString() const
{
	return std::string(getRaw());
}

std::wstring bbe::Utf8String::toStdWString() const
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(getRaw());
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
	size_t length = utf8len(t.getRaw());
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

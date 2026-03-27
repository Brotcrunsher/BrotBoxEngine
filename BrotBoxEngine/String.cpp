#include "BBE/String.h"
#include "BBE/Error.h"
#include "BBE/Math.h"
#include "BBE/Utf8Helpers.h"
#include "stdarg.h"
#include <algorithm>
#include <codecvt>
#include <locale>
#include <sstream>
#include <string>

namespace
{
	struct WesternLatinCasePair
	{
		int32_t upper;
		int32_t lower;
	};

	constexpr WesternLatinCasePair WESTERN_LATIN_CASE_PAIRS[] = {
		{ 0x0100, 0x0101 }, // Ā/ā A with macron
		{ 0x0102, 0x0103 }, // Ă/ă A with breve
		{ 0x0104, 0x0105 }, // Ą/ą A with ogonek
		{ 0x0106, 0x0107 }, // Ć/ć C with acute
		{ 0x0108, 0x0109 }, // Ĉ/ĉ C with circumflex
		{ 0x010A, 0x010B }, // Ċ/ċ C with dot above
		{ 0x010C, 0x010D }, // Č/č C with caron
		{ 0x010E, 0x010F }, // Ď/ď D with caron
		{ 0x0110, 0x0111 }, // Đ/đ D with stroke
		{ 0x0112, 0x0113 }, // Ē/ē E with macron
		{ 0x0114, 0x0115 }, // Ĕ/ĕ E with breve
		{ 0x0116, 0x0117 }, // Ė/ė E with dot above
		{ 0x0118, 0x0119 }, // Ę/ę E with ogonek
		{ 0x011A, 0x011B }, // Ě/ě E with caron
		{ 0x011C, 0x011D }, // Ĝ/ĝ G with circumflex
		{ 0x011E, 0x011F }, // Ğ/ğ G with breve
		{ 0x0120, 0x0121 }, // Ġ/ġ G with dot above
		{ 0x0122, 0x0123 }, // Ģ/ģ G with cedilla
		{ 0x0124, 0x0125 }, // Ĥ/ĥ H with circumflex
		{ 0x0126, 0x0127 }, // Ħ/ħ H with stroke
		{ 0x0128, 0x0129 }, // Ĩ/ĩ I with tilde
		{ 0x012A, 0x012B }, // Ī/ī I with macron
		{ 0x012C, 0x012D }, // Ĭ/ĭ I with breve
		{ 0x012E, 0x012F }, // Į/į I with ogonek
		{ 0x0132, 0x0133 }, // Ĳ/ĳ IJ ligature
		{ 0x0134, 0x0135 }, // Ĵ/ĵ J with circumflex
		{ 0x0136, 0x0137 }, // Ķ/ķ K with cedilla
		{ 0x0139, 0x013A }, // Ĺ/ĺ L with acute
		{ 0x013B, 0x013C }, // Ļ/ļ L with cedilla
		{ 0x013D, 0x013E }, // Ľ/ľ L with caron
		{ 0x013F, 0x0140 }, // Ŀ/ŀ L with middle dot
		{ 0x0141, 0x0142 }, // Ł/ł L with stroke
		{ 0x0143, 0x0144 }, // Ń/ń N with acute
		{ 0x0145, 0x0146 }, // Ņ/ņ N with cedilla
		{ 0x0147, 0x0148 }, // Ň/ň N with caron
		{ 0x014A, 0x014B }, // Ŋ/ŋ Eng
		{ 0x014C, 0x014D }, // Ō/ō O with macron
		{ 0x014E, 0x014F }, // Ŏ/ŏ O with breve
		{ 0x0150, 0x0151 }, // Ő/ő O with double acute
		{ 0x0152, 0x0153 }, // Œ/œ OE ligature
		{ 0x0154, 0x0155 }, // Ŕ/ŕ R with acute
		{ 0x0156, 0x0157 }, // Ŗ/ŗ R with cedilla
		{ 0x0158, 0x0159 }, // Ř/ř R with caron
		{ 0x015A, 0x015B }, // Ś/ś S with acute
		{ 0x015C, 0x015D }, // Ŝ/ŝ S with circumflex
		{ 0x015E, 0x015F }, // Ş/ş S with cedilla
		{ 0x0160, 0x0161 }, // Š/š S with caron
		{ 0x0162, 0x0163 }, // Ţ/ţ T with cedilla
		{ 0x0164, 0x0165 }, // Ť/ť T with caron
		{ 0x0166, 0x0167 }, // Ŧ/ŧ T with stroke
		{ 0x0168, 0x0169 }, // Ũ/ũ U with tilde
		{ 0x016A, 0x016B }, // Ū/ū U with macron
		{ 0x016C, 0x016D }, // Ŭ/ŭ U with breve
		{ 0x016E, 0x016F }, // Ů/ů U with ring above
		{ 0x0170, 0x0171 }, // Ű/ű U with double acute
		{ 0x0172, 0x0173 }, // Ų/ų U with ogonek
		{ 0x0174, 0x0175 }, // Ŵ/ŵ W with circumflex
		{ 0x0176, 0x0177 }, // Ŷ/ŷ Y with circumflex
		{ 0x0179, 0x017A }, // Ź/ź Z with acute
		{ 0x017B, 0x017C }, // Ż/ż Z with dot above
		{ 0x017D, 0x017E }, // Ž/ž Z with caron
		{ 0x0218, 0x0219 }, // Ș/ș S with comma below
		{ 0x021A, 0x021B }, // Ț/ț T with comma below
	};

	const char *utf8PointerAtIndexOrNull(const char *text, std::size_t index)
	{
		const char *ptr = text;
		for (std::size_t i = 0; i < index; i++)
		{
			if (*ptr == '\0')
			{
				return nullptr;
			}
			ptr = bbe::utf8GetNextChar(ptr);
		}
		return ptr;
	}

	const char *utf8PointerAtIndex(const char *text, std::size_t index)
	{
		const char *ptr = utf8PointerAtIndexOrNull(text, index);
		if (ptr == nullptr)
		{
			bbe::Crash(bbe::Error::IllegalIndex);
		}
		return ptr;
	}

	int32_t mapWesternLatinCase(int32_t codepoint, bool toUpper)
	{
		if (toUpper)
		{
			if (codepoint >= 'a' && codepoint <= 'z') return codepoint - ('a' - 'A');
			if ((codepoint >= 0x00E0 && codepoint <= 0x00F6) || (codepoint >= 0x00F8 && codepoint <= 0x00FE))
			{
				return codepoint - 0x20;
			}
			if (codepoint == 0x00FF) return 0x0178; // y diaeresis
			if (codepoint == 0x00DF) return 0x1E9E; // sharp s

			for (const WesternLatinCasePair &pair : WESTERN_LATIN_CASE_PAIRS)
			{
				if (pair.lower == codepoint) return pair.upper;
			}
		}
		else
		{
			if (codepoint >= 'A' && codepoint <= 'Z') return codepoint + ('a' - 'A');
			if ((codepoint >= 0x00C0 && codepoint <= 0x00D6) || (codepoint >= 0x00D8 && codepoint <= 0x00DE))
			{
				return codepoint + 0x20;
			}
			if (codepoint == 0x0178) return 0x00FF; // Y diaeresis
			if (codepoint == 0x1E9E) return 0x00DF; // capital sharp s

			for (const WesternLatinCasePair &pair : WESTERN_LATIN_CASE_PAIRS)
			{
				if (pair.upper == codepoint) return pair.lower;
			}
		}

		return codepoint;
	}
}

void bbe::Utf8String::initializeFromCharArr(const char *data)
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

bbe::Utf8String::Utf8String(const char *data)
{
	//UNTESTED
	initializeFromCharArr(data);
}

bbe::Utf8String::Utf8String(const wchar_t *data)
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

bbe::Utf8String::Utf8String(const std::string &data)
{
	initializeFromCharArr(data.c_str());
}

bbe::Utf8String::Utf8String(const std::initializer_list<char> &il)
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

bbe::Utf8String bbe::Utf8String::format(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	bbe::Utf8String retVal = formatVa(format, args);
	va_end(args);

	return retVal;
}

bbe::Utf8String bbe::Utf8String::formatVa(const char *format, va_list va)
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

void bbe::Utf8String::serialize(bbe::ByteBuffer &buffer) const
{
	buffer.writeNullString(getRaw());
}

bbe::Utf8String bbe::Utf8String::deserialize(bbe::ByteBufferSpan &buffer)
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
		c[0] = static_cast<char>((codePoint >> 6) | 0b11000000);
		c[1] = static_cast<char>((codePoint & 0b00111111) | 0b10000000);
	}
	else if (codePoint <= 0xFFFF)
	{
		// 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
		c[0] = static_cast<char>((codePoint >> 12) | 0b11100000);
		c[1] = static_cast<char>(((codePoint >> 6) & 0b00111111) | 0b10000000);
		c[2] = static_cast<char>((codePoint & 0b00111111) | 0b10000000);
	}
	else
	{
		// 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		c[0] = static_cast<char>((codePoint >> 18) | 0b11110000);
		c[1] = static_cast<char>(((codePoint >> 12) & 0b00111111) | 0b10000000);
		c[2] = static_cast<char>(((codePoint >> 6) & 0b00111111) | 0b10000000);
		c[3] = static_cast<char>((codePoint & 0b00111111) | 0b10000000);
	}

	return bbe::Utf8String(c);
}

bbe::Utf8String bbe::Utf8String::toHex(uint32_t value)
{
	// TODO Test
	// TODO this implementation is probably quite wasteful.
	bbe::Utf8String retVal = "";

	constexpr const char *hexDigits = "0123456789ABCDEF";

	if (value == 0)
	{
		return bbe::Utf8String("0");
	}

	while (value > 0)
	{
		const uint32_t lowValue = value & 0x0F;
		retVal = fromCodePoint(hexDigits[lowValue]) + retVal;
		value >>= 4;
	}

	return retVal;
}

bool bbe::Utf8String::operator==(const Utf8String &other) const
{
	return strcmp(getRaw(), other.getRaw()) == 0;
}
bool bbe::Utf8String::operator==(const char *other) const
{
	return strcmp(getRaw(), other) == 0;
}
namespace bbe
{
	bool operator==(const char *arr, const bbe::Utf8String &string)
	{
		return strcmp(arr, string.getRaw()) == 0;
	}
}

bbe::Utf8StringView::Utf8StringView()
{
}
bbe::Utf8StringView::Utf8StringView(const Utf8String &string, std::size_t start, std::size_t end)
	: m_pstring(&string), m_start(start), m_end(end)
{
	const std::size_t length = utf8len(m_pstring->getRaw());
	if (m_start > length)
	{
		bbe::Crash(bbe::Error::IllegalIndex);
	}
	if (m_end == (size_t)-1 || m_end > length) m_end = length;
	if (m_end < m_start) m_end = m_start;
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
	const char *cPtr = &(*m_pstring)[m_start];
	for (size_t i = m_start; i < end; i++)
	{
		// TODO: Use iterator.
		const size_t charLen = utf8charlen(cPtr);
		retVal += charLen;
		cPtr += charLen;
	}
	return retVal;
}

bool bbe::Utf8String::operator!=(const Utf8String &other) const
{
	return strcmp(getRaw(), other.getRaw()) != 0;
}
bool bbe::Utf8String::operator!=(const char *other) const
{
	return strcmp(getRaw(), other) != 0;
}
namespace bbe
{
	bool operator!=(const char *arr, const bbe::Utf8String &string)
	{
		return strcmp(arr, string.getRaw()) != 0;
	}
}

namespace bbe
{
	std::ostream &operator<<(std::ostream &os, const bbe::Utf8String &string)
	{
		return os << string.getRaw();
	}
}

bbe::Utf8String bbe::Utf8String::operator+(const Utf8String &other) const
{
	//UNTESTED
	Utf8String retVal = *this;
	retVal += other;
	return retVal;
}

bbe::Utf8String bbe::Utf8String::operator+(const char *other) const
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

namespace bbe
{
	bbe::Utf8String operator+(const char *other, const bbe::Utf8String &string)
	{
		//UNTESTED
		return bbe::Utf8String(other) + string;
	}

	bbe::Utf8String operator+(double number, const bbe::Utf8String &string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(int number, const bbe::Utf8String &string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(long long number, const bbe::Utf8String &string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(long double number, const bbe::Utf8String &string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(float number, const bbe::Utf8String &string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(unsigned long long number, const bbe::Utf8String &string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(unsigned long number, const bbe::Utf8String &string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(long number, const bbe::Utf8String &string)
	{
		//UNTESTED
		return bbe::Utf8String(number) + string;
	}

	bbe::Utf8String operator+(unsigned int number, const bbe::Utf8String &string)
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

bbe::Utf8String &bbe::Utf8String::operator+=(const bbe::Utf8String &other)
{
	const size_t oldLength = getLengthBytes();
	const size_t totalLength = oldLength + other.getLengthBytes();
	m_data.growIfNeeded(totalLength + 1);
	memmove(getRaw() + oldLength, other.getRaw(), other.getLengthBytes());
	getRaw()[totalLength] = 0;

	return *this;
}

bbe::Utf8String &bbe::Utf8String::operator+=(const bbe::Utf8StringView &other)
{
	const size_t oldLength = getLengthBytes();
	const size_t otherLength = other.getLengthBytes();
	const size_t totalLength = oldLength + otherLength;
	m_data.growIfNeeded(totalLength + 1);
	const char *otherRaw = utf8PointerAtIndex(other.m_pstring->getRaw(), other.m_start);
	memmove(getRaw() + oldLength, otherRaw, otherLength);
	getRaw()[totalLength] = 0;

	return *this;
}

bbe::Utf8String &bbe::Utf8String::operator+=(const std::string &other)
{
	return operator+=(bbe::Utf8String(other));
}

bbe::Utf8String &bbe::Utf8String::operator+=(const char *other)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(other));
}

bbe::Utf8String &bbe::Utf8String::operator+=(char c)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(c));
}

bbe::Utf8String &bbe::Utf8String::operator+=(double number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String &bbe::Utf8String::operator+=(int number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String &bbe::Utf8String::operator+=(long long number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String &bbe::Utf8String::operator+=(long double number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String &bbe::Utf8String::operator+=(float number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String &bbe::Utf8String::operator+=(unsigned long long number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String &bbe::Utf8String::operator+=(unsigned long number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String &bbe::Utf8String::operator+=(long number)
{
	//UNTESTED
	return operator+=(bbe::Utf8String(number));
}

bbe::Utf8String &bbe::Utf8String::operator+=(unsigned int number)
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
	const size_t length = utf8len(m_data.get());
	if (length == 0) return;

	size_t start = 0;
	while (start < length && utf8IsWhitespace(utf8PointerAtIndex(m_data.get(), start)))
	{
		start++;
	}

	if (start == length)
	{
		m_data.get()[0] = 0;
		return;
	}

	size_t end = length;
	while (end > start && utf8IsWhitespace(utf8PointerAtIndex(m_data.get(), end - 1)))
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
	const std::size_t length = utf8len(m_data.get());
	if (start > length)
	{
		bbe::Crash(bbe::Error::IllegalIndex);
	}
	if (end == (size_t)-1 || end > length) end = length;
	if (end < start) end = start;

	const char *startPtr = utf8PointerAtIndex(m_data.get(), start);
	const char *endPtr = utf8PointerAtIndex(m_data.get(), end);
	const std::size_t sizeOfSubstringInByte = static_cast<std::size_t>(endPtr - startPtr);
	auto raw = m_data.get();
	memmove(raw, startPtr, sizeOfSubstringInByte);
	raw[sizeOfSubstringInByte] = 0;
}

bbe::Utf8StringView bbe::Utf8String::substringView(std::size_t start, std::size_t end) const
{
	return bbe::Utf8StringView(*this, start, end);
}

size_t bbe::Utf8String::count(const Utf8String &countand) const
{
	const size_t countandLengthBytes = countand.getLengthBytes();
	if (countandLengthBytes == 0)
	{
		return 0;
	}
	size_t amount = 0;
	auto readHead = getRaw();

	while ((readHead = strstr(readHead, countand.getRaw())) != nullptr)
	{
		amount++;
		readHead += countandLengthBytes;
	}
	return amount;
}

size_t bbe::Utf8String::count(const char *countand) const
{
	//UNTESTED
	return count(bbe::Utf8String(countand));
}

bbe::DynamicArray<bbe::Utf8String> bbe::Utf8String::split(const bbe::Utf8String &splitAt, bool addEmpty) const
{
	//UNTESTED
	//TODO this method is a little mess. Clean it up!
	if (!addEmpty)
	{
		auto splitWith = split(splitAt, true);
		size_t empties = 0;
		for (const bbe::String &s : splitWith)
		{
			if (s == "") empties++;
		}
		if (empties == 0) return splitWith;

		DynamicArray<Utf8String> retVal(splitWith.getLength() - empties);
		size_t accessIndex = 0;
		for (const bbe::String &s : splitWith)
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

		previousFinding = currentFinding + strlen(splitAt.getRaw());
	}

	Utf8String currentString;
	size_t currentStringLength = getRaw() + getLengthBytes() - previousFinding;
	currentString.m_data.growIfNeeded(currentStringLength + 1);
	memcpy(currentString.m_data.get(), previousFinding, currentStringLength);
	currentString.m_data.get()[currentStringLength] = 0;
	retVal[retVal.getLength() - 1] = currentString;

	return retVal;
}

bbe::DynamicArray<bbe::Utf8String> bbe::Utf8String::split(const char *splitAt, bool addEmpty) const
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

bool bbe::Utf8String::containsAny(const char *string) const
{
	if (string == nullptr) bbe::Crash(bbe::Error::NullPointer);

	for (auto it = getIterator(); it.valid(); ++it)
	{
		for (const char *s = string; *s; s = utf8GetNextChar(s))
		{
			if (it.getCodepoint() == utf8CharToCodePoint(s))
			{
				return true;
			}
		}
	}
	return false;
}

bool bbe::Utf8String::contains(const char *string) const
{
	//UNTESTED
	return strstr(getRaw(), string) != nullptr;
}

bool bbe::Utf8String::contains(const Utf8String &string) const
{
	//UNTESTED
	return contains(string.getRaw());
}

bool bbe::Utf8String::containsIgnoreCase(const char *string) const
{
	return toLowerCase().contains(bbe::Utf8String(string).toLowerCase());
}

bool bbe::Utf8String::containsIgnoreCase(const Utf8String &string) const
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
		if (*it == '\n')
		{
			column = 0;
		}
		if (column == x)
		{
			retVal += "\n";
			column = 0;
		}
		column++;
		retVal += fromCodePoint(it.getCodepoint());
	}

	return retVal;
}

bool bbe::Utf8String::isTextAtLocation(const char *string, size_t index) const
{
	if (string == nullptr) bbe::Crash(bbe::Error::NullPointer);

	const char *location = utf8PointerAtIndexOrNull(getRaw(), index);
	if (location == nullptr) return false;

	const std::size_t searchBytes = strlen(string);
	const std::size_t remainingBytes = static_cast<std::size_t>((getRaw() + getLengthBytes()) - location);
	if (searchBytes > remainingBytes) return false;

	for (std::size_t i = 0; i < searchBytes; i++)
	{
		if (location[i] != string[i])
		{
			return false;
		}
	}
	return true;
}

bool bbe::Utf8String::startsWith(const char *string) const
{
	return isTextAtLocation(string, 0);
}

bool bbe::Utf8String::startsWith(const bbe::String &string) const
{
	return startsWith(string.getRaw());
}

bool bbe::Utf8String::endsWith(const char *string) const
{
	const size_t sLen = strlen(string);
	const size_t thisLen = getLengthBytes();
	if (sLen > thisLen)
	{
		return false;
	}
	return memcmp(getRaw() + thisLen - sLen, string, sLen) == 0;
}

int64_t bbe::Utf8String::search(const char *string, int64_t startIndex) const
{
	if (startIndex < 0)
	{
		return -1;
	}

	const char *startPtr = utf8PointerAtIndexOrNull(getRaw(), static_cast<std::size_t>(startIndex));
	if (startPtr == nullptr)
	{
		return -1;
	}

	const char *firstOcc = strstr(startPtr, string);
	if (firstOcc == nullptr)
	{
		return -1;
	}

	return utf8Distance(getRaw(), firstOcc);
}

int64_t bbe::Utf8String::search(const Utf8String &string, int64_t startIndex) const
{
	return bbe::Utf8String::search(string.getRaw(), startIndex);
}

int64_t bbe::Utf8String::searchLast(const char *string) const
{
	const size_t needleLengthBytes = strlen(string);
	const size_t thisLengthBytes = getLengthBytes();
	if (needleLengthBytes == 0 || needleLengthBytes > thisLengthBytes) return -1;

	const char *lastMatch = nullptr;
	for (const char *ptr = getRaw(); *ptr != '\0'; ptr = utf8GetNextChar(ptr))
	{
		if (static_cast<size_t>((getRaw() + thisLengthBytes) - ptr) < needleLengthBytes)
		{
			break;
		}
		if (memcmp(ptr, string, needleLengthBytes) == 0)
		{
			lastMatch = ptr;
		}
	}

	if (lastMatch == nullptr) return -1;
	return utf8Distance(getRaw(), lastMatch);
}

bool bbe::Utf8String::isNumber() const
{
	if (utf8len(getRaw()) == 0) return false;

	size_t i = 0;
	if (startsWith("+") || startsWith("-")) i++;

	bool dotFound = false;
	for (; i < utf8len(getRaw()); i++)
	{
		const char &c = operator[](i);
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

const char &bbe::Utf8String::operator[](std::size_t index) const
{
	return *utf8PointerAtIndex(getRaw(), index);
}

bool bbe::Utf8String::operator<(const bbe::Utf8String &other) const
{
	auto thisIterator = getIterator();
	auto otherIterator = other.getIterator();

	while (thisIterator.getCodepoint() == otherIterator.getCodepoint() && *thisIterator != '\0' && *otherIterator != '\0')
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

char *bbe::Utf8String::getRaw()
{
	//UNTESTED
	return m_data.get();
}

const char *bbe::Utf8String::getRaw() const
{
	//UNTESTED
	return m_data.get();
}

bbe::Utf8Iterator bbe::Utf8String::getIterator() const
{
	return Utf8Iterator(getRaw());
}

bbe::Utf8String bbe::Utf8String::replace(const Utf8String &searchString, const Utf8String &replaceString) const
{
	if (searchString == "") return *this;
	if (searchString == replaceString) return *this;

	bbe::Utf8String retVal = "";
	const size_t searchStringOccurences = count(searchString);
	retVal.m_data.growIfNeeded(getLengthBytes() + (replaceString.getLengthBytes() - searchString.getLengthBytes()) * searchStringOccurences + 1);
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
	bbe::Utf8String result;
	for (auto it = getIterator(); it.valid(); ++it)
	{
		result += fromCodePoint(mapWesternLatinCase(it.getCodepoint(), true));
	}
	*this = result;
}

void bbe::Utf8String::toLowerCaseInPlace()
{
	bbe::Utf8String result;
	for (auto it = getIterator(); it.valid(); ++it)
	{
		result += fromCodePoint(mapWesternLatinCase(it.getCodepoint(), false));
	}
	*this = result;
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

bbe::Utf8String &bbe::Utf8String::append(const bbe::Utf8String &other)
{
	const size_t oldLength = getLengthBytes();
	size_t totalLength = oldLength + other.getLengthBytes();
	m_data.growIfNeeded(totalLength + 1); // +1 for null terminator
	memmove(getRaw() + oldLength, other.getRaw(), other.getLengthBytes());
	getRaw()[totalLength] = '\0';
	return *this;
}

bbe::Utf8String &bbe::Utf8String::append(const bbe::Utf8String &other, size_t pos, size_t count)
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

bbe::Utf8String &bbe::Utf8String::append(const char *s)
{
	if (s == nullptr) bbe::Crash(bbe::Error::NullPointer);

	size_t sLengthBytes = strlen(s);
	const char *oldRaw = getRaw();
	const size_t oldLength = getLengthBytes();
	const bool aliasesSelf = s >= oldRaw && s <= oldRaw + oldLength;
	const size_t sourceOffset = aliasesSelf ? static_cast<size_t>(s - oldRaw) : 0;
	size_t totalLength = oldLength + sLengthBytes;
	m_data.growIfNeeded(totalLength + 1); // +1 for null terminator
	if (aliasesSelf)
	{
		s = getRaw() + sourceOffset;
	}
	memmove(getRaw() + oldLength, s, sLengthBytes);
	getRaw()[totalLength] = '\0';
	return *this;
}

bbe::Utf8String &bbe::Utf8String::append(const char *s, size_t count)
{
	if (s == nullptr) bbe::Crash(bbe::Error::NullPointer);

	size_t sLengthBytes = strlen(s);
	size_t actualCount = std::min(count, sLengthBytes);
	const char *oldRaw = getRaw();
	const size_t oldLength = getLengthBytes();
	const bool aliasesSelf = s >= oldRaw && s <= oldRaw + oldLength;
	const size_t sourceOffset = aliasesSelf ? static_cast<size_t>(s - oldRaw) : 0;
	size_t totalLength = oldLength + actualCount;
	m_data.growIfNeeded(totalLength + 1); // +1 for null terminator
	if (aliasesSelf)
	{
		s = getRaw() + sourceOffset;
	}
	memmove(getRaw() + oldLength, s, actualCount);
	getRaw()[totalLength] = '\0';
	return *this;
}

bbe::Utf8String &bbe::Utf8String::append(size_t count, char c)
{
	size_t totalLength = getLengthBytes() + count;
	m_data.growIfNeeded(totalLength + 1); // +1 for null terminator
	memset(getRaw() + getLengthBytes(), c, count);
	getRaw()[totalLength] = '\0';
	return *this;
}

bbe::Utf8String &bbe::Utf8String::append(const bbe::Utf8StringView &view)
{
	size_t otherLengthBytes = view.getLengthBytes();
	const size_t oldLength = getLengthBytes();
	size_t totalLength = oldLength + otherLengthBytes;
	m_data.growIfNeeded(totalLength + 1); // +1 for null terminator
	const char *otherRaw = utf8PointerAtIndex(view.m_pstring->getRaw(), view.m_start);
	memmove(getRaw() + oldLength, otherRaw, otherLengthBytes);
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
uint32_t bbe::hash(const bbe::String &t)
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
		_hash = ((_hash << 5) + _hash) + static_cast<uint32_t>(t.getCodepoint(i));
	}

	return _hash;
}

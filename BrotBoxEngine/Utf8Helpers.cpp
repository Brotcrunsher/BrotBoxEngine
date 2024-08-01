#include "BBE/Utf8Helpers.h"
#include "BBE/Utf8Iterator.h"
#include "BBE/Error.h"
#include "BBE/DataType.h"

int32_t bbe::utf8CharToCodePoint(const char* ptr)
{
	const std::size_t length = utf8charlen(ptr);
	const byte* bptr = reinterpret_cast<const byte*>(ptr);

	// TODO it's probably possible to do this a bit more clever.
	switch (length)
	{
	case 1: return *ptr;
	case 2: return ((bptr[0] & 0b00011111) <<  6) | ((bptr[1] & 0b00111111));
	case 3: return ((bptr[0] & 0b00001111) << 12) | ((bptr[1] & 0b00111111) <<  6) | ((bptr[2] & 0b00111111));
	case 4: return ((bptr[0] & 0b00000111) << 18) | ((bptr[1] & 0b00111111) << 12) | ((bptr[2] & 0b00111111) << 6) | ((bptr[3] & 0b00111111));
	}

	bbe::Crash(bbe::Error::IllegalArgument);
}

std::size_t bbe::utf8len(const char* ptr)
{
	if(ptr == nullptr)
	{
		bbe::Crash(bbe::Error::NullPointer);
	}
	std::size_t len = 0;
	bbe::Utf8Iterator iter(ptr);
	while (*iter != '\0')
	{
		len++;
		++iter;
	}
	return len;
}

std::size_t bbe::utf8len(const char* ptr, const char* end)
{
	if(ptr == nullptr)
	{
		bbe::Crash(bbe::Error::NullPointer);
	}
	std::size_t len = 0;
	const byte* bptr = reinterpret_cast<const byte*>(ptr);
	while(*bptr != (byte)0b00000000 && bptr != reinterpret_cast<const byte*>(end))
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
		bbe::Crash(bbe::Error::NullPointer);
	}
	const byte* bptr = reinterpret_cast<const byte*>(ptr);

	if(((*bptr) & (byte)0b10000000) == (byte)0b00000000) return 1;
	if(((*bptr) & (byte)0b11100000) == (byte)0b11000000) return 2;
	if(((*bptr) & (byte)0b11110000) == (byte)0b11100000) return 3;
	if(((*bptr) & (byte)0b11111000) == (byte)0b11110000) return 4;

	bbe::Crash(bbe::Error::NotStartOfUtf8);
}

std::size_t bbe::utf8codePointLen(int32_t codePoint)
{
	if (codePoint < 0) return 0;
	if (codePoint <= 0x7F) return 1;
	if (codePoint <= 0x7FF) return 2;
	if (codePoint <= 0xFFFF) return 3;
	if (codePoint <= 0x10FFFF) return 4;
	return 0; // Invalid code point
}

bool bbe::utf8IsStartOfChar(const char* ptr)
{
	if(ptr == nullptr)
	{
		bbe::Crash(bbe::Error::NullPointer);
	}
	const byte* bptr = reinterpret_cast<const byte*>(ptr);

	return ((*bptr & (byte)0b11000000) != (byte)0b10000000);
}

const char* bbe::utf8GetStartAddrOfCodePoint(const char* ptr)
{
	if(ptr == nullptr)
	{
		bbe::Crash(bbe::Error::NullPointer);
	}

	if(*ptr == '\0')
	{
		bbe::Crash(bbe::Error::UnexpectedEndOfString);
	}

	for(int i = 0; i<4; i++)
	{
		if(ptr[-i] == '\0') 
		{
			bbe::Crash(bbe::Error::NotAUtf8Char);
		}
		if(bbe::utf8IsStartOfChar(&ptr[-i])) 
		{
			return &ptr[-i];
		}
	}

	bbe::Crash(bbe::Error::NotAUtf8Char);
}

const char* bbe::utf8GetNextChar(const char* ptr)
{
	if(*ptr == '\0')
	{
		bbe::Crash(bbe::Error::UnexpectedEndOfString);
	}

	std::size_t length = bbe::utf8charlen(ptr);

	return ptr + length;
}

bool bbe::utf8IsSameChar(const char* ptr1, const char* ptr2)
{
	if(ptr1 == nullptr || ptr2 == nullptr) bbe::Crash(bbe::Error::NullPointer);
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
		bbe::Crash(bbe::Error::NullPointer);
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
	if(ptr == nullptr)
	{
		bbe::Crash(bbe::Error::NullPointer);
	}

	return (*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z');
}

bool bbe::utf8IsDigitChar(const char* ptr)
{
	if(ptr == nullptr)
	{
		bbe::Crash(bbe::Error::NullPointer);
	}

	return (*ptr >= '0' && *ptr <= '9');
}

bool bbe::utf8IsAsciiChar(const char* ptr)
{
	if(ptr == nullptr)
	{
		bbe::Crash(bbe::Error::NullPointer);
	}

	const byte* bptr = reinterpret_cast<const byte*>(ptr);

	return ((*bptr) & (byte)0b10000000) == (byte)0;
}

bool bbe::utf8IsSmallerCodePoint(const char* ptr1, const char* ptr2)
{
	if(ptr1 == nullptr || ptr2 == nullptr)
	{
		bbe::Crash(bbe::Error::NullPointer);
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

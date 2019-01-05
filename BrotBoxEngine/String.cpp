#include "stdafx.h"
#include "BBE/String.h"
#include "BBE/DataType.h"
#include "BBE/Exceptions.h"

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

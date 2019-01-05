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
		return 0;
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
		return 0;
	}
	const byte* bptr = reinterpret_cast<const byte*>(ptr);

	if(((*bptr) & (byte)0b10000000) == (byte)0b00000000) return 1;
	if(((*bptr) & (byte)0b11100000) == (byte)0b11000000) return 2;
	if(((*bptr) & (byte)0b11110000) == (byte)0b11100000) return 3;
	if(((*bptr) & (byte)0b11111000) == (byte)0b11110000) return 4;

	throw NotStartOfUtf8Exception();
}

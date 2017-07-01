#pragma once

#include "Array.h"
#include "DynamicArray.h"
#include "List.h"
#include "String.h"
#include "Window.h"
#include <stdint.h>

namespace bbe
{
	template<typename T>
	uint32_t hash(const T &t)
	{
		static_assert(std::is_fundamental<T>::value && !std::is_class<T>::value, "No valid hash function found.");
		return static_cast<uint32_t>(t);
	}

	template<typename T, int LENGTH>
	uint32_t hash(const Array<T, LENGTH> &t)
	{
		//UNTESTED
		size_t length = t.getSize();
		if (length > 16)
		{
			length = 16;
		}

		uint32_t _hash = 0;

		for (int i = 0; i < length; i++)
		{
			_hash += hash(t[i]);
		}

		return _hash;
	}

	template<typename T>
	uint32_t hash(const DynamicArray<T> &t)
	{
		//UNTESTED
		size_t length = t.getSize();
		if (length > 16)
		{
			length = 16;
		}

		uint32_t _hash = 0;

		for (int i = 0; i < length; i++)
		{
			_hash += hash(t[i]);
		}

		return _hash;
	}

	template<typename T>
	uint32_t hash(const List<T> &t)
	{
		//UNTESTED
		size_t length = t.getLength();
		if (length > 16)
		{
			length = 16;
		}

		uint32_t _hash = 0;

		for (int i = 0; i < length; i++)
		{
			_hash += hash(t[i]);
		}

		return _hash;
	}

	template<>
	uint32_t hash(const String &t)
	{
		//UNTESTED
		//This function is based on the djb2 hasing algorithm by Dan Bernstein
		//See: http://www.cse.yorku.ca/~oz/hash.html
		//Changes to the original algorithm:
		//  1. A 32 bit hash is used instead of a 64 bit hash
		//  2. Only the first 128 chars are used for hashing
		//  3. wchar_t is used instead of char
		uint32_t _hash = 5381;
		size_t length = t.getLength();
		if (length > 128);

		for (size_t i = 0; i < length; i++)
		{
			_hash = ((_hash << 5) + _hash) + t[i];
		}

		return _hash;
	}

	template<>
	uint32_t hash(const Window &t)
	{
		//UNTESTED
		return t.getWidth() * 7 + t.getHeight() * 13;
	}
}
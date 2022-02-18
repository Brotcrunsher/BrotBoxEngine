#pragma once

#include <cstring>
#include <cstdint>

namespace bbe
{
	std::size_t utf8len(const char* ptr);		//Length of a utf8 encoded string.
	std::size_t utf8len(const char* ptr, const char* end);
	std::size_t utf8charlen(const char* ptr);	//Length in byte of a single utf8 char.
	std::size_t utf8codePointLen(int32_t codePoint);
	int32_t utf8CharToCodePoint(const char* ptr);
	bool utf8IsStartOfChar(const char* ptr);
	const char* utf8GetStartAddrOfCodePoint(const char* ptr);
	const char* utf8GetNextChar(const char* ptr);
	bool utf8IsWhitespace(const char* ptr);
	bool utf8IsSameChar(const char* ptr1, const char* ptr2);
	bool utf8IsLatinChar(const char* ptr);
	bool utf8IsDigitChar(const char* ptr);
	bool utf8IsAsciiChar(const char* ptr);
	bool utf8IsSmallerCodePoint(const char* ptr1, const char* ptr2);
	int utf8Distance(const char* ptr1, const char* ptr2);
}

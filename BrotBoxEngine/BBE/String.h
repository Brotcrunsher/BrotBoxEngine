#pragma once

#include <string>
#include <cstring>
#include <cwchar>
#include <stdlib.h>
#include <iostream>
#include "../BBE/DynamicArray.h"
#include "../BBE/Array.h"
#include "../BBE/UtilDebug.h"
#include "../BBE/Hash.h"
#include "../BBE/Utf8Iterator.h"
#include "../BBE/ByteBuffer.h"
#include "../BBE/List.h"
#include "../BBE/SOOBlock.h"

namespace bbe
{
	class Utf8String;

	class Utf8StringView
	{
		friend class Utf8String;
	private:
		const Utf8String* m_pstring = nullptr;
		std::size_t m_start = 0;
		std::size_t m_end = 0;

		Utf8StringView();
		Utf8StringView(const Utf8String& string, std::size_t m_start, std::size_t m_end);

		std::size_t getEnd() const;
		std::size_t getLength() const;
		std::size_t getLengthBytes() const;
	};

	class Utf8String
	{
	private:
		static constexpr size_t BBE_UTF8STRING_SSOSIZE = 16;

		SOOBlock<char, BBE_UTF8STRING_SSOSIZE> m_data;

		void initializeFromCharArr(const char *data);

	public:
		Utf8String();
		/*nonexplicit*/ Utf8String(const char* data);
		/*nonexplicit*/ Utf8String(const wchar_t* data);
		explicit Utf8String(const std::string& data);
		explicit Utf8String(char               c);
		explicit Utf8String(double             number);
		explicit Utf8String(int                number);
		explicit Utf8String(long long          number);
		explicit Utf8String(long double        number);
		explicit Utf8String(float              number);
		explicit Utf8String(unsigned long long number);
		explicit Utf8String(unsigned long      number);
		explicit Utf8String(long               number);
		explicit Utf8String(unsigned int       number);

		/*nonexplicit*/ Utf8String(const std::initializer_list<char>& il);

		static Utf8String format(const char* format, ...);
		static Utf8String formatVa(const char* format, va_list va);

		void serialize(bbe::ByteBuffer& buffer) const;
		static bbe::Utf8String deserialize(bbe::ByteBufferSpan& buffer);

		static Utf8String fromCodePoint(int32_t codePoint);
		static Utf8String toHex(uint32_t value);

		bool operator==(const Utf8String& other) const;
		bool operator==(const char*       other) const;
		friend bool operator==(const char* arr, const Utf8String& string);
		bool operator!=(const Utf8String& other) const;
		bool operator!=(const char*       other) const;
		friend bool operator!=(const char* arr, const Utf8String& string);

		friend std::ostream& operator<<(std::ostream &os, const Utf8String &string);

		Utf8String operator+(const Utf8String& other)   const;
		Utf8String operator+(const char* other)         const;
		Utf8String operator+(double             number) const;
		Utf8String operator+(int                number) const;
		Utf8String operator+(long long          number) const;
		Utf8String operator+(long double        number) const;
		Utf8String operator+(float              number) const;
		Utf8String operator+(unsigned long long number) const;
		Utf8String operator+(unsigned long      number) const;
		Utf8String operator+(long               number) const;
		Utf8String operator+(unsigned int       number) const;
		friend Utf8String operator+(const char* other,         const Utf8String& string);
		friend Utf8String operator+(double             number, const Utf8String& string);
		friend Utf8String operator+(int                number, const Utf8String& string);
		friend Utf8String operator+(long long          number, const Utf8String& string);
		friend Utf8String operator+(long double        number, const Utf8String& string);
		friend Utf8String operator+(float              number, const Utf8String& string);
		friend Utf8String operator+(unsigned long long number, const Utf8String& string);
		friend Utf8String operator+(unsigned long      number, const Utf8String& string);
		friend Utf8String operator+(long               number, const Utf8String& string);
		friend Utf8String operator+(unsigned int       number, const Utf8String& string);

		Utf8String operator*(int32_t mult) const;

		Utf8String& operator+=(const Utf8String& other);
		Utf8String& operator+=(const Utf8StringView& other);
		Utf8String& operator+=(const std::string& other);
		Utf8String& operator+=(const char*       other);
		Utf8String& operator+=(char              c);
		Utf8String& operator+=(double             number);
		Utf8String& operator+=(int                number);
		Utf8String& operator+=(long long          number);
		Utf8String& operator+=(long double        number);
		Utf8String& operator+=(float              number);
		Utf8String& operator+=(unsigned long long number);
		Utf8String& operator+=(unsigned long      number);
		Utf8String& operator+=(long               number);
		Utf8String& operator+=(unsigned int       number);

		bbe::Utf8String rounded(int32_t digitsAfterDot) const;

		bbe::Utf8String trim       () const;
		void            trimInPlace();

		bbe::Utf8String substring       (std::size_t start, std::size_t end = -1) const;
		void            substringInPlace(std::size_t start, std::size_t end = -1);
		bbe::Utf8StringView substringView(std::size_t start, std::size_t end = -1) const;

		size_t count(const Utf8String& countand) const;
		size_t count(const char*       countand) const;

		DynamicArray<Utf8String> split(const Utf8String& splitAt, bool addEmpty = true) const;
		DynamicArray<Utf8String> split(const char*       splitAt, bool addEmpty = true) const;

		DynamicArray<Utf8String> lines(bool addEmpty = true) const;
		
		bool containsAny(const char* string) const;
		bool contains(const char*       string) const;
		bool contains(const Utf8String& string) const;
		bool containsIgnoreCase(const char*       string) const;
		bool containsIgnoreCase(const Utf8String& string) const;

		bbe::Utf8String hardBreakEvery(int32_t x) const;

		bool isTextAtLocation(const char* string, size_t index) const;
		bool startsWith(const char* string) const;
		bool startsWith(const bbe::Utf8String& string) const;
		bool endsWith(const char* string) const;

		int64_t search(const char*       string, int64_t startIndex = 0) const;
		int64_t search(const Utf8String& string, int64_t startIndex = 0) const;
		
		int64_t searchLast(const char* string) const;

		bool isNumber() const;
		bool isEmpty() const;

		long   toLong  (int base = 10) const;
		double toDouble() const;
		float  toFloat () const;

		//      char& operator[](size_t index);		THIS METHOD IS ILLEGAL ON PURPOSE! A single char can be multiple bytes long. Giving direct access to member variables could lead to problems when the programer tries to manipulate the string directly. TODO: Add a "Change char" method.
		const char& operator[](size_t index) const;
		
		bool operator<(const bbe::Utf8String& other) const;

		int32_t getCodepoint(size_t index) const;

		      char* getRaw();
		const char* getRaw() const;

		Utf8Iterator getIterator() const;

		bbe::Utf8String replace(const Utf8String& searchString, const Utf8String& replaceString) const;

		Utf8String toUpperCase() const;
		Utf8String toLowerCase() const;
		void toUpperCaseInPlace();
		void toLowerCaseInPlace();

		size_t getLength     () const;
		size_t getLengthBytes() const;
		size_t getCapacity   () const;

		void resizeCapacity(size_t newCapacity);

		Utf8String leftFill(char c, size_t length);

		Utf8String& append(const Utf8String& other);
		Utf8String& append(const Utf8String& other, size_t pos, size_t count = npos);
		Utf8String& append(const char* s);
		Utf8String& append(const char* s, size_t count);
		Utf8String& append(size_t count, char c);
		Utf8String& append(const Utf8StringView& view);
		static constexpr size_t npos = static_cast<size_t>(-1);

		std::string toStdString() const;
		std::wstring toStdWString() const;
	};

	typedef Utf8String String;

	template<>
	uint32_t hash(const String &t);
}

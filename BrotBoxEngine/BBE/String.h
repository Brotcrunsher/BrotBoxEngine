#pragma once

#include <cstring>
#include <cwchar>
#include <stdlib.h>
#include <iostream>
#include "../BBE/NewDeleteAllocator.h"
#include "../BBE/DynamicArray.h"
#include "../BBE/Array.h"
#include "../BBE/UtilDebug.h"
#include "../BBE/Hash.h"


namespace bbe
{

	std::size_t utf8len(const char* ptr);		//Length of a utf8 encoded string.
	std::size_t utf8charlen(const char* ptr);	//Length in byte of a single utf8 char.
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

	class Utf8String
	{
	private:
		static constexpr size_t BBE_UTF8STRING_SSOSIZE = 16;

		union
		{
			char *m_pdata;
			char  m_ssoData[BBE_UTF8STRING_SSOSIZE];
		} m_UNION;
		bool        m_usesSSO  = true;
		std::size_t m_length   = 0;
		std::size_t m_capacity = 0;

		void growIfNeeded(std::size_t newSize);
		void initializeFromCharArr(const char *data);

	public:
		Utf8String();
		/*nonexplicit*/ Utf8String(const char* data);		
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

		Utf8String(const Utf8String&  other);//Copy Constructor
		Utf8String(Utf8String&& other);      //Move Constructor

		Utf8String& operator=(const Utf8String&  other); //Copy Assignment
		Utf8String& operator=(Utf8String&& other);       //Move Assignment


		~Utf8String();

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

		Utf8String& operator+=(const Utf8String& other);
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

		bbe::Utf8String trim       () const;
		void            trimInPlace();

		bbe::Utf8String substring       (std::size_t start, std::size_t end = -1) const;
		void            substringInPlace(std::size_t start, std::size_t end = -1);

		size_t count(const Utf8String& countand) const;
		size_t count(const char*       countand) const;

		DynamicArray<Utf8String> split(const Utf8String& splitAt) const;
		DynamicArray<Utf8String> split(const char*       splitAt) const;

		bool contains(const char*       string) const;
		bool contains(const Utf8String& string) const;

		int64_t search(const char*       string) const;		//TODO
		int64_t search(const Utf8String& string) const;		//TODO

		long   toLong  (int base = 10) const;
		double toDouble() const;
		float  toFloat () const;

		//      char& operator[](size_t index);		THIS METHOD IS ILLEGAL ON PURPOSE! A single char can be multiple bytes long. Giving direct access to member variables could lead to problems when the programer tries to manipulate the string directly. TODO: Add a "Change char" method.
		const char& operator[](size_t index) const;

		      char* getRaw();
		const char* getRaw() const;


		Utf8String toUpperCase() const;
		Utf8String toLowerCase() const;
		void toUpperCaseInPlace();
		void toLowerCaseInPlace();

		size_t getLength     () const;
		size_t getLengthBytes() const;
		size_t getCapacity   () const;

		Utf8String leftFill(char c, size_t length);
	};

	typedef Utf8String String;

	template<>
	uint32_t hash(const String &t);
}

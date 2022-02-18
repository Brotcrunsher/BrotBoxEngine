#pragma once

#include <cstdint>

namespace bbe
{
	class Utf8Iterator
	{
	private:
		const char* m_pdata;

	public:
		Utf8Iterator();
		Utf8Iterator(const char* ptr);

		Utf8Iterator           (const Utf8Iterator& ) = default;
		Utf8Iterator           (      Utf8Iterator&&) = default;
		Utf8Iterator& operator=(const Utf8Iterator& ) = default;
		Utf8Iterator& operator=(      Utf8Iterator&&) = default;

		Utf8Iterator& operator++();
		Utf8Iterator& operator--();

		Utf8Iterator operator++(int);
		Utf8Iterator operator--(int);

		Utf8Iterator operator+(int32_t advancement) const;
		Utf8Iterator& operator+=(int32_t advancement);

		Utf8Iterator operator-(int32_t advancement) const;
		Utf8Iterator& operator-=(int32_t advancement);

		bool operator==(const Utf8Iterator& other) const;
		bool operator!=(const Utf8Iterator& other) const;
		bool operator< (const Utf8Iterator& other) const;
		bool operator> (const Utf8Iterator& other) const;
		bool operator<=(const Utf8Iterator& other) const;
		bool operator>=(const Utf8Iterator& other) const;

		bool operator==(const char* other) const;
		bool operator!=(const char* other) const;
		bool operator< (const char* other) const;
		bool operator> (const char* other) const;
		bool operator<=(const char* other) const;
		bool operator>=(const char* other) const;

		operator const char* () const;

		int32_t operator-(const Utf8Iterator& other) const;

		int32_t getCodepoint() const;

		const char& operator*() const;
	};
}

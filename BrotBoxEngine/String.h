#pragma once

#include <string>
#include <cwchar>
#include "DynamicArray.h"
#include "Array.h"

namespace bbe
{
	class String
	{
#define SSOSIZE (16)
		//TODO use allocators
	private:
		union
		{
			wchar_t *m_data;
			wchar_t m_ssoData[SSOSIZE];
		};
		bool m_usesSSO = true;
		size_t m_length = 0;
		size_t m_capacity;

		void growIfNeeded(size_t newSize) {
			if (getCapacity() < newSize) {
				size_t newCapa = newSize;
				if (newCapa < getCapacity() * 2) {
					newCapa = getCapacity() * 2;
				}
				wchar_t *newData = new wchar_t[newCapa];
				wmemcpy(newData, getRaw(), getCapacity());

				if (!m_usesSSO) {
					delete[] m_data;
				}

				m_usesSSO = false;
				m_capacity = newCapa;
				m_data = newData;
			}
		}

		void initializeFromWCharArr(const wchar_t *data)
		{
			//PO
			m_data = nullptr;
			m_capacity = 0;

			if (m_length == 0)
			{
				m_length = wcslen(data);
			}
			
			if (m_length < SSOSIZE)
			{
				wmemcpy(m_ssoData, data, m_length + 1);
				m_usesSSO = true;
				m_capacity = SSOSIZE;
			}
			else
			{
				m_data = new wchar_t[m_length + 1];
				wmemcpy(m_data, data, m_length + 1);
				m_usesSSO = false;
				m_capacity = m_length + 1;
			}
		}

		void initializeFromCharArr(const char *data)
		{
			//PO
			m_data = nullptr;
			m_capacity = 0;

			if (m_length == 0)
			{
				m_length = strlen(data);
			}
			
			if (m_length < SSOSIZE)
			{
				mbstowcs_s(0, m_ssoData, m_length + 1, data, m_length);
				m_usesSSO = true;
				m_capacity = SSOSIZE;
			}
			else
			{
				m_data = new wchar_t[m_length + 1];
				mbstowcs_s(0, m_data, m_length + 1, data, m_length);
				m_usesSSO = false;
				m_capacity = m_length + 1;
			}
			
		}

	public:
		String()
		{
			initializeFromWCharArr(L"");
		}

		template<int size>
		String(const Array<wchar_t, size>& arr)
		{
			//UNTESTED
			initializeFromWCharArr(arr.getRaw());
		}

		String(const DynamicArray<wchar_t>& arr)
		{
			//UNTESTED
			initializeFromWCharArr(arr.getRaw());
		}

		template<int size>
		String(const Array<char, size>& arr)
		{
			//UNTESTED
			initializeFromCharArr(arr.getRaw());
		}

		String(const DynamicArray<char>& arr)
		{
			//UNTESTED
			initializeFromCharArr(arr.getRaw());
		}

		String(const wchar_t *data)
		{
			//PO
			initializeFromWCharArr(data);
		}

		String(const char* data)
		{
			initializeFromCharArr(data);
		}

		String(const std::string &data)
		{
			m_length = data.length();
			initializeFromCharArr(data.c_str());
		}

		String(const std::wstring &data)
		{
			m_length = data.length();
			initializeFromWCharArr(data.c_str());
		}

		String(double number)
		{
			initializeFromCharArr(std::to_string(number).c_str());
		}

		String(int number)
		{
			initializeFromCharArr(std::to_string(number).c_str());
		}

		String(long long number)
		{
			initializeFromCharArr(std::to_string(number).c_str());
		}

		String(long double number)
		{
			initializeFromCharArr(std::to_string(number).c_str());
		}

		String(float number)
		{
			initializeFromCharArr(std::to_string(number).c_str());
		}

		String(unsigned long long number)
		{
			initializeFromCharArr(std::to_string(number).c_str());
		}

		String(unsigned long number)
		{
			initializeFromCharArr(std::to_string(number).c_str());
		}

		String(long number)
		{
			initializeFromCharArr(std::to_string(number).c_str());
		}

		String(unsigned int number)
		{
			initializeFromCharArr(std::to_string(number).c_str());
		}

		String(const String&  other)//Copy Constructor
		{ 
			m_length = other.getLength();
			if (other.m_usesSSO)
			{
				initializeFromWCharArr(other.m_ssoData);
			}
			else
			{
				initializeFromWCharArr(other.m_data);
			}
		}

		String(String&& other) //Move Constructor
		{
			m_length = other.m_length;
			m_usesSSO = other.m_usesSSO;
			if (other.m_usesSSO)
			{
				wmemcpy(m_ssoData, other.m_ssoData, SSOSIZE);
				m_capacity = SSOSIZE;
			}
			else
			{
				m_data = other.m_data;
				m_capacity = other.m_capacity;
				other.m_data = nullptr;
			}
			other.m_length = 0;
		}

		String& operator=(const String&  other) //Copy Assignment
		{ 
			if (!m_usesSSO && m_data != nullptr)
			{
				delete[] m_data;
			}

			m_length = other.getLength();
			if (other.m_usesSSO)
			{
				initializeFromWCharArr(other.m_ssoData);
			}
			else
			{
				initializeFromWCharArr(other.m_data);
			}
			return *this;
		}

		String& operator=(String&& other)//Move Assignment
		{ 
			if (!m_usesSSO && m_data != nullptr)
			{
				delete[] m_data;
			}
			
			m_length = other.m_length;
			m_usesSSO = other.m_usesSSO;

			if (other.m_usesSSO)
			{
				wmemcpy(m_ssoData, other.m_ssoData, SSOSIZE);
				m_capacity = SSOSIZE;
			}
			else
			{
				m_capacity = other.m_capacity;
				m_data = other.m_data;
			}
			
			other.m_data = nullptr;
			other.m_length = 0;
			return *this;
		}

		~String()
		{
			if (!m_usesSSO && m_data != nullptr)
			{
				delete[] m_data;
				m_data = nullptr;
			}
		}

		bool operator==(const String& other) const
		{
			return wcscmp(getRaw(), other.getRaw()) == 0;
		}

		bool operator==(const wchar_t* arr) const
		{
			return wcscmp(getRaw(), arr) == 0;
		}

		bool operator==(const char* arr) const
		{
			String localString(arr);
			return operator==(localString);
		}

		bool operator==(const std::string& str) const
		{
			String localString(str);
			return operator==(localString);
		}

		bool operator==(const std::wstring& str) const
		{
			String localString(str);
			return operator==(localString);
		}

		friend bool operator==(const wchar_t* arr, const String& a)
		{
			return a.operator==(arr);
		}

		friend bool operator==(const char* arr, const String& a)
		{
			return a.operator==(arr);
		}

		friend bool operator==(std::string& str, const String& a)
		{
			return a.operator==(str);
		}

		friend bool operator==(std::wstring& str, const String& a)
		{
			return a.operator==(str);
		}

		bool operator!=(const String& other) const
		{
			return !operator==(other);
		}

		bool operator!=(const wchar_t* arr) const
		{
			return !operator==(arr);
		}

		bool operator!=(const char* arr) const
		{
			return !operator==(arr);
		}

		bool operator!=(const std::string& str) const
		{
			return !operator==(str);
		}

		bool operator!=(const std::wstring& str) const
		{
			return !operator==(str);
		}

		friend std::ostream &operator<<(std::ostream &os, const bbe::String &string)
		{
			return os << string.getRaw();
		}

		friend bool operator!=(const wchar_t* arr, const String& string)
		{
			return string.operator!=(arr);
		}

		friend bool operator!=(const char* arr, const String& string)
		{
			return string.operator!=(arr);
		}

		friend bool operator!=(const std::string& str, const String& string)
		{
			return string.operator!=(str);
		}

		friend bool operator!=(const std::wstring& str, const String& string)
		{
			return string.operator!=(str);
		}

		String operator+(const String& other) const
		{
			//PO
			size_t totalLength = m_length + other.m_length;
			String retVal;
			retVal.m_length = totalLength;

			if (totalLength < SSOSIZE)
			{
				memcpy(retVal.m_ssoData, getRaw(), sizeof(wchar_t) * m_length);
				memcpy(retVal.m_ssoData + m_length, other.getRaw(), sizeof(wchar_t) * other.m_length);
				retVal.m_ssoData[totalLength] = 0;
			}
			else
			{
				wchar_t *newData = new wchar_t[totalLength + 1];
				memcpy(newData, getRaw(), sizeof(wchar_t) * m_length);
				memcpy(newData + m_length, other.getRaw(), sizeof(wchar_t) * other.m_length);
				newData[totalLength] = 0;

				retVal.m_usesSSO = false;
				retVal.m_data = newData;
			}
			return retVal;
		}

		String operator+(const std::string& other) const
		{
			return operator+(String(other));
		}

		String operator+(const std::wstring& other) const
		{
			return operator+(String(other));
		}

		String operator+(const wchar_t* other) const
		{
			return operator+(String(other));
		}

		String operator+(const char* other) const
		{
			return operator+(String(other));
		}

		String operator+(double number) const
		{
			return operator+(String(number));
		}

		String operator+(int number) const
		{
			return operator+(String(number));
		}

		String operator+(long long number) const
		{
			return operator+(String(number));
		}

		String operator+(long double number) const
		{
			return operator+(String(number));
		}

		String operator+(float number) const
		{
			return operator+(String(number));
		}

		String operator+(unsigned long long number) const
		{
			return operator+(String(number));
		}

		String operator+(unsigned long number) const
		{
			return operator+(String(number));
		}

		String operator+(long number) const
		{
			return operator+(String(number));
		}

		String operator+(unsigned int number) const
		{
			return operator+(String(number));
		}

		friend String operator+(const std::string& other, const String& string)
		{
			return String(other) + string;
		}

		friend String operator+(const std::wstring& other, const String& string)
		{
			return String(other) + string;
		}

		friend String operator+(const wchar_t* other, const String& string)
		{
			return String(other) + string;
		}

		friend String operator+(const char* other, const String& string)
		{
			return String(other) + string;
		}

		friend String operator+(double number, const String& string)
		{
			return String(number) + string;
		}

		friend String operator+(int number, const String& string)
		{
			return String(number) + string;
		}

		friend String operator+(long long number, const String& string)
		{
			return String(number) + string;
		}

		friend String operator+(long double number, const String& string)
		{
			return String(number) + string;
		}

		friend String operator+(float number, const String& string)
		{
			return String(number) + string;
		}

		friend String operator+(unsigned long long number, const String& string)
		{
			return String(number) + string;
		}

		friend String operator+(unsigned long number, const String& string)
		{
			return String(number) + string;
		}

		friend String operator+(long number, const String& string)
		{
			return String(number) + string;
		}

		friend String operator+(unsigned int number, const String& string)
		{
			return String(number) + string;
		}

		String& operator+=(const String& other)
		{
			size_t totalLength = m_length + other.m_length;
			size_t oldLength = m_length;
			m_length = totalLength;

			if (totalLength < SSOSIZE) {
				memcpy(m_ssoData + oldLength, other.getRaw(), sizeof(wchar_t) * other.m_length);
				m_ssoData[totalLength] = 0;
			}
			else {
				growIfNeeded(totalLength + 1);
				memcpy(getRaw() + oldLength, other.getRaw(), sizeof(wchar_t) * other.m_length);
				m_data[totalLength] = 0;
			}
			return *this;
		}

		String& operator+=(const std::string& other)
		{
			return operator+=(bbe::String(other));
		}

		String& operator+=(const std::wstring& other)
		{
			return operator+=(bbe::String(other));
		}

		String& operator+=(const wchar_t* other)
		{
			return operator+=(bbe::String(other));
		}

		String& operator+=(const char* other)
		{
			return operator+=(bbe::String(other));
		}

		String& operator+=(double number)
		{
			return operator+=(bbe::String(number));
		}

		String& operator+=(int number)
		{
			return operator+=(bbe::String(number));
		}

		String& operator+=(long long number)
		{
			return operator+=(bbe::String(number));
		}

		String& operator+=(long double number)
		{
			return operator+=(bbe::String(number));
		}

		String& operator+=(float number)
		{
			return operator+=(bbe::String(number));
		}

		String& operator+=(unsigned long long number)
		{
			return operator+=(bbe::String(number));
		}

		String& operator+=(unsigned long number)
		{
			return operator+=(bbe::String(number));
		}

		String& operator+=(long number)
		{
			return operator+=(bbe::String(number));
		}

		String& operator+=(unsigned int number)
		{
			return operator+=(bbe::String(number));
		}

		void trim()
		{
			size_t start = 0;
			size_t end = m_length - 1;
			auto raw = getRaw();

			while (iswspace(raw[start]))
			{
				start++;
			}
			while (iswspace(raw[end]) && end != 0)
			{
				end--;
			}

			if (start != 0 || end != m_length)
			{
				if (end == 0) //Special Case, if the string only contains whitespace
				{ 
					m_length = 0;
					raw[m_length] = 0;
				}
				else
				{
					m_length = end - start + 1;
					memmove(raw, &raw[start], sizeof(wchar_t) * m_length);
					raw[m_length] = 0;
				}
			}
		}

		size_t count(const String& countand) const
		{
			size_t countandLength = countand.getLength();
			if (countandLength == 0)
			{
				return 0;
			}
			size_t amount = 0;
			const wchar_t *readHead = getRaw();

			while ((readHead = wcsstr(readHead, countand.getRaw())) != nullptr)
			{
				amount++;
				readHead += countandLength;
			}
			return amount;
		}

		size_t count(const wchar_t* countand) const
		{
			return count(String(countand));
		}

		size_t count(char* countand) const
		{
			return count(String(countand));
		}

		size_t count(std::string countand) const
		{
			return count(String(countand));
		}

		size_t count(std::wstring countand) const
		{
			return count(String(countand));
		}

		DynamicArray<String> split(const String& splitAt) const
		{
			//TODO this method is a little mess. Clean it up!
			size_t counted = count(splitAt);
			if (counted == 0)
			{
				DynamicArray<String> retVal(1);
				retVal[0] = getRaw();
				return retVal;
			}
			DynamicArray<String> retVal(counted + 1);
			const wchar_t *previousFinding = getRaw();
			for (size_t i = 0; i < retVal.getLength() - 1; i++)
			{
				const wchar_t *currentFinding = wcsstr(previousFinding, splitAt.getRaw());
				String currentString;
				size_t currentStringLength = currentFinding - previousFinding;
				currentString.m_usesSSO = false; //TODO make this better! current string could use SSO!
				currentString.m_data = new wchar_t[currentStringLength + 1];
				memcpy(currentString.m_data, previousFinding, currentStringLength * sizeof(wchar_t));
				currentString.m_data[currentStringLength] = 0;
				currentString.m_length = currentStringLength;

				retVal[i] = currentString;

				previousFinding = currentFinding + splitAt.getLength();
			}

			String currentString;
			size_t currentStringLength = m_data + m_length - previousFinding;
			currentString.m_usesSSO = false; //TODO make this better! current string could use SSO!
			currentString.m_data = new wchar_t[currentStringLength + 1];
			memcpy(currentString.m_data, previousFinding, currentStringLength * sizeof(wchar_t));
			currentString.m_data[currentStringLength] = 0;
			currentString.m_length = currentStringLength;
			retVal[retVal.getLength() - 1] = currentString;

			return retVal;
		}

		DynamicArray<String> split(const wchar_t* splitAt) const
		{
			return split(String(splitAt));
		}

		DynamicArray<String> split(const char* splitAt) const
		{
			return split(String(splitAt));
		}

		DynamicArray<String> split(std::string splitAt) const
		{
			return split(String(splitAt));
		}

		DynamicArray<String> split(std::wstring splitAt) const
		{
			return split(String(splitAt));
		}

		bool contains(const wchar_t* string) const
		{
			return contains(String(string));
		}

		bool contains(const char* string) const
		{
			return contains(String(string));
		}

		bool contains(const std::string& string) const
		{
			return contains(String(string));
		}

		bool contains(const std::wstring& string) const
		{
			return contains(String(string));
		}

		bool contains(const String& string) const
		{
			return wcsstr(getRaw(), string.getRaw()) != nullptr;
		}

		int64_t search(const wchar_t* string) const
		{
			return search(String(string));
		}

		int64_t search(const char* string) const
		{
			return search(String(string));
		}

		int64_t search(const std::string& string) const
		{
			return search(String(string));
		}

		int64_t search(const std::wstring& string) const
		{
			return search(String(string));
		}

		int64_t search(const String& string) const
		{
			const wchar_t *found = wcsstr(getRaw(), string.getRaw());
			if (found == nullptr)
			{
				return -1;
			}

			return found - getRaw();
		}

		long toLong(int base = 10)
		{
			return wcstol(getRaw(), nullptr, base);
		}

		double toDouble()
		{
			return wcstod(getRaw(), nullptr);
		}

		float toFloat()
		{
			return wcstof(getRaw(), nullptr);
		}

		wchar_t& operator[](size_t index)
		{
			return getRaw()[index];
		}

		const wchar_t& operator[](size_t index) const
		{
			return getRaw()[index];
		}

		wchar_t* getRaw()
		{
			if (m_usesSSO)
			{
				return m_ssoData;
			}
			else
			{
				return m_data;
			}
		}

		const wchar_t* getRaw() const
		{
			if (m_usesSSO)
			{
				return m_ssoData;
			}
			else
			{
				return m_data;
			}
		}

		void toUpperCase()
		{
			auto raw = getRaw();
			for (size_t i = 0; i < m_length; i++)
			{
				raw[i] = towupper(raw[i]);
			}
		}

		void toLowerCase()
		{
			auto raw = getRaw();
			for (size_t i = 0; i < m_length; i++)
			{
				raw[i] = towlower(raw[i]);
			}
		}

		size_t getLength() const
		{
			return m_length;
		}

		size_t getCapacity() const
		{
			return m_capacity;
		}
	};


}
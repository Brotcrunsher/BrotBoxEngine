#pragma once

#include <string>

namespace bbe {
	class String {
		//TODO use allocators
	private:
		wchar_t *m_data = nullptr;
		size_t m_length = 0;

		void initializeFromWCharArr(const wchar_t *data) {
			if (m_length == 0) {
				m_length = wcslen(data);
			}
			m_data = new wchar_t[m_length + 1];
			memcpy(m_data, data, (m_length + 1) * sizeof(wchar_t));
		}

		void initializeFromCharArr(const char *data) {
			if (m_length == 0) {
				m_length = strlen(data);
			}
			m_data = new wchar_t[m_length + 1];
			mbstowcs_s(0, m_data, m_length + 1, data, m_length);
		}

	public:
		String()
		{
			initializeFromWCharArr(L"");
		}

		String(const wchar_t *data) {
			initializeFromWCharArr(data);
		}

		String(const char* data) {
			initializeFromCharArr(data);
		}

		String(const std::string &data) {
			m_length = data.length();
			initializeFromCharArr(data.c_str());
		}

		String(const std::wstring &data) {
			m_length = data.length();
			initializeFromWCharArr(data.c_str());
		}

		String(double value) {
			initializeFromCharArr(std::to_string(value).c_str());
		}

		String(const String&  other) { //Copy Constructor
			m_length = other.getLength();
			initializeFromWCharArr(other.m_data);
		}

		String(String&& other) { //Move Constructor
			m_length = other.m_length;
			m_data = other.m_data;
			other.m_data = nullptr;
		}

		String& operator=(const String&  other) { //Copy Assignment
			m_length = other.getLength();
			initializeFromWCharArr(other.m_data);
		}

		String& operator=(String&& other) { //Move Assignment
			m_length = other.m_length;
			m_data = other.m_data;
			other.m_data = nullptr;
			return *this;
		}

		~String() {
			if (m_data != nullptr) {
				delete[] m_data;
			}
		}

		bool operator==(const String& other) const {
			return wcscmp(m_data, other.m_data) == 0;
		}

		bool operator==(const wchar_t* arr) const {
			return wcscmp(m_data, arr) == 0;
		}

		bool operator==(const char* arr) const {
			String localString(arr);
			return operator==(localString);
		}

		friend bool operator==(const wchar_t* arr, const String& a) {
			return a.operator==(arr);
		}

		friend bool operator==(const char* arr, const String& a) {
			return a.operator==(arr);
		}

		bool operator!=(const String& other) const {
			return !operator==(other);
		}

		bool operator!=(const wchar_t* arr) const {
			return !operator==(arr);
		}

		bool operator!=(const char* arr) const {
			return !operator==(arr);
		}

		friend bool operator!=(const wchar_t* arr, const String& string) {
			return string.operator!=(arr);
		}

		friend bool operator!=(const char* arr, const String& string) {
			return string.operator!=(arr);
		}

		String operator+(const String& other) const {
			size_t totalLength = m_length + other.m_length;
			wchar_t *newData = new wchar_t[totalLength + 1];
			memcpy(newData, m_data, sizeof(wchar_t) * m_length);
			memcpy(newData + m_length, other.m_data, sizeof(wchar_t) * other.m_length);
			newData[totalLength] = 0;
			String retVal;
			delete[] retVal.m_data;
			retVal.m_data = newData;
			retVal.m_length = totalLength;
			return retVal;
		}

		String operator+(const std::string& other) const {
			return operator+(String(other));
		}

		String operator+(const wchar_t* other) const {
			return operator+(String(other));
		}

		String operator+(const char* other) const {
			return operator+(String(other));
		}

		String operator+(double number) const {
			return operator+(String(number));
		}

		friend String operator+(const std::string& other, const String& string) {
			return string + other;
		}

		friend String operator+(const wchar_t* other, const String& string) {
			return string + other;
		}

		friend String operator+(const char* other, const String& string) {
			return string + other;
		}

		friend String operator+(double number, const String& string) {
			return string + number;
		}

		String& operator+=(const String& other) {
			*this = *this + other;
		}

		String& operator+=(const std::string& other) {
			*this = *this + other;
		}

		String& operator+=(const wchar_t* other) {
			*this = *this + other;
		}

		String& operator+=(const char* other) {
			*this = *this + other;
		}

		String& operator+=(double number) {
			*this = *this + number;
		}

		void trim() {
			int start = 0;
			int end = m_length - 1;
			while (iswspace(m_data[start])) {
				start++;
			}
			while (iswspace(m_data[end]) && end != 0) {
				end--;
			}

			if (start != 0 || end != m_length) {
				if (end == 0) { //Special Case, if the string only contains whitespace
					m_length = 0;
					m_data[m_length] = 0;
				}
				else {
					m_length = end - start + 1;
					memmove(m_data, &m_data[start], sizeof(wchar_t) * m_length);
					m_data[m_length] = 0;
				}
			}
		}

		long toLong(int base = 10) {
			return wcstol(m_data, nullptr, base);
		}

		double toDouble() {
			return wcstod(m_data, nullptr);
		}

		float toFloat() {
			return wcstof(m_data, nullptr);
		}

		wchar_t& operator[](size_t index) {
			return m_data[index];
		}

		wchar_t* getRaw() {
			return m_data;
		}

		size_t getLength() const{
			return m_length;
		}
	};


}
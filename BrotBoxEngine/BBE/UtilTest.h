#pragma once

#include "../BBE/UtilDebug.h"
#include "../BBE/String.h"
#include "../BBE/Hash.h"
#include <iostream>

namespace bbe
{
	namespace test
	{
		class Person;
	}
}

namespace bbe {
	namespace test {

		class ForceException {};

		class Person
		{
		public:
			static int     s_nextPersonIndex;
			static int64_t s_amountOfPersons;
			static int64_t s_amountOfDefaulConstructorCalls;
			static int64_t s_amountOfCopyConstructorCalls;
			static int64_t s_amountOfMoveConstructorCalls;
			static int64_t s_amountOfCopyAssignmentCalls;
			static int64_t s_amountOfMoveAssignmentCalls;
			static int64_t s_amountOfParameterConstructorCalls;
			static int64_t s_amountOfDestructorCalls;
			bbe::String name;
			bbe::String address;
			int age;
			bool destructed = false;
			int personIndex = s_nextPersonIndex++;

			static inline void resetTestStatistics() {
				s_amountOfPersons = 0;
				s_amountOfDefaulConstructorCalls = 0;
				s_amountOfCopyConstructorCalls = 0;
				s_amountOfMoveConstructorCalls = 0;
				s_amountOfCopyAssignmentCalls = 0;
				s_amountOfMoveAssignmentCalls = 0;
				s_amountOfParameterConstructorCalls = 0;
				s_amountOfDestructorCalls = 0;
			}

			inline Person()
				: age(0)
			{
				//std::cout << "Constructor called!" << std::endl;
				Person::s_amountOfPersons++;
				Person::s_amountOfDefaulConstructorCalls++;
			}

			inline Person(const Person& other)
				: name(other.name), address(other.address), age(other.age)
			{
				Person::s_amountOfPersons++;
				Person::s_amountOfCopyConstructorCalls++;
			}

			inline Person(Person&& other)
				: name(other.name), address(other.address), age(other.age)
			{
				Person::s_amountOfPersons++;
				Person::s_amountOfMoveConstructorCalls++;
			}

			inline Person& operator=(const Person& other) {
				if (destructed)
				{
					debugBreak(); //Destructed Object got equal to other object
				}
				name = other.name;
				address = other.address;
				age = other.age;
				Person::s_amountOfCopyAssignmentCalls++;
				return *this;
			}

			inline Person& operator=(Person&& other) {
				if (destructed)
				{
					debugBreak(); //Destructed Object got equal to other object
				}

				name = std::move(other.name);
				address = std::move(other.address);
				age = other.age;
				Person::s_amountOfMoveAssignmentCalls++;
				return *this;
			}

			explicit inline Person(const bbe::String &name, const bbe::String &address, int age)
				: name(name), address(address), age(age)
			{
				//std::cout << "Constructor with parameters called!" << std::endl;
				s_amountOfPersons++;
				Person::s_amountOfParameterConstructorCalls++;
			}

			explicit inline Person(ForceException fe)
				:name("Will throw Name"), address("Will throw street"), age(-1)
			{
				s_amountOfPersons++;
				s_amountOfPersons--;
				throw (int)1;
			}

			inline ~Person() {
				if (destructed)
				{
					debugBreak();	//Was already destructed!
				}
				destructed = true;
				s_amountOfPersons--;
				Person::s_amountOfDestructorCalls++;
			}

			void inline print() {
				std::cout << "name: " << name << " address: " << address << " age: " << age << std::endl;
			}

			bool inline operator==(const Person& other) const {
				//std::cout << "Comparing---" << std::endl;
				//std::wcout << this->name    << " " << other.name    << (this->name    == other.name)    << std::endl;
				//std::wcout << this->address << " " << other.address << (this->address == other.address) << std::endl;
				//std::wcout << this->age     << " " << other.age     << (this->age     == other.age)     << std::endl;
				bool same = name == other.name
					&& address == other.address
					&& age == other.age;
				//std::cout << "Comparison: " << same << std::endl;
				return same;
			}

			bool inline operator!=(const Person& other) const {
				return !operator==(other);
			}

			bool inline operator>(const Person& other) const {
				return age > other.age;
			}

			bool inline operator>=(const Person& other) const {
				return age >= other.age;
			}

			bool inline operator<(const Person& other) const {
				return age < other.age;
			}

			bool inline operator<=(const Person& other) const {
				return age <= other.age;
			}

			static inline void checkIfAllPersonsWereDestroyed() {
				if (s_amountOfPersons != 0) {
					debugBreak();
				}
			}
		};

		std::ostream& operator<<(std::ostream& ostrm, const bbe::test::Person& p);

		int     inline Person::s_nextPersonIndex = 0;
		int64_t inline Person::s_amountOfPersons = 0;
		int64_t inline Person::s_amountOfDefaulConstructorCalls = 0;
		int64_t inline Person::s_amountOfCopyConstructorCalls = 0;
		int64_t inline Person::s_amountOfMoveConstructorCalls = 0;
		int64_t inline Person::s_amountOfCopyAssignmentCalls = 0;
		int64_t inline Person::s_amountOfMoveAssignmentCalls = 0;
		int64_t inline Person::s_amountOfParameterConstructorCalls = 0;
		int64_t inline Person::s_amountOfDestructorCalls = 0;

		template <typename T, typename U>
		void assertEqualsImpl(const char* file, int32_t line, const T &a, const U &b) {
			if (a == b) {
				//Do nothing, test passed
			}
			else {
				std::cout << "a=" << a << " b=" << b << std::endl;
				debugBreakImpl(file, line);
			}
		}
#define assertEquals(a, b) bbe::test::assertEqualsImpl(__FILE__, __LINE__, (a), (b))

		template <typename T, typename U>
		void assertEqualsFloatImpl(const char* file, int32_t line, T a, U b, T epsilon = 0.01f) {
			T diff = a - b;

			if (diff > -epsilon && diff < epsilon) {
				//Do nothing, test passed
			}
			else {
				std::cout << "a=" << a << " b=" << b << std::endl;
				debugBreakImpl(file, line);
			}
		}
#define assertEqualsFloat(a, b) bbe::test::assertEqualsFloatImpl(__FILE__, __LINE__, (a), (b))

		template <typename T, typename U>
		void assertUnequalsImpl(const char* file, int32_t line, T a, U b) {
			if (a != b) {
				//Do nothing, test passed
			}
			else {
				std::cout << "a=" << a << " b=" << b << std::endl;
				debugBreakImpl(file, line);
			}
		}
#define assertUnequals(a, b) bbe::test::assertUnequalsImpl(__FILE__, __LINE__, (a), (b))

		template <typename T, typename U>
		void assertGreaterThanImpl(const char* file, int32_t line, T a, U b) {
			if (a > b) {
				//Do nothing, test passed
			}
			else {
				std::cout << "a=" << a << " b=" << b << std::endl;
				debugBreakImpl(file, line);
			}
		}
#define assertGreaterThan(a, b) bbe::test::assertGreaterThanImpl(__FILE__, __LINE__, (a), (b))

		template <typename T, typename U>
		void assertGreaterEqualsImpl(const char* file, int32_t line, T a, U b) {
			if (a >= b) {
				//Do nothing, test passed
			}
			else {
				std::cout << "a=" << a << " b=" << b << std::endl;
				debugBreakImpl(file, line);
			}
		}
#define assertGreaterEquals(a, b) bbe::test::assertGreaterEqualsImpl(__FILE__, __LINE__, (a), (b))

		template <typename T, typename U>
		void assertLessThanImpl(const char* file, int32_t line, T a, U b) {
			if (a < b) {
				//Do nothing, test passed
			}
			else {
				std::cout << "a=" << a << " b=" << b << std::endl;
				debugBreakImpl(file, line);
			}
		}
#define assertLessThan(a, b) bbe::test::assertLessThanImpl(__FILE__, __LINE__, (a), (b))

		template <typename T, typename U>
		void assertLessEqualsImpl(const char* file, int32_t line, T a, U b) {
			if (a <= b) {
				//Do nothing, test passed
			}
			else {
				std::cout << "a=" << a << " b=" << b << std::endl;
				debugBreakImpl(file, line);
			}
		}
#define assertLessEquals(a, b) bbe::test::assertLessEqualsImpl(__FILE__, __LINE__, (a), (b))
	}

	template<>
	uint32_t inline hash(const test::Person &person)
	{
		uint32_t hashValue = hash(person.age);
		hashValue += hash(person.address);
		hashValue += hash(person.name);

		return hashValue;
	}
}

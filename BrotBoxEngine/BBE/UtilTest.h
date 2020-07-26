#pragma once

#include "../BBE/UtilDebug.h"
#include "../BBE/String.h"
#include "../BBE/Hash.h"
#include <iostream>

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
			bbe::String adress;
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
				: name(other.name), adress(other.adress), age(other.age)
			{
				Person::s_amountOfPersons++;
				Person::s_amountOfCopyConstructorCalls++;
			}

			inline Person(Person&& other)
				: name(other.name), adress(other.adress), age(other.age)
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
				adress = other.adress;
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
				adress = std::move(other.adress);
				age = other.age;
				Person::s_amountOfMoveAssignmentCalls++;
				return *this;
			}

			explicit inline Person(const bbe::String &name, const bbe::String &adress, int age)
				: name(name), adress(adress), age(age) 
			{
				//std::cout << "Constructor with parameters called!" << std::endl;
				s_amountOfPersons++;
				Person::s_amountOfParameterConstructorCalls++;
			}

			explicit inline Person(ForceException fe)
				:name("Will throw Name"), adress("Will throw street"), age(-1)
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
				std::cout << "name: " << name << " adress: " << adress << " age: " << age << std::endl;
			}

			bool inline operator==(const Person& other) const {
				//std::cout << "Comparing---" << std::endl;
				//std::wcout << this->name   << " " << other.name   << (this->name   == other.name)   << std::endl;
				//std::wcout << this->adress << " " << other.adress << (this->adress == other.adress) << std::endl;
				//std::wcout << this->age    << " " << other.age    << (this->age    == other.age)    << std::endl;
				bool same = name == other.name
					&& adress == other.adress
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
		void assertEquals(const T &a, const U &b) {
			if (a == b) {
				//Do nothing, test passed
			}
			else {
				debugBreak();
			}
		}

		template <typename T, typename U>
		void assertEqualsFloat(T a, U b, T epsilon = 0.01f) {
			T diff = a - b;

			if (diff > -epsilon && diff < epsilon) {
				//Do nothing, test passed
			}
			else {
				debugBreak();
			}
		}

		template <typename T, typename U>
		void assertUnequals(T a, U b) {
			if (a != b) {
				//Do nothing, test passed
			}
			else {
				debugBreak();
			}
		}

		template <typename T, typename U>
		void assertGreaterThan(T a, U b) {
			if (a > b) {
				//Do nothing, test passed
			}
			else {
				debugBreak();
			}
		}

		template <typename T, typename U>
		void assertGreaterEquals(T a, U b) {
			if (a >= b) {
				//Do nothing, test passed
			}
			else {
				debugBreak();
			}
		}

		template <typename T, typename U>
		void assertLessThan(T a, U b) {
			if (a < b) {
				//Do nothing, test passed
			}
			else {
				debugBreak();
			}
		}

		template <typename T, typename U>
		void assertLessEquals(T a, U b) {
			if (a <= b) {
				//Do nothing, test passed
			}
			else {
				debugBreak();
			}
		}
	}

	template<>
	uint32_t inline hash(const test::Person &person)
	{
		uint32_t hashValue = hash(person.age);
		hashValue += hash(person.adress);
		hashValue += hash(person.name);

		return hashValue;
	}
}
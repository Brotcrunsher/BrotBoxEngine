#pragma once

#include "UtilDebug.h"
#include "String.h"

namespace bbe {
	namespace test {

		class ForceException{};

		class Person
		{
		public:
			static size_t amountOfPersons;
			static size_t amountOfDefaulConstructorCalls;
			static size_t amountOfCopyConstructorCalls;
			static size_t amountOfMoveConstructorCalls;
			static size_t amountOfCopyAssignmentCalls;
			static size_t amountOfMoveAssignmentCalls;
			static size_t amountOfParameterConstructorCalls;
			static size_t amountOfDestructorCalls;
			bbe::String name;
			bbe::String adress;
			int age;

			static void resetTestStatistics() {
				amountOfPersons = 0;
				amountOfDefaulConstructorCalls = 0;
				amountOfCopyConstructorCalls = 0;
				amountOfMoveConstructorCalls = 0;
				amountOfCopyAssignmentCalls = 0;
				amountOfMoveAssignmentCalls = 0;
				amountOfParameterConstructorCalls = 0;
				amountOfDestructorCalls = 0;
			}

			Person()
				: age(0)
			{
				//std::cout << "Constructor called!" << std::endl;
				Person::amountOfPersons++;
				Person::amountOfDefaulConstructorCalls++;
			}

			Person(const Person& other) {
				name = other.name;
				adress = other.adress;
				age = other.age;
				Person::amountOfPersons++;
				Person::amountOfCopyConstructorCalls++;
			}

			Person(Person&& other) {
				name = other.name;
				adress = other.adress;
				age = other.age;
				Person::amountOfPersons++;
				Person::amountOfMoveConstructorCalls++;
			}

			Person& operator=(const Person& other) {
				name = other.name;
				adress = other.adress;
				age = other.age;
				Person::amountOfPersons++;
				Person::amountOfCopyAssignmentCalls++;
				return *this;
			}

			Person& operator=(Person&& other) {
				name = std::move(other.name);
				adress = std::move(other.adress);
				age = other.age;
				Person::amountOfMoveAssignmentCalls++;
				return *this;
			}

			explicit Person(bbe::String name, bbe::String adress, int age) :
				name(name), adress(adress), age(age) {
				//std::cout << "Constructor with parameters called!" << std::endl;
				amountOfPersons++;
				Person::amountOfParameterConstructorCalls++;
			}

			explicit Person(ForceException fe)
				:name("Will throw Name"), adress("Will throw street"), age(-1)
			{
				amountOfPersons++;
				amountOfPersons--;
				throw 1;
			}

			~Person() {
				amountOfPersons--;
				Person::amountOfDestructorCalls++;
			}

			void print() {
				std::cout << "name: " << name << " adress: " << adress << " age: " << age << std::endl;
			}

			bool operator==(const Person& other) const {
				return name == other.name
					&& adress == other.adress
					&& age == other.age;
			}

			bool operator!=(const Person& other) const {
				return !operator==(other);
			}

			bool operator>(const Person& other) const {
				return age > other.age;
			}

			bool operator>=(const Person& other) const {
				return age >= other.age;
			}

			bool operator<(const Person& other) const {
				return age < other.age;
			}

			bool operator<=(const Person& other) const {
				return age <= other.age;
			}

			static void checkIfAllPersonsWereDestroyed() {
				if (amountOfPersons != 0) {
					debugBreak();
				}
			}
		};
		size_t Person::amountOfPersons = 0;
		size_t Person::amountOfDefaulConstructorCalls = 0;
		size_t Person::amountOfCopyConstructorCalls = 0;
		size_t Person::amountOfMoveConstructorCalls = 0;
		size_t Person::amountOfCopyAssignmentCalls = 0;
		size_t Person::amountOfMoveAssignmentCalls = 0;
		size_t Person::amountOfParameterConstructorCalls = 0;
		size_t Person::amountOfDestructorCalls = 0;

		template <typename T, typename U>
		void assertEquals(T a, U b) {
			if (a == b) {
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
}
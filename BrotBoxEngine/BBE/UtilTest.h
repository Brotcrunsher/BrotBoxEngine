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
			static int nextPersonIndex;

			static int64_t amountOfPersons;
			static int64_t amountOfDefaulConstructorCalls;
			static int64_t amountOfCopyConstructorCalls;
			static int64_t amountOfMoveConstructorCalls;
			static int64_t amountOfCopyAssignmentCalls;
			static int64_t amountOfMoveAssignmentCalls;
			static int64_t amountOfParameterConstructorCalls;
			static int64_t amountOfDestructorCalls;
			bbe::String name;
			bbe::String adress;
			int age;
			bool destructed = false;
			int personIndex = nextPersonIndex++;

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
				if (destructed)
				{
					debugBreak(); //Destructed Object got equal to other object
				}
				name = other.name;
				adress = other.adress;
				age = other.age;
				Person::amountOfCopyAssignmentCalls++;
				return *this;
			}

			Person& operator=(Person&& other) {
				if (destructed)
				{
					debugBreak(); //Destructed Object got equal to other object
				}

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
				if (destructed)
				{
					debugBreak();	//Was already destructed!
				}
				destructed = true;
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
		int Person::nextPersonIndex = 0;
		int64_t Person::amountOfPersons = 0;
		int64_t Person::amountOfDefaulConstructorCalls = 0;
		int64_t Person::amountOfCopyConstructorCalls = 0;
		int64_t Person::amountOfMoveConstructorCalls = 0;
		int64_t Person::amountOfCopyAssignmentCalls = 0;
		int64_t Person::amountOfMoveAssignmentCalls = 0;
		int64_t Person::amountOfParameterConstructorCalls = 0;
		int64_t Person::amountOfDestructorCalls = 0;

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
		void assertEqualsFloat(T a, U b, T epsilon = 0.01) {
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
	uint32_t hash(const test::Person &person)
	{
		uint32_t hashValue = hash(person.age);
		hashValue += hash(person.adress);
		hashValue += hash(person.name);

		return hashValue;
	}
}
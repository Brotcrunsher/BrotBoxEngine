#pragma once

#include "UtilDebug.h"

namespace bbe {
	namespace test {


		class Person
		{
			static size_t amountOfPersons;
		public:
			std::string name;
			std::string adress;
			int age;

			Person() {
				//std::cout << "Constructor called!" << std::endl;
				Person::amountOfPersons++;
			}

			Person(std::string name, std::string adress, int age) :
				name(name), adress(adress), age(age) {
				//std::cout << "Constructor with parameters called!" << std::endl;
				amountOfPersons++;
			}

			~Person() {
				//std::cout << "Destructor called!" << std::endl;
				amountOfPersons--;
			}

			void print() {
				std::cout << "name: " << name << " adress: " << adress << " age: " << age << std::endl;
			}

			static void checkIfAllPersonsWereDestroyed() {
				if (amountOfPersons != 0) {
					debugBreak();
				}
			}
		};
		size_t Person::amountOfPersons = 0;

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
#pragma once

#include "BBE/UniquePointer.h"
#include "BBE/UtilTest.h"
#include "BBE/PoolAllocator.h"
#include "BBE/GeneralPurposeAllocator.h"

namespace bbe {
	namespace test {
		void testUniquePointer() {
			Person::checkIfAllPersonsWereDestroyed();	//if this triggers then another test made a bubu!

			{
				UniquePointer<Person> up(new Person("My Name", "My Address", -182));
				assertEquals(up->adress, "My Address");
				assertEquals(up->name, "My Name");
				assertEquals(up->age, -182);

				up->adress = "New Address";
				up->name = "New Name";
				up->age = 2910;

				assertEquals(up->adress, "New Address");
				assertEquals(up->name, "New Name");
				assertEquals(up->age, 2910);

				assertUnequals(up.getRaw(), nullptr);

				UniquePointer<Person> up2(std::move(up));
				assertEquals(up.getRaw(), nullptr);
				assertEquals(up2->adress, "New Address");
				assertEquals(up2->name, "New Name");
				assertEquals(up2->age, 2910);
			}
			
			Person::checkIfAllPersonsWereDestroyed();

			{
				PoolAllocator<Person> personenAllocator;

				{
					auto p1 = personenAllocator.allocateObjectUniquePointer("Name 1", "Addr 1", 1);
					auto p2 = personenAllocator.allocateObjectUniquePointer("Name 2", "Addr 2", 2);

					assertEquals(p1->name, "Name 1");
					assertEquals(p1->adress, "Addr 1");
					assertEquals(p1->age, 1);

					assertEquals(p2->name, "Name 2");
					assertEquals(p2->adress, "Addr 2");
					assertEquals(p2->age, 2);
				}

				Person::checkIfAllPersonsWereDestroyed();
			}

			Person::checkIfAllPersonsWereDestroyed();

			{
				GeneralPurposeAllocator gpa;
				{
					auto p1 = gpa.allocateObjectsUniquePointer<Person>(1, "Name 3", "Addr 3", 3);
					auto p2 = gpa.allocateObjectsUniquePointer<Person>(1, "Name 4", "Addr 4", 4);

					assertEquals(p1->name, "Name 3");
					assertEquals(p1->adress, "Addr 3");
					assertEquals(p1->age, 3);

					assertEquals(p2->name, "Name 4");
					assertEquals(p2->adress, "Addr 4");
					assertEquals(p2->age, 4);
				}
				Person::checkIfAllPersonsWereDestroyed();
			}

			Person::checkIfAllPersonsWereDestroyed();

			{
				GeneralPurposeAllocator gpa;
				{
					auto p1 = gpa.allocateObjectUniquePointer<Person>("Name 5", "Addr 5", 5);
					auto p2 = gpa.allocateObjectUniquePointer<Person>("Name 6", "Addr 6", 6);

					assertEquals(p1->name, "Name 5");
					assertEquals(p1->adress, "Addr 5");
					assertEquals(p1->age, 5);

					assertEquals(p2->name, "Name 6");
					assertEquals(p2->adress, "Addr 6");
					assertEquals(p2->age, 6);
				}
				Person::checkIfAllPersonsWereDestroyed();
			}

			Person::checkIfAllPersonsWereDestroyed();
		}
	}
}
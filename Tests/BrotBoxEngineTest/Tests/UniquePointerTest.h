#pragma once

#include "BBE/UniquePointer.h"
#include "BBE/UtilTest.h"

namespace bbe {
	namespace test {
		void testUniquePointer() {
			Person::checkIfAllPersonsWereDestroyed();	//if this triggers then another test made a bubu!

			{
				UniquePointer<Person> up(new Person("My Name", "My Address", -182));
				assertEquals(up->address, "My Address");
				assertEquals(up->name, "My Name");
				assertEquals(up->age, -182);

				up->address = "New Address";
				up->name = "New Name";
				up->age = 2910;

				assertEquals(up->address, "New Address");
				assertEquals(up->name, "New Name");
				assertEquals(up->age, 2910);

				assertUnequals(up.getRaw(), nullptr);

				UniquePointer<Person> up2(std::move(up));
				assertEquals(up.getRaw(), nullptr);
				assertEquals(up2->address, "New Address");
				assertEquals(up2->name, "New Name");
				assertEquals(up2->age, 2910);
			}
			
			Person::checkIfAllPersonsWereDestroyed();
		}
	}
}
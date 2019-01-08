#pragma once

#include "BBE/StackAllocator.h"
#include <iostream>
#include <string>
#include "BBE/UtilTest.h"

namespace bbe {
	namespace test {
		void testStackAllocator() {
			bbe::StackAllocator<> sa(sizeof(Person) * 128);
			auto startMarker = sa.getMarker();
			Person* pArr = sa.allocateObjects<Person>(5);
			float* floatData = (float*)sa.allocate(sizeof(float) * 100, alignof(float));

			for (size_t i = 0; i < 100; i++) {
				floatData[i] = (float)i + 100.0f;
			}

			pArr[0].name = "Hugo";
			pArr[1].name = "Ebert";
			pArr[2].name = "Lel";
			pArr[3].name = "Aha?";
			pArr[4].name = "Okay";

			pArr[0].adress = "AStr";
			pArr[1].adress = "BStr";
			pArr[2].adress = "CStr";
			pArr[3].adress = "DStr";
			pArr[4].adress = "EStr";

			pArr[0].age = 1;
			pArr[1].age = 2;
			pArr[2].age = 3;
			pArr[3].age = 4;
			pArr[4].age = 5;

			assertEquals(pArr[0].name, "Hugo");
			assertEquals(pArr[1].name, "Ebert");
			assertEquals(pArr[2].name, "Lel");
			assertEquals(pArr[3].name, "Aha?");
			assertEquals(pArr[4].name, "Okay");

			assertEquals(pArr[0].adress, "AStr");
			assertEquals(pArr[1].adress, "BStr");
			assertEquals(pArr[2].adress, "CStr");
			assertEquals(pArr[3].adress, "DStr");
			assertEquals(pArr[4].adress, "EStr");

			assertEquals(pArr[0].age, 1);
			assertEquals(pArr[1].age, 2);
			assertEquals(pArr[2].age, 3);
			assertEquals(pArr[3].age, 4);
			assertEquals(pArr[4].age, 5);

			for (size_t i = 0; i < 100; i++) {
				assertEquals(floatData[i], (float)i + 100.0f);
			}

			Person* pOut1 = sa.allocateObject<Person>();
			Person* pOut2 = sa.allocateObject<Person>();
			Person* pOut3 = sa.allocateObject<Person>();
			Person* pOut4 = sa.allocateObject<Person>();
			Person* pOut5 = sa.allocateObject<Person>();

			pOut1->name="Outer1"; pOut1->adress="Addr1"; pOut1->age=1;
			pOut2->name="Outer2"; pOut2->adress="Addr2"; pOut2->age=2;
			pOut3->name="Outer3"; pOut3->adress="Addr3"; pOut3->age=3;
			pOut4->name="Outer4"; pOut4->adress="Addr4"; pOut4->age=4;
			pOut5->name="Outer5"; pOut5->adress="Addr5"; pOut5->age=5;

			for (int i = 0; i < 512; i++) {
				auto marker = sa.getMarker();
				Person* person1 = sa.allocateObject<Person>();
				Person* person2 = sa.allocateObject<Person>();
				Person* person3 = sa.allocateObject<Person>();
				Person* person4 = sa.allocateObject<Person>();
				sa.allocateObject<float>(50);
				Person* person5 = sa.allocateObject<Person>();
				try {
					Person* person6 = sa.allocateObjects<Person>(1, ForceException());
					person6->age++; //Anti unused warning
				}
				catch (int) {
					//do nothing
				}

				person1->name = "Hugo";
				person2->name = "Ebert";
				person3->name = "Lel";
				person4->name = "Aha?";
				person5->name = "Okay";

				person1->adress = "AStr";
				person2->adress = "BStr";
				person3->adress = "CStr";
				person4->adress = "DStr";
				person5->adress = "EStr";

				person1->age = 1;
				person2->age = 2;
				person3->age = 3;
				person4->age = 4;
				person5->age = 5;

				assertEquals(person1->name, "Hugo");
				assertEquals(person2->name, "Ebert");
				assertEquals(person3->name, "Lel");
				assertEquals(person4->name, "Aha?");
				assertEquals(person5->name, "Okay");

				assertEquals(person1->adress, "AStr");
				assertEquals(person2->adress, "BStr");
				assertEquals(person3->adress, "CStr");
				assertEquals(person4->adress, "DStr");
				assertEquals(person5->adress, "EStr");

				assertEquals(person1->age, 1);
				assertEquals(person2->age, 2);
				assertEquals(person3->age, 3);
				assertEquals(person4->age, 4);
				assertEquals(person5->age, 5);

				sa.deallocateToMarker(marker);
			}

			assertEquals(pOut1->name, "Outer1"); assertEquals(pOut1->adress, "Addr1"); assertEquals(pOut1->age, 1);
			assertEquals(pOut2->name, "Outer2"); assertEquals(pOut2->adress, "Addr2"); assertEquals(pOut2->age, 2);
			assertEquals(pOut3->name, "Outer3"); assertEquals(pOut3->adress, "Addr3"); assertEquals(pOut3->age, 3);
			assertEquals(pOut4->name, "Outer4"); assertEquals(pOut4->adress, "Addr4"); assertEquals(pOut4->age, 4);
			assertEquals(pOut5->name, "Outer5"); assertEquals(pOut5->adress, "Addr5"); assertEquals(pOut5->age, 5);

			

			sa.deallocateToMarker(startMarker);
			Person::checkIfAllPersonsWereDestroyed();

			for (int i = 0; i < 128; i++) {
				Person* inner = sa.allocateObject<Person>();
				assertUnequals(inner, nullptr);
			}
			bool caughtException = false;
			try
			{
				sa.allocateObject<Person>();
			}
			catch (AllocatorOutOfMemoryException aoome)
			{
				caughtException = true;
			}
			assertEquals(caughtException, true);
			sa.deallocateToMarker(startMarker);
			Person::checkIfAllPersonsWereDestroyed();
		}
	}
}
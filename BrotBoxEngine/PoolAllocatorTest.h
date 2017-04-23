#pragma once

#include "PoolAllocator.h"
#include <iostream>
#include <string>
#include "UtilTest.h"

namespace bbe {
	namespace test {
		void testPoolAllocator() {
			bbe::PoolAllocator<Person> personenAllocator(1024);

			Person* persons[1024];
			for (int i = 0; i < 1024; i++) {
				persons[i] = personenAllocator.allocate();
			}
			for (int i = 0; i < 1024; i++) {
				personenAllocator.deallocate(persons[i]);
			}

			Person* person1 = personenAllocator.allocate();
			Person* person2 = personenAllocator.allocate();
			Person* person3 = personenAllocator.allocate();
			Person* person4 = personenAllocator.allocate();
			Person* person5 = personenAllocator.allocate();

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

			personenAllocator.deallocate(person1);
			personenAllocator.deallocate(person2);
			personenAllocator.deallocate(person3);
			personenAllocator.deallocate(person4);
			personenAllocator.deallocate(person5);

			person2 = personenAllocator.allocate();
			person4 = personenAllocator.allocate();
			person1 = personenAllocator.allocate();
			person5 = personenAllocator.allocate();
			person3 = personenAllocator.allocate();

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

			personenAllocator.deallocate(person1);
			personenAllocator.deallocate(person2);
			personenAllocator.deallocate(person3);
			personenAllocator.deallocate(person4);
			personenAllocator.deallocate(person5);

			person3 = personenAllocator.allocate("Lel", "CStr", 3);
			person4 = personenAllocator.allocate("Aha?", "DStr", 4);
			person1 = personenAllocator.allocate("Hugo", "AStr", 1);
			person2 = personenAllocator.allocate("Ebert", "BStr", 2);
			person5 = personenAllocator.allocate("Okay", "EStr", 5);

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

			personenAllocator.deallocate(person1);
			personenAllocator.deallocate(person2);
			personenAllocator.deallocate(person3);
			personenAllocator.deallocate(person4);
			personenAllocator.deallocate(person5);

			Person::checkIfAllPersonsWereDestroyed();

			bbe::PoolAllocator<char> charAllocator(128);
			char* c1 = charAllocator.allocate();
			char* c2 = charAllocator.allocate();
			char* c3 = charAllocator.allocate();
			char* c4 = charAllocator.allocate();
			char* c5 = charAllocator.allocate();

			*c1 = 'a';
			*c2 = 'b';
			*c3 = 'c';
			*c4 = 'd';
			*c5 = 'e';

			assertEquals(*c1, 'a');
			assertEquals(*c2, 'b');
			assertEquals(*c3, 'c');
			assertEquals(*c4, 'd');
			assertEquals(*c5, 'e');

			charAllocator.deallocate(c1);
			charAllocator.deallocate(c2);
			charAllocator.deallocate(c3);
			charAllocator.deallocate(c4);
			charAllocator.deallocate(c5);
		}
	}
}
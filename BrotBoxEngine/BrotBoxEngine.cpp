// BrotBoxEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PoolAllocatorTest.h"
#include "StackAllocatorTest.h"
#include "StringTest.h"
#include "ListTest.h"
#include "OtherTest.h"
#include "UtilTest.h"
#include <vector>
#include <chrono>

int main()
{
	
	bbe::test::testPoolAllocator();
	bbe::test::testStackAllocator();
	bbe::test::testString();
	bbe::test::testList();
	bbe::test::testAllOthers();

	bbe::test::Person::resetTestStatistics();

	std::vector<bbe::test::Person> listEmpty;

	listEmpty.push_back(bbe::test::Person("Peter", "AStr", 18));
	bbe::test::assertEquals(listEmpty.capacity(), 1);
	bbe::test::assertEquals(listEmpty.size(), 1);
	bbe::test::assertEquals(listEmpty[0].name, "Peter");
	bbe::test::assertEquals(listEmpty[0].adress, "AStr");
	bbe::test::assertEquals(listEmpty[0].age, 18);
	bbe::test::assertEquals(listEmpty.empty(), false);

	listEmpty.pop_back();
	bbe::test::assertEquals(listEmpty.capacity(), 1);
	bbe::test::assertEquals(listEmpty.size(), 0);
	bbe::test::assertEquals(listEmpty.empty(), true);

	listEmpty.shrink_to_fit();

	listEmpty.push_back(bbe::test::Person("Petra", "BStr", 19));
	listEmpty.push_back(bbe::test::Person("Hans", "CStr", 20));
	listEmpty.push_back(bbe::test::Person("Eugen", "DStr", 21));

	bbe::test::assertEquals(listEmpty[0].name, "Petra");
	bbe::test::assertEquals(listEmpty[0].adress, "BStr");
	bbe::test::assertEquals(listEmpty[0].age, 19);
	bbe::test::assertEquals(listEmpty[1].name, "Hans");
	bbe::test::assertEquals(listEmpty[1].adress, "CStr");
	bbe::test::assertEquals(listEmpty[1].age, 20);
	bbe::test::assertEquals(listEmpty[2].name, "Eugen");
	bbe::test::assertEquals(listEmpty[2].adress, "DStr");
	bbe::test::assertEquals(listEmpty[2].age, 21);



	listEmpty.push_back(bbe::test::Person("Brunhilde", "EStr", 22));
	bbe::test::assertEquals(listEmpty.capacity(), 4);
	bbe::test::assertEquals(listEmpty.size(), 4);
	bbe::test::assertEquals(listEmpty.empty(), false);
	bbe::test::assertEquals(listEmpty[0].name, "Petra");
	bbe::test::assertEquals(listEmpty[0].adress, "BStr");
	bbe::test::assertEquals(listEmpty[0].age, 19);
	bbe::test::assertEquals(listEmpty[1].name, "Hans");
	bbe::test::assertEquals(listEmpty[1].adress, "CStr");
	bbe::test::assertEquals(listEmpty[1].age, 20);
	bbe::test::assertEquals(listEmpty[2].name, "Eugen");
	bbe::test::assertEquals(listEmpty[2].adress, "DStr");
	bbe::test::assertEquals(listEmpty[2].age, 21);
	bbe::test::assertEquals(listEmpty[3].name, "Brunhilde");
	bbe::test::assertEquals(listEmpty[3].adress, "EStr");
	bbe::test::assertEquals(listEmpty[3].age, 22);

	listEmpty.pop_back();
	bbe::test::assertEquals(listEmpty.capacity(), 4);
	bbe::test::assertEquals(listEmpty.size(), 3);
	bbe::test::assertEquals(listEmpty.empty(), false);
	bbe::test::assertEquals(listEmpty[0].name, "Petra");
	bbe::test::assertEquals(listEmpty[0].adress, "BStr");
	bbe::test::assertEquals(listEmpty[0].age, 19);
	bbe::test::assertEquals(listEmpty[1].name, "Hans");
	bbe::test::assertEquals(listEmpty[1].adress, "CStr");
	bbe::test::assertEquals(listEmpty[1].age, 20);
	bbe::test::assertEquals(listEmpty[2].name, "Eugen");
	bbe::test::assertEquals(listEmpty[2].adress, "DStr");
	bbe::test::assertEquals(listEmpty[2].age, 21);

	listEmpty.push_back(bbe::test::Person("Zebramensch", "FStr", 23));
	bbe::test::assertEquals(listEmpty.capacity(), 4);
	bbe::test::assertEquals(listEmpty.size(), 4);
	bbe::test::assertEquals(listEmpty.empty(), false);
	bbe::test::assertEquals(listEmpty[0].name, "Petra");
	bbe::test::assertEquals(listEmpty[0].adress, "BStr");
	bbe::test::assertEquals(listEmpty[0].age, 19);
	bbe::test::assertEquals(listEmpty[1].name, "Hans");
	bbe::test::assertEquals(listEmpty[1].adress, "CStr");
	bbe::test::assertEquals(listEmpty[1].age, 20);
	bbe::test::assertEquals(listEmpty[2].name, "Eugen");
	bbe::test::assertEquals(listEmpty[2].adress, "DStr");
	bbe::test::assertEquals(listEmpty[2].age, 21);
	bbe::test::assertEquals(listEmpty[3].name, "Zebramensch");
	bbe::test::assertEquals(listEmpty[3].adress, "FStr");
	bbe::test::assertEquals(listEmpty[3].age, 23);

	listEmpty.clear();

	listEmpty.push_back(bbe::test::Person("IDontKnow", "GStr", 24));
	listEmpty.push_back(bbe::test::Person("Jesus", "HStr", 25));
	listEmpty.push_back(bbe::test::Person("Someone", "IStr", 26));
	listEmpty.push_back(bbe::test::Person("Dragon", "JStr", 27));
	listEmpty.push_back(bbe::test::Person("KeyboardWarrior", "KStr", 28));
	bbe::test::assertEquals(listEmpty[0].name, "IDontKnow");
	bbe::test::assertEquals(listEmpty[0].adress, "GStr");
	bbe::test::assertEquals(listEmpty[0].age, 24);
	bbe::test::assertEquals(listEmpty[1].name, "Jesus");
	bbe::test::assertEquals(listEmpty[1].adress, "HStr");
	bbe::test::assertEquals(listEmpty[1].age, 25);
	bbe::test::assertEquals(listEmpty[2].name, "Someone");
	bbe::test::assertEquals(listEmpty[2].adress, "IStr");
	bbe::test::assertEquals(listEmpty[2].age, 26);
	bbe::test::assertEquals(listEmpty[3].name, "Dragon");
	bbe::test::assertEquals(listEmpty[3].adress, "JStr");
	bbe::test::assertEquals(listEmpty[3].age, 27);
	bbe::test::assertEquals(listEmpty[4].name, "KeyboardWarrior");
	bbe::test::assertEquals(listEmpty[4].adress, "KStr");
	bbe::test::assertEquals(listEmpty[4].age, 28);

	listEmpty.shrink_to_fit();
	bbe::test::assertEquals(listEmpty[0].name, "IDontKnow");
	bbe::test::assertEquals(listEmpty[0].adress, "GStr");
	bbe::test::assertEquals(listEmpty[0].age, 24);
	bbe::test::assertEquals(listEmpty[1].name, "Jesus");
	bbe::test::assertEquals(listEmpty[1].adress, "HStr");
	bbe::test::assertEquals(listEmpty[1].age, 25);
	bbe::test::assertEquals(listEmpty[2].name, "Someone");
	bbe::test::assertEquals(listEmpty[2].adress, "IStr");
	bbe::test::assertEquals(listEmpty[2].age, 26);
	bbe::test::assertEquals(listEmpty[3].name, "Dragon");
	bbe::test::assertEquals(listEmpty[3].adress, "JStr");
	bbe::test::assertEquals(listEmpty[3].age, 27);
	bbe::test::assertEquals(listEmpty[4].name, "KeyboardWarrior");
	bbe::test::assertEquals(listEmpty[4].adress, "KStr");
	bbe::test::assertEquals(listEmpty[4].age, 28);


    return 0;
}


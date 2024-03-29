#pragma once

#include "BBE/List.h"
#include "BBE/UtilTest.h"

namespace bbe
{
	namespace test
	{
		void testListUnsorted();
		void testListSorted();

		template<typename T>
		void printList(List<T> l)
		{
			BBELOG("[");
			for (size_t i = 0; i < l.getLength(); i++)
			{
				BBELOG(l[i]);
				if (i != l.getLength() - 1)
				{
					BBELOG(", ");
				}
			}
			BBELOGLN("]");
		}

		void testList()
		{
			testListUnsorted();
		}

		void testListUnsorted()
		{
			{
				Person::resetTestStatistics();

				List<Person> listEmpty;
				assertEquals(listEmpty.getLength(), 0);
				assertEquals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), true);

				assertEquals(0, Person::s_amountOfPersons);
				assertEquals(0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(0, Person::s_amountOfParameterConstructorCalls);
				listEmpty.add(Person("Peter", "AStr", 18));
				
				assertEquals(listEmpty.getLength(), 1);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty[0].name, "Peter");
				assertEquals(listEmpty[0].address, "AStr");
				assertEquals(listEmpty[0].age, 18);
				assertEquals(listEmpty.isEmpty(), false);

				assertEquals(1, Person::s_amountOfPersons);
				assertEquals(0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(1, Person::s_amountOfParameterConstructorCalls);

				listEmpty.popBack();
				assertEquals(listEmpty.getLength(), 0);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), true);

				assertEquals(0, Person::s_amountOfPersons);
				assertEquals(0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(1, Person::s_amountOfParameterConstructorCalls);

				listEmpty.add(Person("Petra", "BStr", 19));
				listEmpty.add(Person("Hans", "CStr", 20));
				listEmpty.add(Person("Eugen", "DStr", 21));
				assertEquals(listEmpty.getLength(), 3);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);

				assertEquals(3, Person::s_amountOfPersons);
				assertEquals(0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(4, Person::s_amountOfParameterConstructorCalls);

				assertEquals(listEmpty[0].name, "Petra");
				assertEquals(listEmpty[0].address, "BStr");
				assertEquals(listEmpty[0].age, 19);
				assertEquals(listEmpty[1].name, "Hans");
				assertEquals(listEmpty[1].address, "CStr");
				assertEquals(listEmpty[1].age, 20);
				assertEquals(listEmpty[2].name, "Eugen");
				assertEquals(listEmpty[2].address, "DStr");
				assertEquals(listEmpty[2].age, 21);

				listEmpty.add(Person("Brunhilde", "EStr", 22));
				assertEquals(listEmpty.getLength(), 4);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "Petra");
				assertEquals(listEmpty[0].address, "BStr");
				assertEquals(listEmpty[0].age, 19);
				assertEquals(listEmpty[1].name, "Hans");
				assertEquals(listEmpty[1].address, "CStr");
				assertEquals(listEmpty[1].age, 20);
				assertEquals(listEmpty[2].name, "Eugen");
				assertEquals(listEmpty[2].address, "DStr");
				assertEquals(listEmpty[2].age, 21);
				assertEquals(listEmpty[3].name, "Brunhilde");
				assertEquals(listEmpty[3].address, "EStr");
				assertEquals(listEmpty[3].age, 22);

				assertEquals( 4, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals( 5, Person::s_amountOfParameterConstructorCalls);

				listEmpty.popBack();
				assertEquals(listEmpty.getLength(), 3);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "Petra");
				assertEquals(listEmpty[0].address, "BStr");
				assertEquals(listEmpty[0].age, 19);
				assertEquals(listEmpty[1].name, "Hans");
				assertEquals(listEmpty[1].address, "CStr");
				assertEquals(listEmpty[1].age, 20);
				assertEquals(listEmpty[2].name, "Eugen");
				assertEquals(listEmpty[2].address, "DStr");
				assertEquals(listEmpty[2].age, 21);

				assertEquals( 3, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals( 5, Person::s_amountOfParameterConstructorCalls);

				listEmpty.add(Person("Zebramensch", "FStr", 23));
				assertEquals(listEmpty.getLength(), 4);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "Petra");
				assertEquals(listEmpty[0].address, "BStr");
				assertEquals(listEmpty[0].age, 19);
				assertEquals(listEmpty[1].name, "Hans");
				assertEquals(listEmpty[1].address, "CStr");
				assertEquals(listEmpty[1].age, 20);
				assertEquals(listEmpty[2].name, "Eugen");
				assertEquals(listEmpty[2].address, "DStr");
				assertEquals(listEmpty[2].age, 21);
				assertEquals(listEmpty[3].name, "Zebramensch");
				assertEquals(listEmpty[3].address, "FStr");
				assertEquals(listEmpty[3].age, 23);

				assertEquals( 4, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals( 6, Person::s_amountOfParameterConstructorCalls);

				listEmpty.clear();
				assertEquals(listEmpty.getLength(), 0);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), true);

				assertEquals( 0, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals( 6, Person::s_amountOfParameterConstructorCalls);

				listEmpty.add(Person("IDontKnow", "GStr", 24));
				listEmpty.add(Person("Jesus", "HStr", 25));
				listEmpty.add(Person("Someone", "IStr", 26));
				listEmpty.add(Person("Dragon", "JStr", 27));
				listEmpty.add(Person("KeyboardWarrior", "KStr", 28));
				assertEquals(listEmpty.getLength(), 5);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].address, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].address, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].address, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].address, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].address, "KStr");
				assertEquals(listEmpty[4].age, 28);

				assertEquals( 5, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(11, Person::s_amountOfParameterConstructorCalls);

				assertEquals(listEmpty.getLength(), 5);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].address, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].address, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].address, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].address, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].address, "KStr");
				assertEquals(listEmpty[4].age, 28);

				assertEquals( 5, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(11, Person::s_amountOfParameterConstructorCalls);

				Person outerPerson("Outlander", "OutStr", 99);
				listEmpty.add(outerPerson);
				assertEquals(listEmpty.getLength(), 6);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].address, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].address, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].address, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].address, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].address, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].address, "OutStr");
				assertEquals(listEmpty[5].age, 99);

				assertEquals(outerPerson.name, "Outlander");
				assertEquals(outerPerson.address, "OutStr");
				assertEquals(outerPerson.age, 99);

				assertEquals( 7, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(12, Person::s_amountOfParameterConstructorCalls);

				outerPerson.name = "newName";
				outerPerson.address = "OtherStreet";
				outerPerson.age = 100;

				assertEquals(listEmpty.getLength(), 6);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].address, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].address, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].address, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].address, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].address, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].address, "OutStr");
				assertEquals(listEmpty[5].age, 99);

				assertEquals(outerPerson.name, "newName");
				assertEquals(outerPerson.address, "OtherStreet");
				assertEquals(outerPerson.age, 100);

				assertEquals( 7, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(12, Person::s_amountOfParameterConstructorCalls);

				listEmpty.add(Person("CopyMan!", "CopyStr!", -7), 3);
				assertEquals(listEmpty.getLength(), 9);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].address, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].address, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].address, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].address, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].address, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].address, "OutStr");
				assertEquals(listEmpty[5].age, 99);
				assertEquals(listEmpty[6].name, "CopyMan!");
				assertEquals(listEmpty[6].address, "CopyStr!");
				assertEquals(listEmpty[6].age, -7);
				assertEquals(listEmpty[7].name, "CopyMan!");
				assertEquals(listEmpty[7].address, "CopyStr!");
				assertEquals(listEmpty[7].age, -7);
				assertEquals(listEmpty[8].name, "CopyMan!");
				assertEquals(listEmpty[8].address, "CopyStr!");
				assertEquals(listEmpty[8].age, -7);

				assertEquals(10, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(13, Person::s_amountOfParameterConstructorCalls);

				listEmpty[7].name = "Changeling";
				listEmpty[7].address = "Wabble";
				listEmpty[7].age = 7;

				assertEquals(listEmpty.getLength(), 9);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].address, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].address, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].address, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].address, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].address, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].address, "OutStr");
				assertEquals(listEmpty[5].age, 99);
				assertEquals(listEmpty[6].name, "CopyMan!");
				assertEquals(listEmpty[6].address, "CopyStr!");
				assertEquals(listEmpty[6].age, -7);
				assertEquals(listEmpty[7].name, "Changeling");
				assertEquals(listEmpty[7].address, "Wabble");
				assertEquals(listEmpty[7].age, 7);
				assertEquals(listEmpty[8].name, "CopyMan!");
				assertEquals(listEmpty[8].address, "CopyStr!");
				assertEquals(listEmpty[8].age, -7);

				assertEquals(10, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(13, Person::s_amountOfParameterConstructorCalls);

				List<Person> otherList;
				otherList.add(Person("Invader #1", "InvasionStr #1", 30));
				otherList.add(Person("Invader #2", "InvasionStr #2", 31));
				otherList.add(Person("Invader #3", "InvasionStr #3", 32));

				assertEquals(13, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(16, Person::s_amountOfParameterConstructorCalls);

				listEmpty += otherList;
				assertEquals(listEmpty.getLength(), 12);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].address, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].address, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].address, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].address, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].address, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].address, "OutStr");
				assertEquals(listEmpty[5].age, 99);
				assertEquals(listEmpty[6].name, "CopyMan!");
				assertEquals(listEmpty[6].address, "CopyStr!");
				assertEquals(listEmpty[6].age, -7);
				assertEquals(listEmpty[7].name, "Changeling");
				assertEquals(listEmpty[7].address, "Wabble");
				assertEquals(listEmpty[7].age, 7);
				assertEquals(listEmpty[8].name, "CopyMan!");
				assertEquals(listEmpty[8].address, "CopyStr!");
				assertEquals(listEmpty[8].age, -7);
				assertEquals(listEmpty[9].name, "Invader #1");
				assertEquals(listEmpty[9].address, "InvasionStr #1");
				assertEquals(listEmpty[9].age, 30);
				assertEquals(listEmpty[10].name, "Invader #2");
				assertEquals(listEmpty[10].address, "InvasionStr #2");
				assertEquals(listEmpty[10].age, 31);
				assertEquals(listEmpty[11].name, "Invader #3");
				assertEquals(listEmpty[11].address, "InvasionStr #3");
				assertEquals(listEmpty[11].age, 32);

				assertEquals(16, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(16, Person::s_amountOfParameterConstructorCalls);

				otherList[0].name = "Hide #1";
				otherList[0].address = "Hide Str #1";
				otherList[0].age = 80;
				otherList[1].name = "Hide #2";
				otherList[1].address = "Hide Str #2";
				otherList[1].age = 81;
				otherList[2].name = "Hide #3";
				otherList[2].address = "Hide Str #3";
				otherList[2].age = 82;

				assertEquals(listEmpty.getLength(), 12);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].address, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].address, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].address, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].address, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].address, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].address, "OutStr");
				assertEquals(listEmpty[5].age, 99);
				assertEquals(listEmpty[6].name, "CopyMan!");
				assertEquals(listEmpty[6].address, "CopyStr!");
				assertEquals(listEmpty[6].age, -7);
				assertEquals(listEmpty[7].name, "Changeling");
				assertEquals(listEmpty[7].address, "Wabble");
				assertEquals(listEmpty[7].age, 7);
				assertEquals(listEmpty[8].name, "CopyMan!");
				assertEquals(listEmpty[8].address, "CopyStr!");
				assertEquals(listEmpty[8].age, -7);
				assertEquals(listEmpty[9].name, "Invader #1");
				assertEquals(listEmpty[9].address, "InvasionStr #1");
				assertEquals(listEmpty[9].age, 30);
				assertEquals(listEmpty[10].name, "Invader #2");
				assertEquals(listEmpty[10].address, "InvasionStr #2");
				assertEquals(listEmpty[10].age, 31);
				assertEquals(listEmpty[11].name, "Invader #3");
				assertEquals(listEmpty[11].address, "InvasionStr #3");
				assertEquals(listEmpty[11].age, 32);

				assertEquals(16, Person::s_amountOfPersons);
				assertEquals( 0, Person::s_amountOfDefaulConstructorCalls);
				assertEquals(16, Person::s_amountOfParameterConstructorCalls);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				bbe::List<Person> copyFrom(4, "PersonCopy", "StreetCopy", 15);
				assertEquals(copyFrom.getLength(), 4);
				assertUnequals(copyFrom.getRaw(), nullptr);
				assertEquals(copyFrom.isEmpty(), false);
				assertEquals(copyFrom[0].name, "PersonCopy");
				assertEquals(copyFrom[0].address, "StreetCopy");
				assertEquals(copyFrom[0].age, 15);
				assertEquals(copyFrom[1].name, "PersonCopy");
				assertEquals(copyFrom[1].address, "StreetCopy");
				assertEquals(copyFrom[1].age, 15);
				assertEquals(copyFrom[2].name, "PersonCopy");
				assertEquals(copyFrom[2].address, "StreetCopy");
				assertEquals(copyFrom[2].age, 15);
				assertEquals(copyFrom[3].name, "PersonCopy");
				assertEquals(copyFrom[3].address, "StreetCopy");
				assertEquals(copyFrom[3].age, 15);

				List<Person> copyTo(copyFrom);
				assertEquals(copyFrom.getLength(), 4);
				assertUnequals(copyFrom.getRaw(), nullptr);
				assertEquals(copyFrom.isEmpty(), false);
				assertEquals(copyFrom[0].name, "PersonCopy");
				assertEquals(copyFrom[0].address, "StreetCopy");
				assertEquals(copyFrom[0].age, 15);
				assertEquals(copyFrom[1].name, "PersonCopy");
				assertEquals(copyFrom[1].address, "StreetCopy");
				assertEquals(copyFrom[1].age, 15);
				assertEquals(copyFrom[2].name, "PersonCopy");
				assertEquals(copyFrom[2].address, "StreetCopy");
				assertEquals(copyFrom[2].age, 15);
				assertEquals(copyFrom[3].name, "PersonCopy");
				assertEquals(copyFrom[3].address, "StreetCopy");
				assertEquals(copyFrom[3].age, 15);
				assertEquals(copyTo.getLength(), 4);
				assertUnequals(copyTo.getRaw(), nullptr);
				assertEquals(copyTo.isEmpty(), false);
				assertEquals(copyTo[0].name, "PersonCopy");
				assertEquals(copyTo[0].address, "StreetCopy");
				assertEquals(copyTo[0].age, 15);
				assertEquals(copyTo[1].name, "PersonCopy");
				assertEquals(copyTo[1].address, "StreetCopy");
				assertEquals(copyTo[1].age, 15);
				assertEquals(copyTo[2].name, "PersonCopy");
				assertEquals(copyTo[2].address, "StreetCopy");
				assertEquals(copyTo[2].age, 15);
				assertEquals(copyTo[3].name, "PersonCopy");
				assertEquals(copyTo[3].address, "StreetCopy");
				assertEquals(copyTo[3].age, 15);

				List<Person> movedTo(std::move(copyFrom));
				assertEquals(copyFrom.getLength(), 0);
				assertEquals(copyFrom.getRaw(), nullptr);
				assertEquals(copyFrom.isEmpty(), true);
				assertEquals(movedTo.getLength(), 4);
				assertUnequals(movedTo.getRaw(), nullptr);
				assertEquals(movedTo.isEmpty(), false);
				assertEquals(movedTo[0].name, "PersonCopy");
				assertEquals(movedTo[0].address, "StreetCopy");
				assertEquals(movedTo[0].age, 15);
				assertEquals(movedTo[1].name, "PersonCopy");
				assertEquals(movedTo[1].address, "StreetCopy");
				assertEquals(movedTo[1].age, 15);
				assertEquals(movedTo[2].name, "PersonCopy");
				assertEquals(movedTo[2].address, "StreetCopy");
				assertEquals(movedTo[2].age, 15);
				assertEquals(movedTo[3].name, "PersonCopy");
				assertEquals(movedTo[3].address, "StreetCopy");
				assertEquals(movedTo[3].age, 15);
				assertEquals(copyTo.getLength(), 4);
				assertUnequals(copyTo.getRaw(), nullptr);
				assertEquals(copyTo.isEmpty(), false);
				assertEquals(copyTo[0].name, "PersonCopy");
				assertEquals(copyTo[0].address, "StreetCopy");
				assertEquals(copyTo[0].age, 15);
				assertEquals(copyTo[1].name, "PersonCopy");
				assertEquals(copyTo[1].address, "StreetCopy");
				assertEquals(copyTo[1].age, 15);
				assertEquals(copyTo[2].name, "PersonCopy");
				assertEquals(copyTo[2].address, "StreetCopy");
				assertEquals(copyTo[2].age, 15);
				assertEquals(copyTo[3].name, "PersonCopy");
				assertEquals(copyTo[3].address, "StreetCopy");
				assertEquals(copyTo[3].age, 15);

				copyFrom = copyTo;
				assertEquals(copyFrom.getLength(), 4);
				assertUnequals(copyFrom.getRaw(), nullptr);
				assertEquals(copyFrom.isEmpty(), false);
				assertEquals(copyFrom[0].name, "PersonCopy");
				assertEquals(copyFrom[0].address, "StreetCopy");
				assertEquals(copyFrom[0].age, 15);
				assertEquals(copyFrom[1].name, "PersonCopy");
				assertEquals(copyFrom[1].address, "StreetCopy");
				assertEquals(copyFrom[1].age, 15);
				assertEquals(copyFrom[2].name, "PersonCopy");
				assertEquals(copyFrom[2].address, "StreetCopy");
				assertEquals(copyFrom[2].age, 15);
				assertEquals(copyFrom[3].name, "PersonCopy");
				assertEquals(copyFrom[3].address, "StreetCopy");
				assertEquals(copyFrom[3].age, 15);
				assertEquals(movedTo.getLength(), 4);
				assertUnequals(movedTo.getRaw(), nullptr);
				assertEquals(movedTo.isEmpty(), false);
				assertEquals(movedTo[0].name, "PersonCopy");
				assertEquals(movedTo[0].address, "StreetCopy");
				assertEquals(movedTo[0].age, 15);
				assertEquals(movedTo[1].name, "PersonCopy");
				assertEquals(movedTo[1].address, "StreetCopy");
				assertEquals(movedTo[1].age, 15);
				assertEquals(movedTo[2].name, "PersonCopy");
				assertEquals(movedTo[2].address, "StreetCopy");
				assertEquals(movedTo[2].age, 15);
				assertEquals(movedTo[3].name, "PersonCopy");
				assertEquals(movedTo[3].address, "StreetCopy");
				assertEquals(movedTo[3].age, 15);
				assertEquals(copyTo.getLength(), 4);
				assertUnequals(copyTo.getRaw(), nullptr);
				assertEquals(copyTo.isEmpty(), false);
				assertEquals(copyTo[0].name, "PersonCopy");
				assertEquals(copyTo[0].address, "StreetCopy");
				assertEquals(copyTo[0].age, 15);
				assertEquals(copyTo[1].name, "PersonCopy");
				assertEquals(copyTo[1].address, "StreetCopy");
				assertEquals(copyTo[1].age, 15);
				assertEquals(copyTo[2].name, "PersonCopy");
				assertEquals(copyTo[2].address, "StreetCopy");
				assertEquals(copyTo[2].age, 15);
				assertEquals(copyTo[3].name, "PersonCopy");
				assertEquals(copyTo[3].address, "StreetCopy");
				assertEquals(copyTo[3].age, 15);

				copyFrom.clear();

				copyFrom = std::move(copyTo);
				assertEquals(copyFrom.getLength(), 4);
				assertUnequals(copyFrom.getRaw(), nullptr);
				assertEquals(copyFrom.isEmpty(), false);
				assertEquals(copyFrom[0].name, "PersonCopy");
				assertEquals(copyFrom[0].address, "StreetCopy");
				assertEquals(copyFrom[0].age, 15);
				assertEquals(copyFrom[1].name, "PersonCopy");
				assertEquals(copyFrom[1].address, "StreetCopy");
				assertEquals(copyFrom[1].age, 15);
				assertEquals(copyFrom[2].name, "PersonCopy");
				assertEquals(copyFrom[2].address, "StreetCopy");
				assertEquals(copyFrom[2].age, 15);
				assertEquals(copyFrom[3].name, "PersonCopy");
				assertEquals(copyFrom[3].address, "StreetCopy");
				assertEquals(copyFrom[3].age, 15);
				assertEquals(movedTo.getLength(), 4);
				assertUnequals(movedTo.getRaw(), nullptr);
				assertEquals(movedTo.isEmpty(), false);
				assertEquals(movedTo[0].name, "PersonCopy");
				assertEquals(movedTo[0].address, "StreetCopy");
				assertEquals(movedTo[0].age, 15);
				assertEquals(movedTo[1].name, "PersonCopy");
				assertEquals(movedTo[1].address, "StreetCopy");
				assertEquals(movedTo[1].age, 15);
				assertEquals(movedTo[2].name, "PersonCopy");
				assertEquals(movedTo[2].address, "StreetCopy");
				assertEquals(movedTo[2].age, 15);
				assertEquals(movedTo[3].name, "PersonCopy");
				assertEquals(movedTo[3].address, "StreetCopy");
				assertEquals(movedTo[3].age, 15);
				assertEquals(copyTo.getLength(), 0);
				assertEquals(copyTo.getRaw(), nullptr);
				assertEquals(copyTo.isEmpty(), true);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				const List<Person> constList(3, "Sumthing", "nother", 1337);
				assertEquals(constList.getLength(), 3);
				assertEquals(constList.isEmpty(), false);
				assertEquals(constList[0].name, "Sumthing");
				assertEquals(constList[0].address, "nother");
				assertEquals(constList[0].age, 1337);
				assertEquals(constList[1].name, "Sumthing");
				assertEquals(constList[1].address, "nother");
				assertEquals(constList[1].age, 1337);
				assertEquals(constList[2].name, "Sumthing");
				assertEquals(constList[2].address, "nother");
				assertEquals(constList[2].age, 1337);
			}

			{
				List<size_t> size_tList;
				size_tList.add(2);
				size_tList.add(2);
				size_tList.add(2);
				for (size_t i = 0; i < 128; i++)
				{
					size_tList.add(i);
				}

				assertEquals(size_tList.getLength(), 131);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				size_t removedVals = size_tList.removeAll(2);
				assertEquals(removedVals, 4);
				assertEquals(size_tList.getLength(), 127);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 0);
				assertEquals(size_tList[1], 1);
				assertEquals(size_tList[2], 3);
				assertEquals(size_tList[3], 4);
				assertEquals(size_tList[4], 5);
				assertEquals(size_tList[5], 6);

				removedVals = size_tList.removeAll(
					[](const size_t& t)
					{
						return t % 2 == 0;
					});
				assertEquals(removedVals, 63);
				assertEquals(size_tList.getLength(), 64);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 1);
				assertEquals(size_tList[1], 3);
				assertEquals(size_tList[2], 5);
				assertEquals(size_tList[3], 7);
				assertEquals(size_tList[4], 9);
				assertEquals(size_tList[5], 11);

				removedVals = size_tList.removeAll(1337);
				assertEquals(removedVals, 0);
				assertEquals(size_tList.getLength(), 64);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 1);
				assertEquals(size_tList[1], 3);
				assertEquals(size_tList[2], 5);
				assertEquals(size_tList[3], 7);
				assertEquals(size_tList[4], 9);
				assertEquals(size_tList[5], 11);

				removedVals = size_tList.removeAll(
					[](const size_t& t)
					{
						return false;
					});
				assertEquals(removedVals, 0);
				assertEquals(size_tList.getLength(), 64);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 1);
				assertEquals(size_tList[1], 3);
				assertEquals(size_tList[2], 5);
				assertEquals(size_tList[3], 7);
				assertEquals(size_tList[4], 9);
				assertEquals(size_tList[5], 11);

				bool didRemove = size_tList.removeSingle(7);
				assertEquals(didRemove, true);
				assertEquals(size_tList.getLength(), 63);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 1);
				assertEquals(size_tList[1], 3);
				assertEquals(size_tList[2], 5);
				assertEquals(size_tList[3], 9);
				assertEquals(size_tList[4], 11);
				assertEquals(size_tList[5], 13);

				didRemove = size_tList.removeSingle(7);
				assertEquals(didRemove, false);
				assertEquals(size_tList.getLength(), 63);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 1);
				assertEquals(size_tList[1], 3);
				assertEquals(size_tList[2], 5);
				assertEquals(size_tList[3], 9);
				assertEquals(size_tList[4], 11);
				assertEquals(size_tList[5], 13);

				didRemove = size_tList.removeSingle(
					[](const size_t& val)
					{
						return val == 13;
					});
				assertEquals(didRemove, true);
				assertEquals(size_tList.getLength(), 62);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 1);
				assertEquals(size_tList[1], 3);
				assertEquals(size_tList[2], 5);
				assertEquals(size_tList[3], 9);
				assertEquals(size_tList[4], 11);
				assertEquals(size_tList[5], 15);

				didRemove = size_tList.removeSingle(
					[](const size_t& val)
					{
						return val == 13;
					});
				assertEquals(didRemove, false);
				assertEquals(size_tList.getLength(), 62);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 1);
				assertEquals(size_tList[1], 3);
				assertEquals(size_tList[2], 5);
				assertEquals(size_tList[3], 9);
				assertEquals(size_tList[4], 11);
				assertEquals(size_tList[5], 15);
			}
			
			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> pushBackAllList;
				Person someVal("Intruder", "Oh my god str", 100);
				pushBackAllList.addAll(
					Person("A", "AStr", 17),
					Person("B", "BStr", 18),
					someVal,
					Person("C", "CStr", 19),
					Person("D", "DStr", 20),
					Person("E", "EStr", 21),
					Person("F", "FStr", 22)
				);

				assertEquals(pushBackAllList.getLength(), 7);
				assertUnequals(pushBackAllList.getRaw(), nullptr);
				assertEquals(pushBackAllList.isEmpty(), false);


				assertEquals(pushBackAllList[0].name, "A");
				assertEquals(pushBackAllList[0].address, "AStr");
				assertEquals(pushBackAllList[0].age, 17);
				assertEquals(pushBackAllList[1].name, "B");
				assertEquals(pushBackAllList[1].address, "BStr");
				assertEquals(pushBackAllList[1].age, 18);
				assertEquals(pushBackAllList[2].name, "Intruder");
				assertEquals(pushBackAllList[2].address, "Oh my god str");
				assertEquals(pushBackAllList[2].age, 100);
				assertEquals(pushBackAllList[3].name, "C");
				assertEquals(pushBackAllList[3].address, "CStr");
				assertEquals(pushBackAllList[3].age, 19);
				assertEquals(pushBackAllList[4].name, "D");
				assertEquals(pushBackAllList[4].address, "DStr");
				assertEquals(pushBackAllList[4].age, 20);
				assertEquals(pushBackAllList[5].name, "E");
				assertEquals(pushBackAllList[5].address, "EStr");
				assertEquals(pushBackAllList[5].age, 21);
				assertEquals(pushBackAllList[6].name, "F");
				assertEquals(pushBackAllList[6].address, "FStr");
				assertEquals(pushBackAllList[6].age, 22);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<int> containsList;
				containsList.addAll(1, 9, 9, 2, 6, 1, 7, 3, 2, 9, 5);
				assertEquals(containsList.contains(2), true);
				assertEquals(containsList.contains(1), true);
				assertEquals(containsList.contains(5), true);
				assertEquals(containsList.contains(7), true);
				assertEquals(containsList.contains(4), false);
				assertEquals(containsList.contains(8), false);
				assertEquals(containsList.contains(10), false);
				assertEquals(containsList.contains([](const int& val) {return val == 2; }), true);
				assertEquals(containsList.contains([](const int& val) {return val == 1; }), true);
				assertEquals(containsList.contains([](const int& val) {return val == 5; }), true);
				assertEquals(containsList.contains([](const int& val) {return val == 7; }), true);
				assertEquals(containsList.contains([](const int& val) {return val == 4; }), false);
				assertEquals(containsList.contains([](const int& val) {return val == 8; }), false);
				assertEquals(containsList.contains([](const int& val) {return val == 10; }), false);

				assertEquals(containsList.containsUnique(2), false);
				assertEquals(containsList.containsUnique(1), false);
				assertEquals(containsList.containsUnique(5), true);
				assertEquals(containsList.containsUnique(7), true);
				assertEquals(containsList.containsUnique(4), false);
				assertEquals(containsList.containsUnique(8), false);
				assertEquals(containsList.containsUnique(10), false);
				assertEquals(containsList.containsUnique([](const int& val) {return val == 2; }), false);
				assertEquals(containsList.containsUnique([](const int& val) {return val == 1; }), false);
				assertEquals(containsList.containsUnique([](const int& val) {return val == 5; }), true);
				assertEquals(containsList.containsUnique([](const int& val) {return val == 7; }), true);
				assertEquals(containsList.containsUnique([](const int& val) {return val == 4; }), false);
				assertEquals(containsList.containsUnique([](const int& val) {return val == 8; }), false);
				assertEquals(containsList.containsUnique([](const int& val) {return val == 10; }), false);

				assertEquals(containsList.containsAmount(2), 2);
				assertEquals(containsList.containsAmount(1), 2);
				assertEquals(containsList.containsAmount(5), 1);
				assertEquals(containsList.containsAmount(7), 1);
				assertEquals(containsList.containsAmount(4), 0);
				assertEquals(containsList.containsAmount(8), 0);
				assertEquals(containsList.containsAmount(10), 0);
				assertEquals(containsList.containsAmount([](const int& val) {return val == 2; }), 2);
				assertEquals(containsList.containsAmount([](const int& val) {return val == 1; }), 2);
				assertEquals(containsList.containsAmount([](const int& val) {return val == 5; }), 1);
				assertEquals(containsList.containsAmount([](const int& val) {return val == 7; }), 1);
				assertEquals(containsList.containsAmount([](const int& val) {return val == 4; }), 0);
				assertEquals(containsList.containsAmount([](const int& val) {return val == 8; }), 0);
				assertEquals(containsList.containsAmount([](const int& val) {return val == 10; }), 0);

				assertEquals(containsList.getLength(), 11);
				assertUnequals(containsList.getRaw(), nullptr);
				assertEquals(containsList.isEmpty(), false);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<int> sortIntList;
				sortIntList.addAll(9, 2, 1, 0, 10, 49, 2);
				sortIntList.sort();

				assertEquals(sortIntList[0], 0);
				assertEquals(sortIntList[1], 1);
				assertEquals(sortIntList[2], 2);
				assertEquals(sortIntList[3], 2);
				assertEquals(sortIntList[4], 9);
				assertEquals(sortIntList[5], 10);
				assertEquals(sortIntList[6], 49);

				assertEquals(sortIntList.getLength(), 7);
				assertUnequals(sortIntList.getRaw(), nullptr);
				assertEquals(sortIntList.isEmpty(), false);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> sortPersonList;
				sortPersonList.addAll(
					Person("9 year", "9 Street", 9),
					Person("2 year", "2 Street", 2),
					Person("1 year", "1 Street", 1),
					Person("0 year", "0 Street", 0),
					Person("10 year", "10 Street", 10),
					Person("49 year", "49 Street", 49),
					Person("2 year", "2 Street", 2)
				);

				sortPersonList.sort([](const Person& a, const Person& b)
				{
					return a.age < b.age;
				});

				assertEquals(sortPersonList[0].name, "0 year");
				assertEquals(sortPersonList[0].address, "0 Street");
				assertEquals(sortPersonList[0].age, 0);
				assertEquals(sortPersonList[1].name, "1 year");
				assertEquals(sortPersonList[1].address, "1 Street");
				assertEquals(sortPersonList[1].age, 1);
				assertEquals(sortPersonList[2].name, "2 year");
				assertEquals(sortPersonList[2].address, "2 Street");
				assertEquals(sortPersonList[2].age, 2);
				assertEquals(sortPersonList[3].name, "2 year");
				assertEquals(sortPersonList[3].address, "2 Street");
				assertEquals(sortPersonList[3].age, 2);
				assertEquals(sortPersonList[4].name, "9 year");
				assertEquals(sortPersonList[4].address, "9 Street");
				assertEquals(sortPersonList[4].age, 9);
				assertEquals(sortPersonList[5].name, "10 year");
				assertEquals(sortPersonList[5].address, "10 Street");
				assertEquals(sortPersonList[5].age, 10);
				assertEquals(sortPersonList[6].name, "49 year");
				assertEquals(sortPersonList[6].address, "49 Street");
				assertEquals(sortPersonList[6].age, 49);

				assertEquals(sortPersonList.getLength(), 7);
				assertUnequals(sortPersonList.getRaw(), nullptr);
				assertEquals(sortPersonList.isEmpty(), false);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> findList;
				findList.addAll(
					Person("9 year", "9 Street", 9),
					Person("2 year", "2 Street", 2),
					Person("1 year", "1 Street", 1),
					Person("0 year", "0 Street", 0),
					Person("10 year", "10 Street", 10),
					Person("49 year", "49 Street", 49),
					Person("2 year", "2 Street", 2)
				);

				Person* find = findList.find(Person("0 year", "0 Street", 0));
				BBELOGLN((long long) find);
				assertUnequals(find, nullptr);
				assertEquals(find->name, "0 year");
				assertEquals(find->address, "0 Street");
				assertEquals(find->age, 0);

				find->name = "Changed name";
				find->address = "Changed adress";
				find->age = -1;
				assertEquals(find->name, "Changed name");
				assertEquals(find->address, "Changed adress");
				assertEquals(find->age, -1);
				assertEquals(findList[3].name, "Changed name");
				assertEquals(findList[3].address, "Changed adress");
				assertEquals(findList[3].age, -1);

				find = findList.find(Person("2 year", "2 Street", 2));
				assertUnequals(find, nullptr);
				assertEquals(find->name, "2 year");
				assertEquals(find->address, "2 Street");
				assertEquals(find->age, 2);

				find->name = "Changed name2";
				find->address = "Changed adress2";
				find->age = -2;
				assertEquals(find->name, "Changed name2");
				assertEquals(find->address, "Changed adress2");
				assertEquals(find->age, -2);
				assertEquals(findList[1].name, "Changed name2");
				assertEquals(findList[1].address, "Changed adress2");
				assertEquals(findList[1].age, -2);

				find = findList.find(Person("Not", "There", 1832));
				assertEquals(find, nullptr);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> findList;
				findList.addAll(
					Person("9 year", "9 Street", 9),
					Person("2 year", "2 Street", 2),
					Person("1 year", "1 Street", 1),
					Person("0 year", "0 Street", 0),
					Person("10 year", "10 Street", 10),
					Person("49 year", "49 Street", 49),
					Person("2 year", "2 Street", 2)
				);

				Person* find = findList.findLast(Person("0 year", "0 Street", 0));
				assertUnequals(find, nullptr);
				assertEquals(find->name, "0 year");
				assertEquals(find->address, "0 Street");
				assertEquals(find->age, 0);

				find->name = "Changed name";
				find->address = "Changed adress";
				find->age = -1;
				assertEquals(find->name, "Changed name");
				assertEquals(find->address, "Changed adress");
				assertEquals(find->age, -1);
				assertEquals(findList[3].name, "Changed name");
				assertEquals(findList[3].address, "Changed adress");
				assertEquals(findList[3].age, -1);

				find = findList.findLast(Person("2 year", "2 Street", 2));
				assertUnequals(find, nullptr);
				assertEquals(find->name, "2 year");
				assertEquals(find->address, "2 Street");
				assertEquals(find->age, 2);

				find->name = "Changed name2";
				find->address = "Changed adress2";
				find->age = -2;
				assertEquals(find->name, "Changed name2");
				assertEquals(find->address, "Changed adress2");
				assertEquals(find->age, -2);
				assertEquals(findList[6].name, "Changed name2");
				assertEquals(findList[6].address, "Changed adress2");
				assertEquals(findList[6].age, -2);

				find = findList.findLast(Person("Not", "There", 1832));
				assertEquals(find, nullptr);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> findList;
				findList.addAll(
					Person("9 year", "9 Street", 9),
					Person("2 year", "2 Street", 2),
					Person("1 year", "1 Street", 1),
					Person("0 year", "0 Street", 0),
					Person("10 year", "10 Street", 10),
					Person("49 year", "49 Street", 49),
					Person("2 year", "2 Street", 2)
				);

				Person* find = findList.find([](const Person& p) {return p.age == 0; });
				assertUnequals(find, nullptr);
				assertEquals(find->name, "0 year");
				assertEquals(find->address, "0 Street");
				assertEquals(find->age, 0);

				find->name = "Changed name";
				find->address = "Changed adress";
				find->age = -1;
				assertEquals(find->name, "Changed name");
				assertEquals(find->address, "Changed adress");
				assertEquals(find->age, -1);
				assertEquals(findList[3].name, "Changed name");
				assertEquals(findList[3].address, "Changed adress");
				assertEquals(findList[3].age, -1);

				find = findList.find([](const Person& p) {return p.age == 2; });
				assertUnequals(find, nullptr);
				assertEquals(find->name, "2 year");
				assertEquals(find->address, "2 Street");
				assertEquals(find->age, 2);

				find->name = "Changed name2";
				find->address = "Changed adress2";
				find->age = -2;
				assertEquals(find->name, "Changed name2");
				assertEquals(find->address, "Changed adress2");
				assertEquals(find->age, -2);
				assertEquals(findList[1].name, "Changed name2");
				assertEquals(findList[1].address, "Changed adress2");
				assertEquals(findList[1].age, -2);

				find = findList.find([](const Person& p) {return p.age == 13483; });
				assertEquals(find, nullptr);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> findList;
				findList.addAll(
					Person("9 year", "9 Street", 9),
					Person("2 year", "2 Street", 2),
					Person("1 year", "1 Street", 1),
					Person("0 year", "0 Street", 0),
					Person("10 year", "10 Street", 10),
					Person("49 year", "49 Street", 49),
					Person("2 year", "2 Street", 2)
				);

				Person* find = findList.findLast([](const Person& p) {return p.age == 0; });
				assertUnequals(find, nullptr);
				assertEquals(find->name, "0 year");
				assertEquals(find->address, "0 Street");
				assertEquals(find->age, 0);

				find->name = "Changed name";
				find->address = "Changed adress";
				find->age = -1;
				assertEquals(find->name, "Changed name");
				assertEquals(find->address, "Changed adress");
				assertEquals(find->age, -1);
				assertEquals(findList[3].name, "Changed name");
				assertEquals(findList[3].address, "Changed adress");
				assertEquals(findList[3].age, -1);

				find = findList.findLast([](const Person& p) {return p.age == 2; });
				assertUnequals(find, nullptr);
				assertEquals(find->name, "2 year");
				assertEquals(find->address, "2 Street");
				assertEquals(find->age, 2);

				find->name = "Changed name2";
				find->address = "Changed adress2";
				find->age = -2;
				assertEquals(find->name, "Changed name2");
				assertEquals(find->address, "Changed adress2");
				assertEquals(find->age, -2);
				assertEquals(findList[6].name, "Changed name2");
				assertEquals(findList[6].address, "Changed adress2");
				assertEquals(findList[6].age, -2);

				find = findList.findLast([](const Person& p) {return p.age == 13483; });
				assertEquals(find, nullptr);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				{
					List<Person> l1;
					List<Person> l2;

					l1.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					assertEquals(l1 == l2, true);
				}

				{
					List<Person> l1;
					List<Person> l2;

					l1.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("10 year", "10 Street", 10),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					assertEquals(l1 == l2, false);
				}

				{
					List<Person> l1;
					List<Person> l2;

					l1.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					assertEquals(l1 == l2, false);
				}

								{
					List<Person> l1;
					List<Person> l2;

					l1.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					assertEquals(l1 == l2, true);
				}

				{
					List<Person> l1;
					List<Person> l2;

					l1.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("10 year", "10 Street", 10),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					assertEquals(l1 == l2, false);
				}

				{
					List<Person> l1;
					List<Person> l2;

					l1.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					assertEquals(l1 == l2, false);
				}

				{
					List<Person> l1;
					List<Person> l2;

					l1.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					assertEquals(l1 != l2, false);
				}

				{
					List<Person> l1;
					List<Person> l2;

					l1.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("10 year", "10 Street", 10),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					assertEquals(l1 != l2, true);
				}

				{
					List<Person> l1;
					List<Person> l2;

					l1.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.addAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					assertEquals(l1 != l2, true);
				}

				Person::checkIfAllPersonsWereDestroyed();
				

				{
					List<Person> list;
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
					list.add(Person("B Name", "B Str", 2));
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
					list.add(Person("B Name", "B Str", 2));
					list.add(Person("C Name", "C Str", 3));
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
					list.add(Person("B Name", "B Str", 2));
					list.add(Person("C Name", "C Str", 3));
					list.add(Person("D Name", "D Str", 4));
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
					list.add(Person("B Name", "B Str", 2));
					list.add(Person("C Name", "C Str", 3));
					list.add(Person("D Name", "D Str", 4));
					list.add(Person("E Name", "E Str", 5));
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
					list.add(Person("B Name", "B Str", 2));
					list.add(Person("C Name", "C Str", 3));
					list.add(Person("D Name", "D Str", 4));
					list.add(Person("E Name", "E Str", 5));
					list.add(Person("F Name", "F Str", 6));
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
					list.add(Person("B Name", "B Str", 2));
					list.add(Person("C Name", "C Str", 3));
					list.add(Person("D Name", "D Str", 4));
					list.add(Person("E Name", "E Str", 5));
					list.add(Person("F Name", "F Str", 6));
					list.add(Person("G Name", "G Str", 7));
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
					list.add(Person("B Name", "B Str", 2));
					list.add(Person("C Name", "C Str", 3));
					list.add(Person("D Name", "D Str", 4));
					list.add(Person("E Name", "E Str", 5));
					list.add(Person("F Name", "F Str", 6));
					list.add(Person("G Name", "G Str", 7));
					list.add(Person("H Name", "H Str", 8));
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
					list.add(Person("B Name", "B Str", 2));
					list.add(Person("C Name", "C Str", 3));
					list.add(Person("D Name", "D Str", 4));
					list.add(Person("E Name", "E Str", 5));
					list.add(Person("F Name", "F Str", 6));
					list.add(Person("G Name", "G Str", 7));
					list.add(Person("H Name", "H Str", 8));
					list.add(Person("I Name", "I Str", 9));
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
					list.add(Person("B Name", "B Str", 2));
					list.add(Person("C Name", "C Str", 3));
					list.add(Person("D Name", "D Str", 4));
					list.add(Person("E Name", "E Str", 5));
					list.add(Person("F Name", "F Str", 6));
					list.add(Person("G Name", "G Str", 7));
					list.add(Person("H Name", "H Str", 8));
					list.add(Person("I Name", "I Str", 9));
					list.add(Person("J Name", "J Str", 10));
				}

				Person::checkIfAllPersonsWereDestroyed();

				{
					List<Person> list;
					list.add(Person("A Name", "A Str", 1));
					list.add(Person("B Name", "B Str", 2));
					list.add(Person("C Name", "C Str", 3));
					list.add(Person("D Name", "D Str", 4));
					list.add(Person("E Name", "E Str", 5));
					list.add(Person("F Name", "F Str", 6));
					list.add(Person("G Name", "G Str", 7));
					list.add(Person("H Name", "H Str", 8));
					list.add(Person("I Name", "I Str", 9));
					list.add(Person("J Name", "J Str", 10));
					list.add(Person("K Name", "K Str", 11));
				}

				Person::checkIfAllPersonsWereDestroyed();

			}
		}
	}
}
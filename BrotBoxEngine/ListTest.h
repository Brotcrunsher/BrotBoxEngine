#pragma once

#include "List.h"
#include "UtilTest.h"

namespace bbe {
	namespace test {
		void testList() {
			{
				Person::resetTestStatistics();

				List<Person> listEmpty;
				assertEquals(listEmpty.getCapacity(), 0);
				assertEquals(listEmpty.getLength(), 0);
				assertEquals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), true);

				assertEquals(0, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(0, Person::amountOfCopyConstructorCalls);
				assertEquals(0, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(0, Person::amountOfParameterConstructorCalls);
				assertEquals(0, Person::amountOfDestructorCalls);
				listEmpty.pushBack(Person("Peter", "AStr", 18));
				
				assertEquals(listEmpty.getCapacity(), 1);
				assertEquals(listEmpty.getLength(), 1);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty[0].name, "Peter");
				assertEquals(listEmpty[0].adress, "AStr");
				assertEquals(listEmpty[0].age, 18);
				assertEquals(listEmpty.isEmpty(), false);

				assertEquals(1, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(0, Person::amountOfCopyConstructorCalls);
				assertEquals(1, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(1, Person::amountOfParameterConstructorCalls);
				assertEquals(1, Person::amountOfDestructorCalls);

				listEmpty.popBack();
				assertEquals(listEmpty.getCapacity(), 1);
				assertEquals(listEmpty.getLength(), 0);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), true);

				assertEquals(0, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(0, Person::amountOfCopyConstructorCalls);
				assertEquals(1, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(1, Person::amountOfParameterConstructorCalls);
				assertEquals(2, Person::amountOfDestructorCalls);
				
				listEmpty.shrink();
				assertEquals(listEmpty.getCapacity(), 0);
				assertEquals(listEmpty.getLength(), 0);
				assertEquals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), true);

				listEmpty.pushBack(Person("Petra", "BStr", 19));
				listEmpty.pushBack(Person("Hans", "CStr", 20));
				listEmpty.pushBack(Person("Eugen", "DStr", 21));
				assertEquals(listEmpty.getCapacity(), 4);
				assertEquals(listEmpty.getLength(), 3);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);

				assertEquals(3, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(0, Person::amountOfCopyConstructorCalls);
				assertEquals(7, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(4, Person::amountOfParameterConstructorCalls);
				assertEquals(8, Person::amountOfDestructorCalls);

				assertEquals(listEmpty[0].name, "Petra");
				assertEquals(listEmpty[0].adress, "BStr");
				assertEquals(listEmpty[0].age, 19);
				assertEquals(listEmpty[1].name, "Hans");
				assertEquals(listEmpty[1].adress, "CStr");
				assertEquals(listEmpty[1].age, 20);
				assertEquals(listEmpty[2].name, "Eugen");
				assertEquals(listEmpty[2].adress, "DStr");
				assertEquals(listEmpty[2].age, 21);

				listEmpty.pushBack(Person("Brunhilde", "EStr", 22));
				assertEquals(listEmpty.getCapacity(), 4);
				assertEquals(listEmpty.getLength(), 4);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "Petra");
				assertEquals(listEmpty[0].adress, "BStr");
				assertEquals(listEmpty[0].age, 19);
				assertEquals(listEmpty[1].name, "Hans");
				assertEquals(listEmpty[1].adress, "CStr");
				assertEquals(listEmpty[1].age, 20);
				assertEquals(listEmpty[2].name, "Eugen");
				assertEquals(listEmpty[2].adress, "DStr");
				assertEquals(listEmpty[2].age, 21);
				assertEquals(listEmpty[3].name, "Brunhilde");
				assertEquals(listEmpty[3].adress, "EStr");
				assertEquals(listEmpty[3].age, 22);

				assertEquals(4, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(0, Person::amountOfCopyConstructorCalls);
				assertEquals(8, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(5, Person::amountOfParameterConstructorCalls);
				assertEquals(9, Person::amountOfDestructorCalls);

				listEmpty.popBack();
				assertEquals(listEmpty.getCapacity(), 4);
				assertEquals(listEmpty.getLength(), 3);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "Petra");
				assertEquals(listEmpty[0].adress, "BStr");
				assertEquals(listEmpty[0].age, 19);
				assertEquals(listEmpty[1].name, "Hans");
				assertEquals(listEmpty[1].adress, "CStr");
				assertEquals(listEmpty[1].age, 20);
				assertEquals(listEmpty[2].name, "Eugen");
				assertEquals(listEmpty[2].adress, "DStr");
				assertEquals(listEmpty[2].age, 21);

				assertEquals(3, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(0, Person::amountOfCopyConstructorCalls);
				assertEquals(8, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(5, Person::amountOfParameterConstructorCalls);
				assertEquals(10, Person::amountOfDestructorCalls);

				listEmpty.pushBack(Person("Zebramensch", "FStr", 23));
				assertEquals(listEmpty.getCapacity(), 4);
				assertEquals(listEmpty.getLength(), 4);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "Petra");
				assertEquals(listEmpty[0].adress, "BStr");
				assertEquals(listEmpty[0].age, 19);
				assertEquals(listEmpty[1].name, "Hans");
				assertEquals(listEmpty[1].adress, "CStr");
				assertEquals(listEmpty[1].age, 20);
				assertEquals(listEmpty[2].name, "Eugen");
				assertEquals(listEmpty[2].adress, "DStr");
				assertEquals(listEmpty[2].age, 21);
				assertEquals(listEmpty[3].name, "Zebramensch");
				assertEquals(listEmpty[3].adress, "FStr");
				assertEquals(listEmpty[3].age, 23);

				assertEquals(4, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(0, Person::amountOfCopyConstructorCalls);
				assertEquals(9, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(6, Person::amountOfParameterConstructorCalls);
				assertEquals(11, Person::amountOfDestructorCalls);

				listEmpty.clear();
				assertEquals(listEmpty.getCapacity(), 4);
				assertEquals(listEmpty.getLength(), 0);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), true);

				assertEquals(0, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(0, Person::amountOfCopyConstructorCalls);
				assertEquals(9, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(6, Person::amountOfParameterConstructorCalls);
				assertEquals(15, Person::amountOfDestructorCalls);

				listEmpty.pushBack(Person("IDontKnow", "GStr", 24));
				listEmpty.pushBack(Person("Jesus", "HStr", 25));
				listEmpty.pushBack(Person("Someone", "IStr", 26));
				listEmpty.pushBack(Person("Dragon", "JStr", 27));
				listEmpty.pushBack(Person("KeyboardWarrior", "KStr", 28));
				assertEquals(listEmpty.getCapacity(), 8);
				assertEquals(listEmpty.getLength(), 5);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].adress, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].adress, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].adress, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].adress, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].adress, "KStr");
				assertEquals(listEmpty[4].age, 28);

				assertEquals(5, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(0, Person::amountOfCopyConstructorCalls);
				assertEquals(18, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(11, Person::amountOfParameterConstructorCalls);
				assertEquals(24, Person::amountOfDestructorCalls);

				listEmpty.shrink();
				assertEquals(listEmpty.getCapacity(), 5);
				assertEquals(listEmpty.getLength(), 5);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].adress, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].adress, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].adress, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].adress, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].adress, "KStr");
				assertEquals(listEmpty[4].age, 28);

				assertEquals(5, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(0, Person::amountOfCopyConstructorCalls);
				assertEquals(23, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(11, Person::amountOfParameterConstructorCalls);
				assertEquals(29, Person::amountOfDestructorCalls);

				Person outerPerson("Outlander", "OutStr", 99);
				listEmpty.pushBack(outerPerson);
				assertEquals(listEmpty.getCapacity(), 10);
				assertEquals(listEmpty.getLength(), 6);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].adress, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].adress, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].adress, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].adress, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].adress, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].adress, "OutStr");
				assertEquals(listEmpty[5].age, 99);

				assertEquals(outerPerson.name, "Outlander");
				assertEquals(outerPerson.adress, "OutStr");
				assertEquals(outerPerson.age, 99);

				assertEquals(7, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(1, Person::amountOfCopyConstructorCalls);
				assertEquals(28, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(12, Person::amountOfParameterConstructorCalls);
				assertEquals(34, Person::amountOfDestructorCalls);

				outerPerson.name = "newName";
				outerPerson.adress = "OtherStreet";
				outerPerson.age = 100;

				assertEquals(listEmpty.getCapacity(), 10);
				assertEquals(listEmpty.getLength(), 6);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].adress, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].adress, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].adress, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].adress, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].adress, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].adress, "OutStr");
				assertEquals(listEmpty[5].age, 99);

				assertEquals(outerPerson.name, "newName");
				assertEquals(outerPerson.adress, "OtherStreet");
				assertEquals(outerPerson.age, 100);

				assertEquals(7, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(1, Person::amountOfCopyConstructorCalls);
				assertEquals(28, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(12, Person::amountOfParameterConstructorCalls);
				assertEquals(34, Person::amountOfDestructorCalls);

				listEmpty.pushBack(Person("CopyMan!", "CopyStr!", -7), 3);
				assertEquals(listEmpty.getCapacity(), 10);
				assertEquals(listEmpty.getLength(), 9);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].adress, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].adress, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].adress, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].adress, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].adress, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].adress, "OutStr");
				assertEquals(listEmpty[5].age, 99);
				assertEquals(listEmpty[6].name, "CopyMan!");
				assertEquals(listEmpty[6].adress, "CopyStr!");
				assertEquals(listEmpty[6].age, -7);
				assertEquals(listEmpty[7].name, "CopyMan!");
				assertEquals(listEmpty[7].adress, "CopyStr!");
				assertEquals(listEmpty[7].age, -7);
				assertEquals(listEmpty[8].name, "CopyMan!");
				assertEquals(listEmpty[8].adress, "CopyStr!");
				assertEquals(listEmpty[8].age, -7);

				assertEquals(10, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(4, Person::amountOfCopyConstructorCalls);
				assertEquals(28, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(13, Person::amountOfParameterConstructorCalls);
				assertEquals(35, Person::amountOfDestructorCalls);

				listEmpty[7].name = "Changeling";
				listEmpty[7].adress = "Wabble";
				listEmpty[7].age = 7;

				assertEquals(listEmpty.getCapacity(), 10);
				assertEquals(listEmpty.getLength(), 9);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].adress, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].adress, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].adress, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].adress, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].adress, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].adress, "OutStr");
				assertEquals(listEmpty[5].age, 99);
				assertEquals(listEmpty[6].name, "CopyMan!");
				assertEquals(listEmpty[6].adress, "CopyStr!");
				assertEquals(listEmpty[6].age, -7);
				assertEquals(listEmpty[7].name, "Changeling");
				assertEquals(listEmpty[7].adress, "Wabble");
				assertEquals(listEmpty[7].age, 7);
				assertEquals(listEmpty[8].name, "CopyMan!");
				assertEquals(listEmpty[8].adress, "CopyStr!");
				assertEquals(listEmpty[8].age, -7);

				assertEquals(10, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(4, Person::amountOfCopyConstructorCalls);
				assertEquals(28, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(13, Person::amountOfParameterConstructorCalls);
				assertEquals(35, Person::amountOfDestructorCalls);

				List<Person> otherList;
				otherList.pushBack(Person("Invader #1", "InvasionStr #1", 30));
				otherList.pushBack(Person("Invader #2", "InvasionStr #2", 31));
				otherList.pushBack(Person("Invader #3", "InvasionStr #3", 32));

				assertEquals(13, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(4, Person::amountOfCopyConstructorCalls);
				assertEquals(34, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(16, Person::amountOfParameterConstructorCalls);
				assertEquals(41, Person::amountOfDestructorCalls);

				listEmpty += otherList;
				assertEquals(listEmpty.getCapacity(), 20);
				assertEquals(listEmpty.getLength(), 12);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].adress, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].adress, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].adress, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].adress, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].adress, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].adress, "OutStr");
				assertEquals(listEmpty[5].age, 99);
				assertEquals(listEmpty[6].name, "CopyMan!");
				assertEquals(listEmpty[6].adress, "CopyStr!");
				assertEquals(listEmpty[6].age, -7);
				assertEquals(listEmpty[7].name, "Changeling");
				assertEquals(listEmpty[7].adress, "Wabble");
				assertEquals(listEmpty[7].age, 7);
				assertEquals(listEmpty[8].name, "CopyMan!");
				assertEquals(listEmpty[8].adress, "CopyStr!");
				assertEquals(listEmpty[8].age, -7);
				assertEquals(listEmpty[9].name, "Invader #1");
				assertEquals(listEmpty[9].adress, "InvasionStr #1");
				assertEquals(listEmpty[9].age, 30);
				assertEquals(listEmpty[10].name, "Invader #2");
				assertEquals(listEmpty[10].adress, "InvasionStr #2");
				assertEquals(listEmpty[10].age, 31);
				assertEquals(listEmpty[11].name, "Invader #3");
				assertEquals(listEmpty[11].adress, "InvasionStr #3");
				assertEquals(listEmpty[11].age, 32);

				assertEquals(16, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(10, Person::amountOfCopyConstructorCalls);
				assertEquals(44, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(16, Person::amountOfParameterConstructorCalls);
				assertEquals(54, Person::amountOfDestructorCalls);

				otherList[0].name = "Hide #1";
				otherList[0].adress = "Hide Str #1";
				otherList[0].age = 80;
				otherList[1].name = "Hide #2";
				otherList[1].adress = "Hide Str #2";
				otherList[1].age = 81;
				otherList[2].name = "Hide #3";
				otherList[2].adress = "Hide Str #3";
				otherList[2].age = 82;

				assertEquals(listEmpty.getCapacity(), 20);
				assertEquals(listEmpty.getLength(), 12);
				assertUnequals(listEmpty.getRaw(), nullptr);
				assertEquals(listEmpty.isEmpty(), false);
				assertEquals(listEmpty[0].name, "IDontKnow");
				assertEquals(listEmpty[0].adress, "GStr");
				assertEquals(listEmpty[0].age, 24);
				assertEquals(listEmpty[1].name, "Jesus");
				assertEquals(listEmpty[1].adress, "HStr");
				assertEquals(listEmpty[1].age, 25);
				assertEquals(listEmpty[2].name, "Someone");
				assertEquals(listEmpty[2].adress, "IStr");
				assertEquals(listEmpty[2].age, 26);
				assertEquals(listEmpty[3].name, "Dragon");
				assertEquals(listEmpty[3].adress, "JStr");
				assertEquals(listEmpty[3].age, 27);
				assertEquals(listEmpty[4].name, "KeyboardWarrior");
				assertEquals(listEmpty[4].adress, "KStr");
				assertEquals(listEmpty[4].age, 28);
				assertEquals(listEmpty[5].name, "Outlander");
				assertEquals(listEmpty[5].adress, "OutStr");
				assertEquals(listEmpty[5].age, 99);
				assertEquals(listEmpty[6].name, "CopyMan!");
				assertEquals(listEmpty[6].adress, "CopyStr!");
				assertEquals(listEmpty[6].age, -7);
				assertEquals(listEmpty[7].name, "Changeling");
				assertEquals(listEmpty[7].adress, "Wabble");
				assertEquals(listEmpty[7].age, 7);
				assertEquals(listEmpty[8].name, "CopyMan!");
				assertEquals(listEmpty[8].adress, "CopyStr!");
				assertEquals(listEmpty[8].age, -7);
				assertEquals(listEmpty[9].name, "Invader #1");
				assertEquals(listEmpty[9].adress, "InvasionStr #1");
				assertEquals(listEmpty[9].age, 30);
				assertEquals(listEmpty[10].name, "Invader #2");
				assertEquals(listEmpty[10].adress, "InvasionStr #2");
				assertEquals(listEmpty[10].age, 31);
				assertEquals(listEmpty[11].name, "Invader #3");
				assertEquals(listEmpty[11].adress, "InvasionStr #3");
				assertEquals(listEmpty[11].age, 32);

				assertEquals(16, Person::amountOfPersons);
				assertEquals(0, Person::amountOfDefaulConstructorCalls);
				assertEquals(10, Person::amountOfCopyConstructorCalls);
				assertEquals(44, Person::amountOfMoveConstructorCalls);
				assertEquals(0, Person::amountOfCopyAssignmentCalls);
				assertEquals(0, Person::amountOfMoveAssignmentCalls);
				assertEquals(16, Person::amountOfParameterConstructorCalls);
				assertEquals(54, Person::amountOfDestructorCalls);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				bbe::List<Person> copyFrom(4, "PersonCopy", "StreetCopy", 15);
				assertEquals(copyFrom.getCapacity(), 4);
				assertEquals(copyFrom.getLength(), 4);
				assertUnequals(copyFrom.getRaw(), nullptr);
				assertEquals(copyFrom.isEmpty(), false);
				assertEquals(copyFrom[0].name, "PersonCopy");
				assertEquals(copyFrom[0].adress, "StreetCopy");
				assertEquals(copyFrom[0].age, 15);
				assertEquals(copyFrom[1].name, "PersonCopy");
				assertEquals(copyFrom[1].adress, "StreetCopy");
				assertEquals(copyFrom[1].age, 15);
				assertEquals(copyFrom[2].name, "PersonCopy");
				assertEquals(copyFrom[2].adress, "StreetCopy");
				assertEquals(copyFrom[2].age, 15);
				assertEquals(copyFrom[3].name, "PersonCopy");
				assertEquals(copyFrom[3].adress, "StreetCopy");
				assertEquals(copyFrom[3].age, 15);

				List<Person> copyTo(copyFrom);
				assertEquals(copyFrom.getCapacity(), 4);
				assertEquals(copyFrom.getLength(), 4);
				assertUnequals(copyFrom.getRaw(), nullptr);
				assertEquals(copyFrom.isEmpty(), false);
				assertEquals(copyFrom[0].name, "PersonCopy");
				assertEquals(copyFrom[0].adress, "StreetCopy");
				assertEquals(copyFrom[0].age, 15);
				assertEquals(copyFrom[1].name, "PersonCopy");
				assertEquals(copyFrom[1].adress, "StreetCopy");
				assertEquals(copyFrom[1].age, 15);
				assertEquals(copyFrom[2].name, "PersonCopy");
				assertEquals(copyFrom[2].adress, "StreetCopy");
				assertEquals(copyFrom[2].age, 15);
				assertEquals(copyFrom[3].name, "PersonCopy");
				assertEquals(copyFrom[3].adress, "StreetCopy");
				assertEquals(copyFrom[3].age, 15);
				assertEquals(copyTo.getCapacity(), 4);
				assertEquals(copyTo.getLength(), 4);
				assertUnequals(copyTo.getRaw(), nullptr);
				assertEquals(copyTo.isEmpty(), false);
				assertEquals(copyTo[0].name, "PersonCopy");
				assertEquals(copyTo[0].adress, "StreetCopy");
				assertEquals(copyTo[0].age, 15);
				assertEquals(copyTo[1].name, "PersonCopy");
				assertEquals(copyTo[1].adress, "StreetCopy");
				assertEquals(copyTo[1].age, 15);
				assertEquals(copyTo[2].name, "PersonCopy");
				assertEquals(copyTo[2].adress, "StreetCopy");
				assertEquals(copyTo[2].age, 15);
				assertEquals(copyTo[3].name, "PersonCopy");
				assertEquals(copyTo[3].adress, "StreetCopy");
				assertEquals(copyTo[3].age, 15);

				List<Person> movedTo(std::move(copyFrom));
				assertEquals(copyFrom.getCapacity(), 0);
				assertEquals(copyFrom.getLength(), 0);
				assertEquals(copyFrom.getRaw(), nullptr);
				assertEquals(copyFrom.isEmpty(), true);
				assertEquals(movedTo.getCapacity(), 4);
				assertEquals(movedTo.getLength(), 4);
				assertUnequals(movedTo.getRaw(), nullptr);
				assertEquals(movedTo.isEmpty(), false);
				assertEquals(movedTo[0].name, "PersonCopy");
				assertEquals(movedTo[0].adress, "StreetCopy");
				assertEquals(movedTo[0].age, 15);
				assertEquals(movedTo[1].name, "PersonCopy");
				assertEquals(movedTo[1].adress, "StreetCopy");
				assertEquals(movedTo[1].age, 15);
				assertEquals(movedTo[2].name, "PersonCopy");
				assertEquals(movedTo[2].adress, "StreetCopy");
				assertEquals(movedTo[2].age, 15);
				assertEquals(movedTo[3].name, "PersonCopy");
				assertEquals(movedTo[3].adress, "StreetCopy");
				assertEquals(movedTo[3].age, 15);
				assertEquals(copyTo.getCapacity(), 4);
				assertEquals(copyTo.getLength(), 4);
				assertUnequals(copyTo.getRaw(), nullptr);
				assertEquals(copyTo.isEmpty(), false);
				assertEquals(copyTo[0].name, "PersonCopy");
				assertEquals(copyTo[0].adress, "StreetCopy");
				assertEquals(copyTo[0].age, 15);
				assertEquals(copyTo[1].name, "PersonCopy");
				assertEquals(copyTo[1].adress, "StreetCopy");
				assertEquals(copyTo[1].age, 15);
				assertEquals(copyTo[2].name, "PersonCopy");
				assertEquals(copyTo[2].adress, "StreetCopy");
				assertEquals(copyTo[2].age, 15);
				assertEquals(copyTo[3].name, "PersonCopy");
				assertEquals(copyTo[3].adress, "StreetCopy");
				assertEquals(copyTo[3].age, 15);

				copyFrom = copyTo;
				assertEquals(copyFrom.getCapacity(), 4);
				assertEquals(copyFrom.getLength(), 4);
				assertUnequals(copyFrom.getRaw(), nullptr);
				assertEquals(copyFrom.isEmpty(), false);
				assertEquals(copyFrom[0].name, "PersonCopy");
				assertEquals(copyFrom[0].adress, "StreetCopy");
				assertEquals(copyFrom[0].age, 15);
				assertEquals(copyFrom[1].name, "PersonCopy");
				assertEquals(copyFrom[1].adress, "StreetCopy");
				assertEquals(copyFrom[1].age, 15);
				assertEquals(copyFrom[2].name, "PersonCopy");
				assertEquals(copyFrom[2].adress, "StreetCopy");
				assertEquals(copyFrom[2].age, 15);
				assertEquals(copyFrom[3].name, "PersonCopy");
				assertEquals(copyFrom[3].adress, "StreetCopy");
				assertEquals(copyFrom[3].age, 15);
				assertEquals(movedTo.getCapacity(), 4);
				assertEquals(movedTo.getLength(), 4);
				assertUnequals(movedTo.getRaw(), nullptr);
				assertEquals(movedTo.isEmpty(), false);
				assertEquals(movedTo[0].name, "PersonCopy");
				assertEquals(movedTo[0].adress, "StreetCopy");
				assertEquals(movedTo[0].age, 15);
				assertEquals(movedTo[1].name, "PersonCopy");
				assertEquals(movedTo[1].adress, "StreetCopy");
				assertEquals(movedTo[1].age, 15);
				assertEquals(movedTo[2].name, "PersonCopy");
				assertEquals(movedTo[2].adress, "StreetCopy");
				assertEquals(movedTo[2].age, 15);
				assertEquals(movedTo[3].name, "PersonCopy");
				assertEquals(movedTo[3].adress, "StreetCopy");
				assertEquals(movedTo[3].age, 15);
				assertEquals(copyTo.getCapacity(), 4);
				assertEquals(copyTo.getLength(), 4);
				assertUnequals(copyTo.getRaw(), nullptr);
				assertEquals(copyTo.isEmpty(), false);
				assertEquals(copyTo[0].name, "PersonCopy");
				assertEquals(copyTo[0].adress, "StreetCopy");
				assertEquals(copyTo[0].age, 15);
				assertEquals(copyTo[1].name, "PersonCopy");
				assertEquals(copyTo[1].adress, "StreetCopy");
				assertEquals(copyTo[1].age, 15);
				assertEquals(copyTo[2].name, "PersonCopy");
				assertEquals(copyTo[2].adress, "StreetCopy");
				assertEquals(copyTo[2].age, 15);
				assertEquals(copyTo[3].name, "PersonCopy");
				assertEquals(copyTo[3].adress, "StreetCopy");
				assertEquals(copyTo[3].age, 15);

				copyFrom.clear();
				copyFrom.shrink();

				copyFrom = std::move(copyTo);
				assertEquals(copyFrom.getCapacity(), 4);
				assertEquals(copyFrom.getLength(), 4);
				assertUnequals(copyFrom.getRaw(), nullptr);
				assertEquals(copyFrom.isEmpty(), false);
				assertEquals(copyFrom[0].name, "PersonCopy");
				assertEquals(copyFrom[0].adress, "StreetCopy");
				assertEquals(copyFrom[0].age, 15);
				assertEquals(copyFrom[1].name, "PersonCopy");
				assertEquals(copyFrom[1].adress, "StreetCopy");
				assertEquals(copyFrom[1].age, 15);
				assertEquals(copyFrom[2].name, "PersonCopy");
				assertEquals(copyFrom[2].adress, "StreetCopy");
				assertEquals(copyFrom[2].age, 15);
				assertEquals(copyFrom[3].name, "PersonCopy");
				assertEquals(copyFrom[3].adress, "StreetCopy");
				assertEquals(copyFrom[3].age, 15);
				assertEquals(movedTo.getCapacity(), 4);
				assertEquals(movedTo.getLength(), 4);
				assertUnequals(movedTo.getRaw(), nullptr);
				assertEquals(movedTo.isEmpty(), false);
				assertEquals(movedTo[0].name, "PersonCopy");
				assertEquals(movedTo[0].adress, "StreetCopy");
				assertEquals(movedTo[0].age, 15);
				assertEquals(movedTo[1].name, "PersonCopy");
				assertEquals(movedTo[1].adress, "StreetCopy");
				assertEquals(movedTo[1].age, 15);
				assertEquals(movedTo[2].name, "PersonCopy");
				assertEquals(movedTo[2].adress, "StreetCopy");
				assertEquals(movedTo[2].age, 15);
				assertEquals(movedTo[3].name, "PersonCopy");
				assertEquals(movedTo[3].adress, "StreetCopy");
				assertEquals(movedTo[3].age, 15);
				assertEquals(copyTo.getCapacity(), 0);
				assertEquals(copyTo.getLength(), 0);
				assertEquals(copyTo.getRaw(), nullptr);
				assertEquals(copyTo.isEmpty(), true);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				const List<Person> constList(3, "Sumthing", "nother", 1337);
				assertEquals(constList.getCapacity(), 3);
				assertEquals(constList.getLength(), 3);
				assertEquals(constList.isEmpty(), false);
				assertEquals(constList[0].name, "Sumthing");
				assertEquals(constList[0].adress, "nother");
				assertEquals(constList[0].age, 1337);
				assertEquals(constList[1].name, "Sumthing");
				assertEquals(constList[1].adress, "nother");
				assertEquals(constList[1].age, 1337);
				assertEquals(constList[2].name, "Sumthing");
				assertEquals(constList[2].adress, "nother");
				assertEquals(constList[2].age, 1337);
			}

			{
				List<size_t> size_tList;
				size_tList.pushBack(2);
				size_tList.pushBack(2);
				size_tList.pushBack(2);
				for (size_t i = 0; i < 128; i++) {
					size_tList.pushBack(i);
				}

				assertEquals(size_tList.getCapacity(), 256);
				assertEquals(size_tList.getLength(), 131);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				size_t removedVals = size_tList.removeAll(2);
				assertEquals(removedVals, 4);
				assertEquals(size_tList.getCapacity(), 256);
				assertEquals(size_tList.getLength(), 127);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 0);
				assertEquals(size_tList[1], 1);
				assertEquals(size_tList[2], 3);
				assertEquals(size_tList[3], 4);
				assertEquals(size_tList[4], 5);
				assertEquals(size_tList[5], 6);

				removedVals = size_tList.removeAll([](const size_t& t) {return t % 2 == 0; });
				assertEquals(removedVals, 63);
				assertEquals(size_tList.getCapacity(), 256);
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
				assertEquals(size_tList.getCapacity(), 256);
				assertEquals(size_tList.getLength(), 64);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 1);
				assertEquals(size_tList[1], 3);
				assertEquals(size_tList[2], 5);
				assertEquals(size_tList[3], 7);
				assertEquals(size_tList[4], 9);
				assertEquals(size_tList[5], 11);

				removedVals = size_tList.removeAll([](const size_t& t) {return false; });
				assertEquals(removedVals, 0);
				assertEquals(size_tList.getCapacity(), 256);
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
				assertEquals(size_tList.getCapacity(), 256);
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
				assertEquals(size_tList.getCapacity(), 256);
				assertEquals(size_tList.getLength(), 63);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 1);
				assertEquals(size_tList[1], 3);
				assertEquals(size_tList[2], 5);
				assertEquals(size_tList[3], 9);
				assertEquals(size_tList[4], 11);
				assertEquals(size_tList[5], 13);

				didRemove = size_tList.removeSingle([](const size_t& val) {return val == 13; });
				assertEquals(didRemove, true);
				assertEquals(size_tList.getCapacity(), 256);
				assertEquals(size_tList.getLength(), 62);
				assertUnequals(size_tList.getRaw(), nullptr);
				assertEquals(size_tList.isEmpty(), false);

				assertEquals(size_tList[0], 1);
				assertEquals(size_tList[1], 3);
				assertEquals(size_tList[2], 5);
				assertEquals(size_tList[3], 9);
				assertEquals(size_tList[4], 11);
				assertEquals(size_tList[5], 15);

				didRemove = size_tList.removeSingle([](const size_t& val) {return val == 13; });
				assertEquals(didRemove, false);
				assertEquals(size_tList.getCapacity(), 256);
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
				pushBackAllList.pushBackAll(
					Person("A", "AStr", 17),
					Person("B", "BStr", 18),
					someVal,
					Person("C", "CStr", 19),
					Person("D", "DStr", 20),
					Person("E", "EStr", 21),
					Person("F", "FStr", 22)
				);

				assertEquals(pushBackAllList.getCapacity(), 8);
				assertEquals(pushBackAllList.getLength(), 7);
				assertUnequals(pushBackAllList.getRaw(), nullptr);
				assertEquals(pushBackAllList.isEmpty(), false);


				assertEquals(pushBackAllList[0].name, "A");
				assertEquals(pushBackAllList[0].adress, "AStr");
				assertEquals(pushBackAllList[0].age, 17);
				assertEquals(pushBackAllList[1].name, "B");
				assertEquals(pushBackAllList[1].adress, "BStr");
				assertEquals(pushBackAllList[1].age, 18);
				assertEquals(pushBackAllList[2].name, "Intruder");
				assertEquals(pushBackAllList[2].adress, "Oh my god str");
				assertEquals(pushBackAllList[2].age, 100);
				assertEquals(pushBackAllList[3].name, "C");
				assertEquals(pushBackAllList[3].adress, "CStr");
				assertEquals(pushBackAllList[3].age, 19);
				assertEquals(pushBackAllList[4].name, "D");
				assertEquals(pushBackAllList[4].adress, "DStr");
				assertEquals(pushBackAllList[4].age, 20);
				assertEquals(pushBackAllList[5].name, "E");
				assertEquals(pushBackAllList[5].adress, "EStr");
				assertEquals(pushBackAllList[5].age, 21);
				assertEquals(pushBackAllList[6].name, "F");
				assertEquals(pushBackAllList[6].adress, "FStr");
				assertEquals(pushBackAllList[6].age, 22);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<int> containsList;
				containsList.pushBackAll(1, 9, 9, 2, 6, 1, 7, 3, 2, 9, 5);
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

				assertEquals(containsList.getCapacity(), 16);
				assertEquals(containsList.getLength(), 11);
				assertUnequals(containsList.getRaw(), nullptr);
				assertEquals(containsList.isEmpty(), false);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<int> sortIntList;
				sortIntList.pushBackAll(9, 2, 1, 0, 10, 49, 2);
				sortIntList.sort();

				assertEquals(sortIntList[0], 0);
				assertEquals(sortIntList[1], 1);
				assertEquals(sortIntList[2], 2);
				assertEquals(sortIntList[3], 2);
				assertEquals(sortIntList[4], 9);
				assertEquals(sortIntList[5], 10);
				assertEquals(sortIntList[6], 49);

				assertEquals(sortIntList.getCapacity(), 8);
				assertEquals(sortIntList.getLength(), 7);
				assertUnequals(sortIntList.getRaw(), nullptr);
				assertEquals(sortIntList.isEmpty(), false);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> sortPersonList;
				sortPersonList.pushBackAll(
					Person("9 year", "9 Street", 9),
					Person("2 year", "2 Street", 2),
					Person("1 year", "1 Street", 1),
					Person("0 year", "0 Street", 0),
					Person("10 year", "10 Street", 10),
					Person("49 year", "49 Street", 49),
					Person("2 year", "2 Street", 2)
				);

				sortPersonList.sort([](const Person& a, const Person& b) {
					return a.age < b.age;
				});

				assertEquals(sortPersonList[0].name, "0 year");
				assertEquals(sortPersonList[0].adress, "0 Street");
				assertEquals(sortPersonList[0].age, 0);
				assertEquals(sortPersonList[1].name, "1 year");
				assertEquals(sortPersonList[1].adress, "1 Street");
				assertEquals(sortPersonList[1].age, 1);
				assertEquals(sortPersonList[2].name, "2 year");
				assertEquals(sortPersonList[2].adress, "2 Street");
				assertEquals(sortPersonList[2].age, 2);
				assertEquals(sortPersonList[3].name, "2 year");
				assertEquals(sortPersonList[3].adress, "2 Street");
				assertEquals(sortPersonList[3].age, 2);
				assertEquals(sortPersonList[4].name, "9 year");
				assertEquals(sortPersonList[4].adress, "9 Street");
				assertEquals(sortPersonList[4].age, 9);
				assertEquals(sortPersonList[5].name, "10 year");
				assertEquals(sortPersonList[5].adress, "10 Street");
				assertEquals(sortPersonList[5].age, 10);
				assertEquals(sortPersonList[6].name, "49 year");
				assertEquals(sortPersonList[6].adress, "49 Street");
				assertEquals(sortPersonList[6].age, 49);


				assertEquals(sortPersonList.getCapacity(), 8);
				assertEquals(sortPersonList.getLength(), 7);
				assertUnequals(sortPersonList.getRaw(), nullptr);
				assertEquals(sortPersonList.isEmpty(), false);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> findList;
				findList.pushBackAll(
					Person("9 year", "9 Street", 9),
					Person("2 year", "2 Street", 2),
					Person("1 year", "1 Street", 1),
					Person("0 year", "0 Street", 0),
					Person("10 year", "10 Street", 10),
					Person("49 year", "49 Street", 49),
					Person("2 year", "2 Street", 2)
				);

				Person* find = findList.find(Person("0 year", "0 Street", 0));
				assertUnequals(find, nullptr);
				assertEquals(find->name, "0 year");
				assertEquals(find->adress, "0 Street");
				assertEquals(find->age, 0);

				find->name = "Changed name";
				find->adress = "Changed adress";
				find->age = -1;
				assertEquals(find->name, "Changed name");
				assertEquals(find->adress, "Changed adress");
				assertEquals(find->age, -1);
				assertEquals(findList[3].name, "Changed name");
				assertEquals(findList[3].adress, "Changed adress");
				assertEquals(findList[3].age, -1);

				find = findList.find(Person("2 year", "2 Street", 2));
				assertUnequals(find, nullptr);
				assertEquals(find->name, "2 year");
				assertEquals(find->adress, "2 Street");
				assertEquals(find->age, 2);

				find->name = "Changed name2";
				find->adress = "Changed adress2";
				find->age = -2;
				assertEquals(find->name, "Changed name2");
				assertEquals(find->adress, "Changed adress2");
				assertEquals(find->age, -2);
				assertEquals(findList[1].name, "Changed name2");
				assertEquals(findList[1].adress, "Changed adress2");
				assertEquals(findList[1].age, -2);

				find = findList.find(Person("Not", "There", 1832));
				assertEquals(find, nullptr);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> findList;
				findList.pushBackAll(
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
				assertEquals(find->adress, "0 Street");
				assertEquals(find->age, 0);

				find->name = "Changed name";
				find->adress = "Changed adress";
				find->age = -1;
				assertEquals(find->name, "Changed name");
				assertEquals(find->adress, "Changed adress");
				assertEquals(find->age, -1);
				assertEquals(findList[3].name, "Changed name");
				assertEquals(findList[3].adress, "Changed adress");
				assertEquals(findList[3].age, -1);

				find = findList.findLast(Person("2 year", "2 Street", 2));
				assertUnequals(find, nullptr);
				assertEquals(find->name, "2 year");
				assertEquals(find->adress, "2 Street");
				assertEquals(find->age, 2);

				find->name = "Changed name2";
				find->adress = "Changed adress2";
				find->age = -2;
				assertEquals(find->name, "Changed name2");
				assertEquals(find->adress, "Changed adress2");
				assertEquals(find->age, -2);
				assertEquals(findList[6].name, "Changed name2");
				assertEquals(findList[6].adress, "Changed adress2");
				assertEquals(findList[6].age, -2);

				find = findList.findLast(Person("Not", "There", 1832));
				assertEquals(find, nullptr);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> findList;
				findList.pushBackAll(
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
				assertEquals(find->adress, "0 Street");
				assertEquals(find->age, 0);

				find->name = "Changed name";
				find->adress = "Changed adress";
				find->age = -1;
				assertEquals(find->name, "Changed name");
				assertEquals(find->adress, "Changed adress");
				assertEquals(find->age, -1);
				assertEquals(findList[3].name, "Changed name");
				assertEquals(findList[3].adress, "Changed adress");
				assertEquals(findList[3].age, -1);

				find = findList.find([](const Person& p) {return p.age == 2; });
				assertUnequals(find, nullptr);
				assertEquals(find->name, "2 year");
				assertEquals(find->adress, "2 Street");
				assertEquals(find->age, 2);

				find->name = "Changed name2";
				find->adress = "Changed adress2";
				find->age = -2;
				assertEquals(find->name, "Changed name2");
				assertEquals(find->adress, "Changed adress2");
				assertEquals(find->age, -2);
				assertEquals(findList[1].name, "Changed name2");
				assertEquals(findList[1].adress, "Changed adress2");
				assertEquals(findList[1].age, -2);

				find = findList.find([](const Person& p) {return p.age == 13483; });
				assertEquals(find, nullptr);
			}

			Person::checkIfAllPersonsWereDestroyed();
			Person::resetTestStatistics();

			{
				List<Person> findList;
				findList.pushBackAll(
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
				assertEquals(find->adress, "0 Street");
				assertEquals(find->age, 0);

				find->name = "Changed name";
				find->adress = "Changed adress";
				find->age = -1;
				assertEquals(find->name, "Changed name");
				assertEquals(find->adress, "Changed adress");
				assertEquals(find->age, -1);
				assertEquals(findList[3].name, "Changed name");
				assertEquals(findList[3].adress, "Changed adress");
				assertEquals(findList[3].age, -1);

				find = findList.findLast([](const Person& p) {return p.age == 2; });
				assertUnequals(find, nullptr);
				assertEquals(find->name, "2 year");
				assertEquals(find->adress, "2 Street");
				assertEquals(find->age, 2);

				find->name = "Changed name2";
				find->adress = "Changed adress2";
				find->age = -2;
				assertEquals(find->name, "Changed name2");
				assertEquals(find->adress, "Changed adress2");
				assertEquals(find->age, -2);
				assertEquals(findList[6].name, "Changed name2");
				assertEquals(findList[6].adress, "Changed adress2");
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

					l1.pushBackAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.pushBackAll(
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

					l1.pushBackAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("10 year", "10 Street", 10),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.pushBackAll(
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

					l1.pushBackAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.pushBackAll(
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

					l1.pushBackAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.pushBackAll(
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

					l1.pushBackAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("10 year", "10 Street", 10),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.pushBackAll(
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

					l1.pushBackAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.pushBackAll(
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

					l1.pushBackAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("10 year", "10 Street", 10),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.pushBackAll(
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

					l1.pushBackAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("10 year", "10 Street", 10),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.pushBackAll(
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

					l1.pushBackAll(
						Person("9 year", "9 Street", 9),
						Person("2 year", "2 Street", 2),
						Person("1 year", "1 Street", 1),
						Person("0 year", "0 Street", 0),
						Person("49 year", "49 Street", 49),
						Person("2 year", "2 Street", 2)
					);

					l2.pushBackAll(
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


				
			}
		}
	}
}
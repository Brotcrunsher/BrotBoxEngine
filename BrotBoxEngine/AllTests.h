#pragma once


#include "PoolAllocatorTest.h"
#include "StackAllocatorTest.h"
#include "GeneralPurposeAllocatorTest.h"
#include "StringTest.h"
#include "ListTest.h"
#include "HashMapTest.h"
#include "OtherTest.h"
#include "UtilTest.h"
#include "UniquePointerTest.h"

namespace bbe {
	namespace test {
		void runAllTests() {
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testPoolAllocator();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testStackAllocator();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testGeneralPurposeAllocator();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testString();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testList();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testHashMap();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testAllOthers();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testUniquePointer();
			Person::checkIfAllPersonsWereDestroyed();
		}
	}
}
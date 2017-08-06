#pragma once


#include "../BBE/PoolAllocatorTest.h"
#include "../BBE/StackAllocatorTest.h"
#include "../BBE/GeneralPurposeAllocatorTest.h"
#include "../BBE/DefragmentationAllocatorTest.h"
#include "../BBE/StringTest.h"
#include "../BBE/ListTest.h"
#include "../BBE/HashMapTest.h"
#include "../BBE/StackTest.h"
#include "../BBE/OtherTest.h"
#include "../BBE/UtilTest.h"
#include "../BBE/UniquePointerTest.h"

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
			bbe::test::testDefragmentationAllocator();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testString();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testList();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testHashMap();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testStack();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testAllOthers();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testUniquePointer();
			Person::checkIfAllPersonsWereDestroyed();
		}
	}
}
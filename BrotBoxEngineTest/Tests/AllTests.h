#pragma once


#include "PoolAllocatorTest.h"
#include "StackAllocatorTest.h"
#include "GeneralPurposeAllocatorTest.h"
#include "DefragmentationAllocatorTest.h"
#include "StringTest.h"
#include "DataStructures/ListTest.h"
#include "DataStructures/HashMapTest.h"
#include "DataStructures/StackTest.h"
#include "DataStructures\ArrayTest.h"
#include "DataStructures\DynamicArrayTest.h"
#include "BBE/UtilTest.h"
#include "UniquePointerTest.h"
#include "Matrix4Test.h"
#include "MathTest.h"

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
			bbe::test::testUniquePointer();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testArray();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testDynamicArray();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testMatrix4();
			Person::checkIfAllPersonsWereDestroyed();
			bbe::test::testMath();
			Person::checkIfAllPersonsWereDestroyed();
		}
	}
}
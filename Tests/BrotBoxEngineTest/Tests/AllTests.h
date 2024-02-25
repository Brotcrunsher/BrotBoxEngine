#pragma once

#include "BBE/Logging.h"
#include "DataStructures/ListTest.h"
#include "DataStructures/HashMapTest.h"
#include "DataStructures/StackTest.h"
#include "DataStructures/ArrayTest.h"
#include "DataStructures/DynamicArrayTest.h"
#include "BBE/UtilTest.h"
#include "UniquePointerTest.h"
#include "Matrix4Test.h"
#include "MathTest.h"
#include "Vector2Test.h"
#include "Vector3Test.h"
#include "LinearCongruentialGeneratorTest.h"
#include "ImageTest.h"

namespace bbe {
	namespace test {
		void runAllTests() {
			BBELOGLN("Starting Tests!");

			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing List");
			bbe::test::testList();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing HashMap");
			bbe::test::testHashMap();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing Stack");
			bbe::test::testStack();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing UniquePointer");
			bbe::test::testUniquePointer();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing Array");
			bbe::test::testArray();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing DynamicArray");
			bbe::test::testDynamicArray();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing testMatrix4");
			bbe::test::testMatrix4();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing testMath");
			bbe::test::testMath();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing Vector2");
			bbe::test::testVector2();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing Vector3");
			bbe::test::testVector3();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing LinearCongruentialGenerator");
			bbe::test::testLinearCongruentailGenerators();
			Person::checkIfAllPersonsWereDestroyed();

			BBELOGLN("Testing Image");
			bbe::test::testImage();
			Person::checkIfAllPersonsWereDestroyed();


			BBELOGLN("All Tests complete!");
		}
	}
}
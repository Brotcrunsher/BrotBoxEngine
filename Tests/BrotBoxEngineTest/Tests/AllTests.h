#pragma once


#include "PoolAllocatorTest.h"
#include "StackAllocatorTest.h"
#include "GeneralPurposeAllocatorTest.h"
#include "DefragmentationAllocatorTest.h"
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
			std::cout << "Starting Tests!" << std::endl;

			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing PoolAllocator" << std::endl;
			bbe::test::testPoolAllocator();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing StackAllocator" << std::endl;
			bbe::test::testStackAllocator();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing GeneralPurposeAllocator" << std::endl;
			bbe::test::testGeneralPurposeAllocator();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing DefragmentationAllocaotr" << std::endl;
			bbe::test::testDefragmentationAllocator();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing List" << std::endl;
			bbe::test::testList();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing HashMap" << std::endl;
			bbe::test::testHashMap();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing Stack" << std::endl;
			bbe::test::testStack();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing UniquePointer" << std::endl;
			bbe::test::testUniquePointer();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing Array" << std::endl;
			bbe::test::testArray();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing DynamicArray" << std::endl;
			bbe::test::testDynamicArray();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing testMatrix4" << std::endl;
			bbe::test::testMatrix4();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing testMath" << std::endl;
			bbe::test::testMath();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing Vector2" << std::endl;
			bbe::test::testVector2();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing Vector3" << std::endl;
			bbe::test::testVector3();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing LinearCongruentialGenerator" << std::endl;
			bbe::test::testLinearCongruentailGenerators();
			Person::checkIfAllPersonsWereDestroyed();

			std::cout << "Testing Image" << std::endl;
			bbe::test::testImage();
			Person::checkIfAllPersonsWereDestroyed();


			std::cout << "All Tests complete!" << std::endl;
		}
	}
}
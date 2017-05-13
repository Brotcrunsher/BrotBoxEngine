// BrotBoxEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "AllTests.h"
#include "PoolAllocatorPerformanceTime.h"
#include "StringPerformanceTime.h"
#include "List.h"
#include "UniquePointer.h"

int main()
{

	for (int i = 0; i < 11; i++) {
		int checkVal = i;
		bbe::List<int, true> list;
		list.pushBackAll(1, 2, 3, 4, 5, 6, 7, 7, 8);
		list.removeSingle(checkVal);
		//std::cout << checkVal << " " << list.getIndexWhenPushedBack(checkVal) << std::endl;
		int* left;
		int* right;
		list.getNeighbors(checkVal, left, right);
		std::cout << checkVal << " ";
		if (left == nullptr) {
			std::cout << "nullptr" << " " << *right;
		}
		else if (right == nullptr) {
			std::cout << *left << " " << "nullptr";
		}
		else {
			std::cout << *left << " " << *right;
		}
		std::cout << std::endl;
	}

	bbe::test::Person::checkIfAllPersonsWereDestroyed();
	{
		bbe::UniquePointer<bbe::test::Person> up(new bbe::test::Person());
	}
	bbe::test::Person::checkIfAllPersonsWereDestroyed();
	

	bbe::test::runAllTests();
	//bbe::test::poolAllocatorPrintAllocationSpeed();
	//bbe::test::stringSpeed();

    return 0;
}


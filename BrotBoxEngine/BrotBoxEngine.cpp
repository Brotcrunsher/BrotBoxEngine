// BrotBoxEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "AllTests.h"
#include "PoolAllocatorPerformanceTime.h"
#include "GeneralPurposeAllocatorPerformanceTime.h"
#include "StringPerformanceTime.h"
#include "HashMapPerformanceTime.h"
#include "List.h"
#include "UniquePointer.h"
#include "Window.h"
#include "Random.h"
#include "UtilTest.h"
#include "Unconstructed.h"
#include "GeneralPurposeAllocator.h"
#include "LinearCongruentialGenerator.h"


int main()
{
	bbe::LCG32 lcg;
	uint32_t smallest = std::numeric_limits<uint32_t>::max();
	uint32_t biggest = 0;

	uint32_t firstSeed = 10;
	uint64_t secondSeed = 1000;


	while (true)
	{
		uint32_t num = lcg.next();
		if (num < smallest)
		{
			std::cout << "SMALLEST : " << num << std::endl;
			smallest = num;
		}
		if (num > biggest)
		{
			std::cout << "BIGGEST : " << num << std::endl;
			biggest = num;
		}
	}

	//bbe::test::NewDeleteAllocationDeallocationSpeed();
	//bbe::test::GeneralPurposeAllocatorAllocationDeallocationSpeed();
	//bbe::test::DefragmentationAllocatorAllocationDeallocationSpeed();

	//bbe::test::runAllTests();
	//bbe::test::poolAllocatorPrintAllocationSpeed();
	//bbe::test::stringSpeed();
	//bbe::test::GeneralPurposeAllocatorAllocationDeallocationSpeed();
	//bbe::test::NewDeleteAllocationDeallocationSpeed();
    
	return 0;
}


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

int main()
{


	//bbe::test::NewDeleteAllocationDeallocationSpeed();
	//bbe::test::GeneralPurposeAllocatorAllocationDeallocationSpeed();
	//bbe::test::DefragmentationAllocatorAllocationDeallocationSpeed();

	bbe::test::runAllTests();
	//bbe::test::poolAllocatorPrintAllocationSpeed();
	//bbe::test::stringSpeed();
	//bbe::test::GeneralPurposeAllocatorAllocationDeallocationSpeed();
	//bbe::test::NewDeleteAllocationDeallocationSpeed();
    
	return 0;
}


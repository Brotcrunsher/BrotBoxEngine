// BrotBoxEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "AllTests.h"
#include "PoolAllocatorPerformanceTime.h"

int main()
{
	bbe::test::runAllTests();
	bbe::test::poolAllocatorPrintAllocationSpeed();

    return 0;
}


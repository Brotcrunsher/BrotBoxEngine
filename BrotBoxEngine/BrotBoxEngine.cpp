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
#include "MersenneTwister.h"
#include "UtilTest.h"
#include "Unconstructed.h"
#include "GeneralPurposeAllocator.h"
#include "LinearCongruentialGenerator.h"
#include "WindowTest.h"


int main()
{
	bbe::test::testWindow();
	return 0;
}


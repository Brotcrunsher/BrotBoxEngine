// BrotBoxEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PoolAllocatorTest.h"
#include "StackAllocatorTest.h"
#include "StringTest.h"
#include "OtherTest.h"
#include <chrono>

int main()
{
	bbe::test::testPoolAllocator();
	bbe::test::testStackAllocator();
	bbe::test::testString();
	bbe::test::testAllOthers();


    return 0;
}


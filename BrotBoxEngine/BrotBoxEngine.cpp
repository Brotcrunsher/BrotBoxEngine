// BrotBoxEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PoolAllocatorTest.h"
#include "StackAllocatorTest.h"
#include "OtherTest.h"

int main()
{
	bbe::test::testPoolAllocator();
	bbe::test::testStackAllocator();
	bbe::test::testAllOthers();
    return 0;
}


// BrotBoxEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PoolAllocatorTest.h"
#include "StackAllocatorTest.h"
#include "StringTest.h"
#include "ListTest.h"
#include "OtherTest.h"
#include "UtilTest.h"
#include <vector>
#include <chrono>

int main()
{
	bbe::test::testPoolAllocator();
	bbe::test::testStackAllocator();
	bbe::test::testString();
	bbe::test::testList();
	bbe::test::testAllOthers();

    return 0;
}


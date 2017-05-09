#pragma once


#include "PoolAllocatorTest.h"
#include "StackAllocatorTest.h"
#include "StringTest.h"
#include "ListTest.h"
#include "OtherTest.h"
#include "UtilTest.h"

namespace bbe {
	namespace test {
		void runAllTests() {
			bbe::test::testPoolAllocator();
			bbe::test::testStackAllocator();
			bbe::test::testString();
			bbe::test::testList();
			bbe::test::testAllOthers();
		}
	}
}
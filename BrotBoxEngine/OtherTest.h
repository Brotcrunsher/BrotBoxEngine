#pragma once

#include "UtilMath.h"
#include "UtilTest.h"

namespace bbe {
	namespace test {
		void _testMath() {
			assertEquals(nextMultiple(16, 31), 32);
			assertEquals(nextMultiple(16, 32), 32);
			assertEquals(nextMultiple(16, 33), 48);
		}

		void testAllOthers() {
			_testMath();
		}
	}
}
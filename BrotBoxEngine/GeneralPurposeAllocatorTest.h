#pragma once

#include "GeneralPurposeAllocator.h"
#include "UtilDebug.h"

namespace bbe {
	namespace test {
		void testGeneralPurposeAllocator() {
			GeneralPurposeAllocator gpa(10000);

			float* f1 = gpa.allocateObjects<float>(20);
			float* f2 = gpa.allocateObjects<float>(50);


			for (int i = 0; i < 20; i++) {
				f1[i] = i + 2;
			}
			for (int i = 0; i < 50; i++) {
				f2[i] = i + 200;
			}


			for (int i = 0; i < 20; i++) {
				assertEquals(f1[i], i + 2);
			}
			for (int i = 0; i < 50; i++) {
				assertEquals(f2[i], i + 200);
			}

			gpa.deallocateObjects(f2, 50);
			gpa.deallocateObjects(f1, 20);

		}
	}
}
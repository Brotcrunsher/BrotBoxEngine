#pragma once

#include "GeneralPurposeAllocator.h"
#include "UtilDebug.h"
#include "Random.h"

namespace bbe {
	namespace test {
		void testGeneralPurposeAllocator() {

			{
				GeneralPurposeAllocator gpa(10000);

				auto f1 = gpa.allocateObjects<float>(20);
				auto f2 = gpa.allocateObjects<float>(50);


				for (int i = 0; i < 20; i++) {
					f1[i] = (float)(i + 2);
				}
				for (int i = 0; i < 50; i++) {
					f2[i] = (float)(i + 200);
				}


				for (int i = 0; i < 20; i++) {
					assertEquals(f1[i], i + 2);
				}
				for (int i = 0; i < 50; i++) {
					assertEquals(f2[i], i + 200);
				}

				gpa.deallocateObjects(f1);
				gpa.deallocateObjects(f2);
			}
			

			{
				GeneralPurposeAllocator gpa(sizeof(float) * 128);
				List<GeneralPurposeAllocator::GeneralPurposeAllocatorPointer<float>> list;
				for (int i = 0; i < 64; i++)
				{
					list.pushBack(gpa.allocateObject<float>());
				}

				for (int i = 0; i < 64; i++)
				{
					gpa.deallocateObjects(list[i]);
				}
			}

			{
				GeneralPurposeAllocator gpa(sizeof(float) * 128);
				List<GeneralPurposeAllocator::GeneralPurposeAllocatorPointer<float>> list;
				Random rand;

				for (int i = 0; i < 1024 * 128; i++)
				{
					if (rand.randomBool())
					{
						if (list.getLength() < 64)
						{
							list.pushBack(gpa.allocateObject<float>());
						}
					}
					else
					{
						if (list.getLength() > 0)
						{
							size_t index = rand.randomInt(list.getLength());
							gpa.deallocateObjects(list[index]);
							list.removeIndex(index);
						}
					}
				}

				for (int i = 0; i < list.getLength(); i++)
				{
					gpa.deallocateObjects(list[i]);
				}

			}

		}
	}
}
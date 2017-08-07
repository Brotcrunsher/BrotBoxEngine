#pragma once

#include "BBE/GeneralPurposeAllocator.h"
#include "BBE/UtilDebug.h"
#include "BBE/Random.h"

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

				gpa.deallocate(f1);
				gpa.deallocate(f2);
			}
			

			{
				GeneralPurposeAllocator gpa(sizeof(float) * 128);
				List<GeneralPurposeAllocator::GeneralPurposeAllocatorPointer<float>> list;
				for (int i = 0; i < 64; i++)
				{
					list.add(gpa.allocateObject<float>());
				}

				for (int i = 0; i < 64; i++)
				{
					gpa.deallocate(list[i]);
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
							list.add(gpa.allocateObject<float>());
						}
					}
					else
					{
						if (list.getLength() > 0)
						{
							size_t index = (size_t)rand.randomInt((int)list.getLength());
							gpa.deallocate(list[index]);
							list.removeIndex(index);
						}
					}
				}

				for (int i = 0; i < list.getLength(); i++)
				{
					gpa.deallocate(list[i]);
				}

			}

			{
				GeneralPurposeAllocator gpa;

				auto f1 = gpa.allocateObjects<Person>();

				f1->print();

				gpa.deallocate(f1);
			}

		}
	}
}
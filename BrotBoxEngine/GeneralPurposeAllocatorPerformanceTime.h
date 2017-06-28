#pragma once

#include "GeneralPurposeAllocator.h"
#include "Random.h"
#include "CPUWatch.h"
#include "UtilTest.h"
#include "String.h"
#include <iostream>

namespace bbe {
	namespace test {
		void NewDeleteAllocationDeallocationSpeed()
		{
			{
				List<Person*> list;
				Random rand;

				CPUWatch watch;
				for (int i = 0; i < 1024 * 128; i++)
				{
					if (rand.randomBool())
					{
						if (list.getLength() < 64)
						{
							Person *p = new Person[128];
							list.pushBack(p);
						}
					}
					else
					{
						if (list.getLength() > 0)
						{
							size_t index = rand.randomInt(list.getLength());
							Person* p = list[index];
							delete[] (p);
							list.removeIndex(index);
						}
					}
				}

				for (int i = 0; i < list.getLength(); i++)
				{
					delete[] list[i];
				}
				std::cout << "Time new/delete took: " << watch.getTimeExpiredSeconds() << std::endl;

			}
		}
		void GeneralPurposeAllocatorAllocationDeallocationSpeed() {
			{
				GeneralPurposeAllocator gpa(sizeof(float) * 1024 * 1024);
				List<GeneralPurposeAllocator::GeneralPurposeAllocatorPointer<Person>> list;
				Random rand;

				CPUWatch watch;
				for (int i = 0; i < 1024 * 128; i++)
				{
					if (rand.randomBool())
					{
						if (list.getLength() < 64)
						{
							list.pushBack(gpa.allocateObjects<Person>(128));
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
				std::cout << "Time GPA took: " << watch.getTimeExpiredSeconds() << std::endl;

			}


			
		}
	}
}
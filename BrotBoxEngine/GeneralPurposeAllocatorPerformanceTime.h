#pragma once

#include "GeneralPurposeAllocator.h"
#include "DefragmentationAllocator.h"
#include "Random.h"
#include "CPUWatch.h"
#include "UtilTest.h"
#include "String.h"
#include <iostream>

namespace bbe {
	namespace test {
		int runs = 1024 * 128;

		void NewDeleteAllocationDeallocationSpeed()
		{
			{
				List<Person*> list;
				Random rand;

				CPUWatch watch;
				for (int i = 0; i < runs; i++)
				{
					if (rand.randomBool())
					{
						if (list.getLength() < 64)
						{
							Person *p = new Person[rand.randomInt(1024) + 1];
							list.add(p);
						}
					}
					else
					{
						if (list.getLength() > 0)
						{
							size_t index = (size_t)rand.randomInt((int)list.getLength());
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
		void GeneralPurposeAllocatorAllocationDeallocationSpeed()
		{
			{
				GeneralPurposeAllocator gpa(sizeof(Person) * 1024 * 1024);
				List<GeneralPurposeAllocator::GeneralPurposeAllocatorPointer<Person>> list;
				Random rand;

				CPUWatch watch;
				for (int i = 0; i < runs; i++)
				{
					if (rand.randomBool())
					{
						if (list.getLength() < 64)
						{
							list.add(gpa.allocateObjects<Person>(rand.randomInt(1024) + 1));
						}
					}
					else
					{
						if (list.getLength() > 0)
						{
							size_t index = (size_t)rand.randomInt((int)list.getLength());
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
		void DefragmentationAllocatorAllocationDeallocationSpeed()
		{
			{
				DefragmentationAllocator da(sizeof(Person) * 1024 * 1024);
				List<DefragmentationAllocator::DefragmentationAllocatorPointer<Person>> list;
				Random rand;

				CPUWatch watch;
				for (int i = 0; i < runs; i++)
				{
					if (rand.randomBool())
					{
						if (list.getLength() < 64)
						{
							list.add(da.allocateObjects<Person>(rand.randomInt(1024) + 1));
						}
					}
					else
					{
						if (list.getLength() > 0)
						{
							size_t index = (size_t)rand.randomInt((int)list.getLength());
							da.deallocateObjects(list[index]);
							list.removeIndex(index);
						}
					}

					if (i % 10 == 0)
					{
						da.defragment();
					}
				}

				for (int i = 0; i < list.getLength(); i++)
				{
					da.deallocateObjects(list[i]);
				}
				std::cout << "Time DA took: " << watch.getTimeExpiredSeconds() << std::endl;

			}
		}
	}
}
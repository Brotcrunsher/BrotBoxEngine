#pragma once

#include "BBE/DefragmentationAllocator.h"
#include "BBE/UtilDebug.h"
#include "BBE/Random.h"

namespace bbe {
	namespace test {
		enum State
		{
			unused, used, freed
		};

		void testDefragmentationAllocator() {

			{
				DefragmentationAllocator da(10000);

				auto f1 = da.allocateObjects<float>(20);
				auto f2 = da.allocateObjects<float>(50);


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

				da.deallocate(f1);
				da.deallocate(f2);
			}


			{
				DefragmentationAllocator da(sizeof(float) * 128);
				List<DefragmentationAllocator::DefragmentationAllocatorPointer<float>> list;
				for (int i = 0; i < 64; i++)
				{
					list.add(da.allocateObject<float>());
				}

				for (int i = 0; i < 64; i++)
				{
					da.deallocate(list[i]);
				}
			}

			{
				State *states = new State[1024 * 128];
				for (int i = 0; i < 1024 * 128; i++)
				{
					states[i] = unused;
				}
				DefragmentationAllocator da(sizeof(int) * 128, sizeof(int) * 128);
				List<DefragmentationAllocator::DefragmentationAllocatorPointer<int>> list;
				Random rand;

				for (int i = 0; i < 1024 * 128; i++)
				{
					if (rand.randomBool())
					{
						if (list.getLength() < 64)
						{
							list.add(da.allocateObject<int>());
							*(list[list.getLength() - 1]) = i;
							assertEquals(states[i], unused);
							states[i] = used;
						}
					}
					else
					{
						if (list.getLength() > 0)
						{
							size_t index = (size_t)rand.randomInt((int)list.getLength());
							assertEquals(states[*list[index]], used);
							states[*list[index]] = freed;

							*list[index] = 0;
							da.deallocate(list[index]);
							list.removeIndex(index);
						}
					}

					if (i % 10 == 0)
					{
						da.defragment();
					}
				}

				for (size_t i = 0; i < list.getLength(); i++)
				{
					da.deallocate(list[i]);
				}

			}

			{
				DefragmentationAllocator da;

				auto f1 = da.allocateObjects<Person>();

				f1->print();

				da.deallocate(f1);
			}
			
			{
				DefragmentationAllocator da;
				auto p1 = da.allocateObject<Person>("AName", "AStr", 18);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);

				assertEquals(da.needsDefragmentation(), false);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);

				assertEquals(da.defragment(), false);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);

				auto p2 = da.allocateObject<Person>("BName", "BStr", 29);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);
				assertEquals(p2->name, "BName");
				assertEquals(p2->adress, "BStr");
				assertEquals(p2->age, 29);

				assertEquals(da.needsDefragmentation(), false);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);
				assertEquals(p2->name, "BName");
				assertEquals(p2->adress, "BStr");
				assertEquals(p2->age, 29);

				assertEquals(da.defragment(), false);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);
				assertEquals(p2->name, "BName");
				assertEquals(p2->adress, "BStr");
				assertEquals(p2->age, 29);

				auto p3 = da.allocateObject<Person>("CName", "CStr", 89);
				void* orgAddrP3 = p3.getRaw();
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);
				assertEquals(p2->name, "BName");
				assertEquals(p2->adress, "BStr");
				assertEquals(p2->age, 29);
				assertEquals(p3->name, "CName");
				assertEquals(p3->adress, "CStr");
				assertEquals(p3->age, 89);
				assertUnequals(orgAddrP3, nullptr);

				assertEquals(da.needsDefragmentation(), false);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);
				assertEquals(p2->name, "BName");
				assertEquals(p2->adress, "BStr");
				assertEquals(p2->age, 29);
				assertEquals(p3->name, "CName");
				assertEquals(p3->adress, "CStr");
				assertEquals(p3->age, 89);

				assertEquals(da.defragment(), false);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);
				assertEquals(p2->name, "BName");
				assertEquals(p2->adress, "BStr");
				assertEquals(p2->age, 29);
				assertEquals(p3->name, "CName");
				assertEquals(p3->adress, "CStr");
				assertEquals(p3->age, 89);

				da.deallocate(p2);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);
				assertEquals(p3->name, "CName");
				assertEquals(p3->adress, "CStr");
				assertEquals(p3->age, 89);

				assertEquals(da.needsDefragmentation(), true);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);
				assertEquals(p3->name, "CName");
				assertEquals(p3->adress, "CStr");
				assertEquals(p3->age, 89);

				assertEquals(da.defragment(), true);
				assertEquals(p1->name, "AName");
				assertEquals(p1->adress, "AStr");
				assertEquals(p1->age, 18);
				assertEquals(p3->name, "CName");
				assertEquals(p3->adress, "CStr");
				assertEquals(p3->age, 89);
				assertUnequals(p3.getRaw(), orgAddrP3);

				da.deallocate(p1);
				assertEquals(p3->name, "CName");
				assertEquals(p3->adress, "CStr");
				assertEquals(p3->age, 89);

				assertEquals(da.needsDefragmentation(), true);
				assertEquals(p3->name, "CName");
				assertEquals(p3->adress, "CStr");
				assertEquals(p3->age, 89);

				assertEquals(da.defragment(), true);
				assertEquals(p3->name, "CName");
				assertEquals(p3->adress, "CStr");
				assertEquals(p3->age, 89);

				da.deallocate(p3);

				assertEquals(da.needsDefragmentation(), false);
			}

		}
	}
}
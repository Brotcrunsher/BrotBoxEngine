#pragma once

#include "BBE/DynamicArray.h"
#include "BBE/DefragmentationAllocator.h"
#include "BBE/List.h"
#include "BBE/Array.h"
#include "BBE/UtilTest.h"
#include "BBE/UtilDebug.h"

namespace bbe
{
	namespace test
	{
		void testDynamicArray()
		{
			{
				DynamicArray<int> da(16);

				assertEquals(da.getLength(), 16);
				assertUnequals(da.getRaw(), nullptr);

				for (size_t i = 0; i < da.getLength(); i++)
				{
					da[i] = i + 1000;
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i], i + 1000);
				}

				DynamicArray<int> da2 = da;

				assertEquals(da.getLength(), 16);
				assertUnequals(da.getRaw(), nullptr);
				assertEquals(da2.getLength(), 16);
				assertUnequals(da2.getRaw(), nullptr);

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i], i + 1000);
					assertEquals(da2[i], i + 1000);
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					da[i] = i + 2000;
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i], i + 2000);
					assertEquals(da2[i], i + 1000);
				}

				DynamicArray<int> da3 = std::move(da);

				assertEquals(da3.getLength(), 16);
				assertUnequals(da3.getRaw(), nullptr);
				assertEquals(da2.getLength(), 16);
				assertUnequals(da2.getRaw(), nullptr);

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da3[i], i + 2000);
					assertEquals(da2[i], i + 1000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					da2[i] = i + 4000;
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da3[i], i + 2000);
					assertEquals(da2[i], i + 4000);
				}

				const DynamicArray<int> da4 = da3;
				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da4[i], i + 2000);
					assertEquals(da3[i], i + 2000);
					assertEquals(da2[i], i + 4000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					da3[i] = i + 5000;
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da4[i], i + 2000);
					assertEquals(da3[i], i + 5000);
					assertEquals(da2[i], i + 4000);
				}

				DynamicArray<int> da5(128);
				for (size_t i = 0; i < da5.getLength(); i++)
				{
					da5[i] = 6000 + i;
				}

				da3 = da5;

				assertEquals(da3.getLength(), 128);
				assertEquals(da5.getLength(), 128);
				assertUnequals(da3.getRaw(), nullptr);
				assertUnequals(da5.getRaw(), nullptr);

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da3[i], 6000 + i);
					assertEquals(da5[i], 6000 + i);
				}

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					da3[i] = 7000 + i;
				}

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da3[i], 7000 + i);
					assertEquals(da5[i], 6000 + i);
				}

				da2 = std::move(da3);

				assertEquals(da2.getLength(), 128);
				assertEquals(da5.getLength(), 128);
				assertUnequals(da2.getRaw(), nullptr);
				assertUnequals(da5.getRaw(), nullptr);
				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da2[i], 7000 + i);
					assertEquals(da5[i], 6000 + i);
				}

				List<int, false> list{ 1, 2, 3, 4, 5 };
				DynamicArray<int> da6(list);
				assertEquals(da6.getLength(), 5);
				assertUnequals(da6.getRaw(), nullptr);
				for (size_t i = 0; i < da6.getLength(); i++)
				{
					assertEquals(da6[i], i + 1);
				}

				List<int, true> list2{ 1, 2, 3, 4, 5 };
				DynamicArray<int> da8(list2);
				assertEquals(da8.getLength(), 5);
				assertUnequals(da8.getRaw(), nullptr);
				for (size_t i = 0; i < da8.getLength(); i++)
				{
					assertEquals(da8[i], i + 1);
				}

				Array<int, 6> arr;
				for (size_t i = 0; i < arr.getLength(); i++)
				{
					arr[i] = i + 50;
				}
				DynamicArray<int> da7(arr);
				assertEquals(da7.getLength(), 6);
				assertUnequals(da7.getRaw(), nullptr);
				for (size_t i = 0; i < da7.getLength(); i++)
				{
					assertEquals(da7[i], i + 50);
				}

				DynamicArray<int> hashDa1{ 1, 2, 3, 4 };
				DynamicArray<int> hashDa2{ 1, 2, 3, 4 };
				DynamicArray<int> hashDa3{ 2, 3, 4 };

				assertEquals  (hash(hashDa1), hash(hashDa2));
				assertUnequals(hash(hashDa1), hash(hashDa3));

				for (size_t i = 0; i < hashDa1.getLength(); i++)
				{
					assertEquals(hashDa1[i], i + 1);
				}

			}


			












			{
				DefragmentationAllocator defrag(1024 * 1024);
				DynamicArray<int, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<int>> da(16, &defrag);

				assertEquals(da.getLength(), 16);
				assertUnequals(da.getRaw(), nullptr);

				for (size_t i = 0; i < da.getLength(); i++)
				{
					da[i] = i + 1000;
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i], i + 1000);
				}

				DynamicArray<int, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<int>> da2 = da;

				assertEquals(da.getLength(), 16);
				assertUnequals(da.getRaw(), nullptr);
				assertEquals(da2.getLength(), 16);
				assertUnequals(da2.getRaw(), nullptr);

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i], i + 1000);
					assertEquals(da2[i], i + 1000);
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					da[i] = i + 2000;
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i], i + 2000);
					assertEquals(da2[i], i + 1000);
				}

				DynamicArray<int, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<int>> da3 = std::move(da);

				assertEquals(da3.getLength(), 16);
				assertUnequals(da3.getRaw(), nullptr);
				assertEquals(da2.getLength(), 16);
				assertUnequals(da2.getRaw(), nullptr);

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da3[i], i + 2000);
					assertEquals(da2[i], i + 1000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					da2[i] = i + 4000;
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da3[i], i + 2000);
					assertEquals(da2[i], i + 4000);
				}

				const DynamicArray<int, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<int>> da4 = da3;
				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da4[i], i + 2000);
					assertEquals(da3[i], i + 2000);
					assertEquals(da2[i], i + 4000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					da3[i] = i + 5000;
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da4[i], i + 2000);
					assertEquals(da3[i], i + 5000);
					assertEquals(da2[i], i + 4000);
				}

				DynamicArray<int, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<int>> da5(128, &defrag);
				for (size_t i = 0; i < da5.getLength(); i++)
				{
					da5[i] = 6000 + i;
				}

				da3 = da5;

				assertEquals(da3.getLength(), 128);
				assertEquals(da5.getLength(), 128);
				assertUnequals(da3.getRaw(), nullptr);
				assertUnequals(da5.getRaw(), nullptr);

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da3[i], 6000 + i);
					assertEquals(da5[i], 6000 + i);
				}

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					da3[i] = 7000 + i;
				}

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da3[i], 7000 + i);
					assertEquals(da5[i], 6000 + i);
				}

				da2 = std::move(da3);

				assertEquals(da2.getLength(), 128);
				assertEquals(da5.getLength(), 128);
				assertUnequals(da2.getRaw(), nullptr);
				assertUnequals(da5.getRaw(), nullptr);
				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da2[i], 7000 + i);
					assertEquals(da5[i], 6000 + i);
				}

				List<int, false> list{ 1, 2, 3, 4, 5 };
				DynamicArray<int, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<int>> da6(list, &defrag);
				assertEquals(da6.getLength(), 5);
				assertUnequals(da6.getRaw(), nullptr);
				for (size_t i = 0; i < da6.getLength(); i++)
				{
					assertEquals(da6[i], i + 1);
				}

				List<int, true> list2{ 1, 2, 3, 4, 5 };
				DynamicArray<int, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<int>> da8(list2, &defrag);
				assertEquals(da8.getLength(), 5);
				assertUnequals(da8.getRaw(), nullptr);
				for (size_t i = 0; i < da8.getLength(); i++)
				{
					assertEquals(da8[i], i + 1);
				}

				Array<int, 6> arr;
				for (size_t i = 0; i < arr.getLength(); i++)
				{
					arr[i] = i + 50;
				}
				DynamicArray<int, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<int>> da7(arr, &defrag);
				assertEquals(da7.getLength(), 6);
				assertUnequals(da7.getRaw(), nullptr);
				for (size_t i = 0; i < da7.getLength(); i++)
				{
					assertEquals(da7[i], i + 50);
				}

			}


			{
				DynamicArray<Person> da(16);

				assertEquals(da.getLength(), 16);
				assertUnequals(da.getRaw(), nullptr);

				for (size_t i = 0; i < da.getLength(); i++)
				{
					da[i] = Person("", "", i + 1000);
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i].age, i + 1000);
				}

				DynamicArray<Person> da2 = da;

				assertEquals(da.getLength(), 16);
				assertUnequals(da.getRaw(), nullptr);
				assertEquals(da2.getLength(), 16);
				assertUnequals(da2.getRaw(), nullptr);

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i].age, i + 1000);
					assertEquals(da2[i].age, i + 1000);
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					da[i] = Person("", "", i + 2000);
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i].age, i + 2000);
					assertEquals(da2[i].age, i + 1000);
				}

				DynamicArray<Person> da3 = std::move(da);

				assertEquals(da3.getLength(), 16);
				assertUnequals(da3.getRaw(), nullptr);
				assertEquals(da2.getLength(), 16);
				assertUnequals(da2.getRaw(), nullptr);

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da3[i].age, i + 2000);
					assertEquals(da2[i].age, i + 1000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					da2[i] = Person("", "", i + 4000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da3[i].age, i + 2000);
					assertEquals(da2[i].age, i + 4000);
				}

				const DynamicArray<Person> da4 = da3;
				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da4[i].age, i + 2000);
					assertEquals(da3[i].age, i + 2000);
					assertEquals(da2[i].age, i + 4000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					da3[i] = Person("", "", i + 5000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da4[i].age, i + 2000);
					assertEquals(da3[i].age, i + 5000);
					assertEquals(da2[i].age, i + 4000);
				}

				DynamicArray<Person> da5(128);
				for (size_t i = 0; i < da5.getLength(); i++)
				{
					da5[i] = Person("", "", 6000 + i);
				}

				da3 = da5;

				assertEquals(da3.getLength(), 128);
				assertEquals(da5.getLength(), 128);
				assertUnequals(da3.getRaw(), nullptr);
				assertUnequals(da5.getRaw(), nullptr);

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da3[i].age, 6000 + i);
					assertEquals(da5[i].age, 6000 + i);
				}

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					da3[i] = Person("", "", 7000 + i);
				}

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da3[i].age, 7000 + i);
					assertEquals(da5[i].age, 6000 + i);
				}

				da2 = std::move(da3);

				assertEquals(da2.getLength(), 128);
				assertEquals(da5.getLength(), 128);
				assertUnequals(da2.getRaw(), nullptr);
				assertUnequals(da5.getRaw(), nullptr);
				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da2[i].age, 7000 + i);
					assertEquals(da5[i].age, 6000 + i);
				}

				List<Person, false> list{ Person("", "", 1), 
					Person("", "", 2),
					Person("", "", 3),
					Person("", "", 4),
					Person("", "", 5) };
				DynamicArray<Person> da6(list);
				assertEquals(da6.getLength(), 5);
				assertUnequals(da6.getRaw(), nullptr);
				for (size_t i = 0; i < da6.getLength(); i++)
				{
					assertEquals(da6[i].age, i + 1);
				}

				List<Person, true> list2{
					Person("", "", 1),
					Person("", "", 2),
					Person("", "", 3),
					Person("", "", 4),
					Person("", "", 5) };
				DynamicArray<Person> da8(list2);
				assertEquals(da8.getLength(), 5);
				assertUnequals(da8.getRaw(), nullptr);
				for (size_t i = 0; i < da8.getLength(); i++)
				{
					assertEquals(da8[i].age, i + 1);
				}

				Array<Person, 6> arr;
				for (size_t i = 0; i < arr.getLength(); i++)
				{
					arr[i] = Person("", "", i + 50);
				}
				DynamicArray<Person> da7(arr);
				assertEquals(da7.getLength(), 6);
				assertUnequals(da7.getRaw(), nullptr);
				for (size_t i = 0; i < da7.getLength(); i++)
				{
					assertEquals(da7[i].age, i + 50);
				}

				DynamicArray<Person> hashDa1{
					Person("", "", 1),
					Person("", "", 2),
					Person("", "", 3),
					Person("", "", 4)
				};
				DynamicArray<Person> hashDa2{
					Person("", "", 1),
					Person("", "", 2),
					Person("", "", 3),
					Person("", "", 4)
				};
				DynamicArray<Person> hashDa3{
					Person("", "", 2),
					Person("", "", 3),
					Person("", "", 4)
				};

				assertEquals(hash(hashDa1), hash(hashDa2));
				assertUnequals(hash(hashDa1), hash(hashDa3));

				for (size_t i = 0; i < hashDa1.getLength(); i++)
				{
					assertEquals(hashDa1[i], Person("", "", i + 1));
				}
			}

			{
				DefragmentationAllocator defrag (1024 * 1024);
				DynamicArray<Person, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<Person>> da(16, &defrag);

				assertEquals(da.getLength(), 16);
				assertUnequals(da.getRaw(), nullptr);

				for (size_t i = 0; i < da.getLength(); i++)
				{
					da[i] = Person("", "", i + 1000);
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i].age, i + 1000);
				}

				DynamicArray<Person, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<Person>> da2 = da;

				assertEquals(da.getLength(), 16);
				assertUnequals(da.getRaw(), nullptr);
				assertEquals(da2.getLength(), 16);
				assertUnequals(da2.getRaw(), nullptr);

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i].age, i + 1000);
					assertEquals(da2[i].age, i + 1000);
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					da[i] = Person("", "", i + 2000);
				}

				for (size_t i = 0; i < da.getLength(); i++)
				{
					assertEquals(da[i].age, i + 2000);
					assertEquals(da2[i].age, i + 1000);
				}

				DynamicArray<Person, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<Person>> da3 = std::move(da);

				assertEquals(da3.getLength(), 16);
				assertUnequals(da3.getRaw(), nullptr);
				assertEquals(da2.getLength(), 16);
				assertUnequals(da2.getRaw(), nullptr);

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da3[i].age, i + 2000);
					assertEquals(da2[i].age, i + 1000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					da2[i] = Person("", "", i + 4000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da3[i].age, i + 2000);
					assertEquals(da2[i].age, i + 4000);
				}

				const DynamicArray<Person, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<Person>> da4 = da3;
				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da4[i].age, i + 2000);
					assertEquals(da3[i].age, i + 2000);
					assertEquals(da2[i].age, i + 4000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					da3[i] = Person("", "", i + 5000);
				}

				for (size_t i = 0; i < da3.getLength(); i++)
				{
					assertEquals(da4[i].age, i + 2000);
					assertEquals(da3[i].age, i + 5000);
					assertEquals(da2[i].age, i + 4000);
				}

				DynamicArray<Person, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<Person>> da5(128, &defrag);
				for (size_t i = 0; i < da5.getLength(); i++)
				{
					da5[i] = Person("", "", 6000 + i);
				}

				da3 = da5;

				assertEquals(da3.getLength(), 128);
				assertEquals(da5.getLength(), 128);
				assertUnequals(da3.getRaw(), nullptr);
				assertUnequals(da5.getRaw(), nullptr);

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da3[i].age, 6000 + i);
					assertEquals(da5[i].age, 6000 + i);
				}

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					da3[i] = Person("", "", 7000 + i);
				}

				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da3[i].age, 7000 + i);
					assertEquals(da5[i].age, 6000 + i);
				}

				da2 = std::move(da3);

				assertEquals(da2.getLength(), 128);
				assertEquals(da5.getLength(), 128);
				assertUnequals(da2.getRaw(), nullptr);
				assertUnequals(da5.getRaw(), nullptr);
				for (size_t i = 0; i < da5.getLength(); i++)
				{
					assertEquals(da2[i].age, 7000 + i);
					assertEquals(da5[i].age, 6000 + i);
				}

				List<Person, false> list{ Person("", "", 1), 
					Person("", "", 2),
					Person("", "", 3),
					Person("", "", 4),
					Person("", "", 5) };
				DynamicArray<Person, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<Person>> da6(list, &defrag);
				assertEquals(da6.getLength(), 5);
				assertUnequals(da6.getRaw(), nullptr);
				for (size_t i = 0; i < da6.getLength(); i++)
				{
					assertEquals(da6[i].age, i + 1);
				}

				List<Person, true> list2{
					Person("", "", 1),
					Person("", "", 2),
					Person("", "", 3),
					Person("", "", 4),
					Person("", "", 5) };
				DynamicArray<Person, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<Person>> da8(list2, &defrag);
				assertEquals(da8.getLength(), 5);
				assertUnequals(da8.getRaw(), nullptr);
				for (size_t i = 0; i < da8.getLength(); i++)
				{
					assertEquals(da8[i].age, i + 1);
				}

				Array<Person, 6> arr;
				for (size_t i = 0; i < arr.getLength(); i++)
				{
					arr[i] = Person("", "", i + 50);
				}
				DynamicArray<Person, DefragmentationAllocator, DefragmentationAllocator::DefragmentationAllocatorPointer<Person>> da7(arr, &defrag);
				assertEquals(da7.getLength(), 6);
				assertUnequals(da7.getRaw(), nullptr);
				for (size_t i = 0; i < da7.getLength(); i++)
				{
					assertEquals(da7[i].age, i + 50);
				}

			}











			{
				const DynamicArray<int> da(1337);
				assertEquals(da.getLength(), 1337);
				assertUnequals(da.getRaw(), nullptr);
			}
		}
	}
}
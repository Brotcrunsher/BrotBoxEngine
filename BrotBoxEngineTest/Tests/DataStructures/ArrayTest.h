#pragma once

#include "BBE/Array.h"
#include "BBE/UtilTest.h"
#include "BBE/UtilDebug.h"

namespace bbe
{
	namespace test
	{
		void testArray()
		{
			{
				bbe::Array<int, 16> arr;

				assertUnequals(arr.getRaw(), nullptr);
				assertEquals(arr.getLength(), 16);

				const bbe::Array<int, 16> constArr;
				assertUnequals(constArr.getRaw(), nullptr);
				assertEquals(constArr.getLength(), 16);

				for (size_t i = 0; i < arr.getLength(); i++)
				{
					arr[i] = i + 1000;
				}

				for (size_t i = 0; i < arr.getLength(); i++)
				{
					assertEquals(arr[i], i + 1000);
				}

				const bbe::Array<int, 16> constArr2 = arr;

				for (size_t i = 0; i < arr.getLength(); i++)
				{
					assertEquals(arr[i], i + 1000);
				}
				for (size_t i = 0; i < constArr2.getLength(); i++)
				{
					assertEquals(constArr2[i], i + 1000);
				}


				for (size_t i = 0; i < arr.getLength(); i++)
				{
					arr[i] = i + 2000;
				}

				for (size_t i = 0; i < arr.getLength(); i++)
				{
					assertEquals(arr[i], i + 2000);
				}
				for (size_t i = 0; i < constArr2.getLength(); i++)
				{
					assertEquals(constArr2[i], i + 1000);
				}

				bbe::Array<int, 16> arr2 = std::move(arr);
				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i], i + 2000);
				}

				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					arr2[i] = i + 3000;
				}
				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i], i + 3000);
				}

				bbe::Array<int, 16> arr3;
				arr3 = arr2;

				for (size_t i = 0; i < arr3.getLength(); i++)
				{
					assertEquals(arr3[i], i + 3000);
				}
				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i], i + 3000);
				}

				for (size_t i = 0; i < arr3.getLength(); i++)
				{
					arr3[i] = i + 4000;
				}
				for (size_t i = 0; i < arr3.getLength(); i++)
				{
					assertEquals(arr3[i], i + 4000);
				}
				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i], i + 3000);
				}

				arr2 = std::move(arr3);

				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i], i + 4000);
				}

				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i], i + 4000);
				}

				bbe::Array<int, 16> arr4;
				arr4 = arr2;

				assertEquals(hash(arr4), hash(arr2));

				bbe::Array<int, 4> arr5{ 1, 2, 3, 4 };
				for (size_t i = 0; i < arr5.getLength(); i++)
				{
					assertEquals(arr5[i], i + 1);
				}
			}


			{
				bbe::Array<Person, 16> arr;

				assertUnequals(arr.getRaw(), nullptr);
				assertEquals(arr.getLength(), 16);

				const bbe::Array<int, 16> constArr;
				assertUnequals(constArr.getRaw(), nullptr);
				assertEquals(constArr.getLength(), 16);

				for (size_t i = 0; i < arr.getLength(); i++)
				{
					arr[i] = Person("", "", i + 1000);
				}
				
				for (size_t i = 0; i < arr.getLength(); i++)
				{
					assertEquals(arr[i].age, i + 1000);
				}

				const bbe::Array<Person, 16> constArr2 = arr;

				for (size_t i = 0; i < arr.getLength(); i++)
				{
					assertEquals(arr[i].age, i + 1000);
				}
				for (size_t i = 0; i < constArr2.getLength(); i++)
				{
					assertEquals(constArr2[i].age, i + 1000);
				}


				for (size_t i = 0; i < arr.getLength(); i++)
				{
					arr[i] = Person("", "", i + 2000);
				}

				for (size_t i = 0; i < arr.getLength(); i++)
				{
					assertEquals(arr[i].age, i + 2000);
				}
				for (size_t i = 0; i < constArr2.getLength(); i++)
				{
					assertEquals(constArr2[i].age, i + 1000);
				}

				bbe::Array<Person, 16> arr2 = std::move(arr);

				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i].age, i + 2000);
				}

				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					arr2[i] = Person("", "", i + 3000);
				}
				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i].age, i + 3000);
				}

				bbe::Array<Person, 16> arr3;
				arr3 = arr2;

				for (size_t i = 0; i < arr3.getLength(); i++)
				{
					assertEquals(arr3[i].age, i + 3000);
				}
				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i].age, i + 3000);
				}

				for (size_t i = 0; i < arr3.getLength(); i++)
				{
					arr3[i] = Person("", "", i + 4000);
				}
				for (size_t i = 0; i < arr3.getLength(); i++)
				{
					assertEquals(arr3[i].age, i + 4000);
				}
				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i].age, i + 3000);
				}

				arr2 = std::move(arr3);

				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i].age, i + 4000);
				}
				for (size_t i = 0; i < arr2.getLength(); i++)
				{
					assertEquals(arr2[i].age, i + 4000);
				}

				bbe::Array<Person, 16> arr4;
				arr4 = arr2;

				assertEquals(hash(arr4), hash(arr2));
			}
		}
	}
}

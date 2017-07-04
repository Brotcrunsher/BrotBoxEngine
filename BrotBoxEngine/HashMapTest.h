#pragma once


#include "Hash.h"
#include "HashMap.h"
#include "UtilDebug.h"


namespace bbe
{
	namespace test
	{
		void testHashMap()
		{
			//TODO add more tests
			{
				HashMap<int, Person> hashMap;

				for (int i = 0; i < 1024; i++)
				{
					assertEquals(hashMap.get(i), nullptr);
				}
				
				hashMap.add(1, Person("Peter", "AStr", 40));
				assertEquals(hashMap.get(1)->name, "Peter");
				assertEquals(hashMap.get(1)->adress, "AStr");
				assertEquals(hashMap.get(1)->age, 40);

				hashMap.add(2, Person("B", "BStr", 41));
				assertEquals(hashMap.get(1)->name, "Peter");
				assertEquals(hashMap.get(1)->adress, "AStr");
				assertEquals(hashMap.get(1)->age, 40);
				assertEquals(hashMap.get(2)->name, "B");
				assertEquals(hashMap.get(2)->adress, "BStr");
				assertEquals(hashMap.get(2)->age, 41);

				for (int i = 3; i < 1024 * 16; i++)
				{
					hashMap.add(i, Person("Undefined Name", "Undefinded Addr", i + 20));
					assertEquals(hashMap.get(1)->name, "Peter");
					assertEquals(hashMap.get(1)->adress, "AStr");
					assertEquals(hashMap.get(1)->age, 40);
					assertEquals(hashMap.get(2)->name, "B");
					assertEquals(hashMap.get(2)->adress, "BStr");
					assertEquals(hashMap.get(2)->age, 41);
					for (int k = 3; k <= i; k++)
					{
						assertEquals(hashMap.get(k)->name, "Undefined Name");
						assertEquals(hashMap.get(k)->adress, "Undefinded Addr");
						assertEquals(hashMap.get(k)->age, k + 20);
					}
				}
			}
			
		}
	}
}
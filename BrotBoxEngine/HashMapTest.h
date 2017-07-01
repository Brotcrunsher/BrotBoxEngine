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

				for (size_t i = 0; i < 1024; i++)
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
			}
			
		}
	}
}
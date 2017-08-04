#pragma once

#include "HashMap.h"
#include "String.h"
#include "CPUWatch.h"
#include <unordered_map>
#include <iostream>


namespace bbe
{
	namespace test
	{
		void hashMapPrintSpeed()
		{
			{
				HashMap<int, String> map;

				CPUWatch watchAdd;
				for (int i = 0; i < 1024 * 1024; i++)
				{
					map.add(i, L"Hallo");
				}
				std::cout << "BBE Hashmap add speed: " << watchAdd.getTimeExpiredSeconds() << std::endl;

				CPUWatch watchGet;
				for (int i = 0; i < 1024 * 1024; i++)
				{
					map.get(i);
				}
				std::cout << "BBE Hashmap get speed: " << watchGet.getTimeExpiredSeconds() << std::endl;
			}

			{
				int a = 5;
				std::hash<int> hasher;
				hasher(a);

				std::unordered_map<int, bbe::String> map;
				CPUWatch watchAdd;
				for (int i = 0; i < 1024 * 1024; i++)
				{
					map.insert(std::pair<int, bbe::String>(i, L"Hallo"));
				}
				std::cout << "STD Hashmap add speed: " << watchAdd.getTimeExpiredSeconds() << std::endl;

				CPUWatch watchGet;
				for (int i = 0; i < 1024 * 1024; i++)
				{
					map.find(i);
				}
				std::cout << "STD Hashmap get speed: " << watchGet.getTimeExpiredSeconds() << std::endl;
			}
		}
	}
}
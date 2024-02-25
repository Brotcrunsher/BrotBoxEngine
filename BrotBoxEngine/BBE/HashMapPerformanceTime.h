#pragma once

#include "../BBE/HashMap.h"
#include "../BBE/String.h"
#include "../BBE/CPUWatch.h"
#include "../BBE/Logging.h"
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
				BBELOGLN("BBE Hashmap add speed: " << watchAdd.getTimeExpiredSeconds());

				CPUWatch watchGet;
				for (int i = 0; i < 1024 * 1024; i++)
				{
					map.get(i);
				}
				BBELOGLN("BBE Hashmap get speed: " << watchGet.getTimeExpiredSeconds());
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
				BBELOGLN("STD Hashmap add speed: " << watchAdd.getTimeExpiredSeconds());

				CPUWatch watchGet;
				for (int i = 0; i < 1024 * 1024; i++)
				{
					map.find(i);
				}
				BBELOGLN("STD Hashmap get speed: " << watchGet.getTimeExpiredSeconds());
			}
		}
	}
}
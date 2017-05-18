#pragma once

#include "PoolAllocator.h"
#include <iostream>
#include "UtilTest.h"
#include "CPUWatch.h"
#include "String.h"
#include <string>
#include <vector>

namespace bbe {
	namespace test {
		void someTest(const wchar_t* arr, size_t size) {

		}

		void stringSpeedAddition() {
			double total = 0;
			int numRuns = 0;
			while (true) {
				CPUWatch allocationWatch;
				bbe::String a(L"Hallo ");
				bbe::String b(L"Welt");
				for (int i = 0; i < 10000000; i++) {
					a += b;
				}

				total += allocationWatch.getTimeExpiredSeconds();
				numRuns++;



				std::cout << total/numRuns << std::endl;
			}
		}

		void stringSpeedCreation() {
			//char c = 0;
			//std::vector<std::wstring> vec;
			//std::vector<bbe::String> vec;
			//std::cin >> c;
			while (true) {
				CPUWatch allocationWatch;
				for (int i = 0; i < 10000000; i++) {
					std::wstring ape(L"Hallooooooooooooooooooooo!");
					//bbe::String ape(L"Hallooooooooooooooooooooo!");

					//ape[0] = c;
					//vec.push_back(ape);
				}
				std::cout << allocationWatch.getTimeExpiredSeconds() << std::endl;
				//vec.clear();
			}

		}
	}
}
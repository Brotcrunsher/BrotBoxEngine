#pragma once

#include "String.h"
#include <iostream>

namespace bbe {
	namespace test {
		void testString() {

			bbe::String emptyString;
			bbe::String stringWChar(L"Hallo WChar");
			bbe::String stringClassic("Hallo Classic");
			
			std::cout << (emptyString == "") << std::endl;
			std::cout << ("" == emptyString) << std::endl;
			std::cout << (emptyString == "a") << std::endl;
			std::cout << ("a" == emptyString) << std::endl;

			std::wcout << stringWChar.getRaw()   << " " << stringWChar.getLength()   << std::endl;
			std::wcout << stringClassic.getRaw() << " " << stringClassic.getLength() << std::endl;
			std::wcout << emptyString.getRaw()   << " " << emptyString.getLength()   << std::endl;
			std::wcout << (emptyString   == stringWChar) << " " << (emptyString   == stringClassic) << " " << (emptyString   == emptyString) << std::endl
				       << (stringWChar   == stringWChar) << " " << (stringWChar   == stringClassic) << " " << (stringWChar   == emptyString) << std::endl
				       << (stringClassic == stringWChar) << " " << (stringClassic == stringClassic) << " " << (stringClassic == emptyString) << std::endl;

			std::cout << (emptyString != "") << std::endl;
			std::cout << ("" != emptyString) << std::endl;
			std::cout << (emptyString != "a") << std::endl;
			std::cout << ("a" != emptyString) << std::endl;

			std::wcout << stringWChar.getRaw()   << " " << stringWChar.getLength()   << std::endl;
			std::wcout << stringClassic.getRaw() << " " << stringClassic.getLength() << std::endl;
			std::wcout << emptyString.getRaw()   << " " << emptyString.getLength()   << std::endl;
			std::wcout << (emptyString != stringWChar) << " " << (emptyString != stringClassic) << " " << (emptyString != emptyString) << std::endl
				       << (stringWChar != stringWChar) << " " << (stringWChar != stringClassic) << " " << (stringWChar != emptyString) << std::endl
				       << (stringClassic != stringWChar) << " " << (stringClassic != stringClassic) << " " << (stringClassic != emptyString) << std::endl;


			bbe::String addString001("Servus ");
			addString001 = addString001 + L"Weltchen";
			std::wcout << addString001.getRaw();

			bbe::String numberString("1929");
			long number = numberString.toLong();
			std::cout << number << std::endl;


			bbe::String trim1("                                                      dguisd fdis                      ");
			trim1.trim();

			std::wcout << "\"" << trim1.getRaw() << "\" " << trim1.getLength() << std::endl;


			bbe::String countString("Ha! Test yeh Test Test TestTest okay edgeTestcase?");
			std::wcout << countString.count(L"Test") << " " << countString.count(L"hahaha") << " " << countString.count(L"") << std::endl;
			auto split = countString.split(L" ");
			std::wcout << countString.getRaw() << std::endl;
			for (size_t i = 0; i < split.getLength(); i++) {
				std::wcout << "\"" << split[i].getRaw() << "\"" << std::endl;
			}
		}
	}
}
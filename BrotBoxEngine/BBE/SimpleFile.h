#pragma once

#include "../BBE/List.h"
#include "../BBE/String.h"
#include <fstream>

namespace bbe
{
	namespace simpleFile
	{
		bbe::List<unsigned char> readBinaryFile(const bbe::String &filepath);

		bbe::List<float> readFloatArrFromFile(const bbe::String& filePath);
		void writeFloatArrToFile(const bbe::String &filePath, float *arr, size_t size);
		void writeStringToFile(const bbe::String& filePath, const bbe::String& stringToWrite);
		bool doesFileExist(const bbe::String& filePath);
		bbe::String readFile(const bbe::String& filePath);
	}
}

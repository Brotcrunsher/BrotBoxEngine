#pragma once

#include "../BBE/List.h"
#include "../BBE/String.h"
#include <fstream>

namespace bbe
{
	namespace simpleFile
	{
		bbe::List<unsigned char> readBinaryFile(const bbe::String &filepath);

		void writeFloatArrToFile(const bbe::String &filePath, float *arr, size_t size);
	}
}

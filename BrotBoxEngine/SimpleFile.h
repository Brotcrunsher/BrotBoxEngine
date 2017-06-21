#pragma once

#include "List.h"
#include "String.h"
#include <fstream>

namespace bbe
{
	namespace simpleFile
	{
		bbe::List<char> readBinaryFile(const bbe::String &filepath)
		{
			//UNTESTED
			std::ifstream file(filepath.getRaw(), std::ios::binary | std::ios::ate);

			if (file)
			{
				size_t fileSize = (size_t)file.tellg();
				bbe::List<char> fileBuffer;
				fileBuffer.resizeCapacityAndLength(fileSize);
				file.seekg(0);
				file.read(fileBuffer.getRaw(), fileSize);
				file.close();
				return fileBuffer;
			}
			else
			{
				throw std::runtime_error("Failed to open file!");
			}
		}
	}
}
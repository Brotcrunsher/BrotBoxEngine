#include "BBE/SimpleFile.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>

bbe::List<char> bbe::simpleFile::readBinaryFile(const bbe::String & filepath)
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

void bbe::simpleFile::writeFloatArrToFile(const bbe::String & filePath, float * arr, size_t size)
{	
	std::ofstream file(filePath.getRaw());
	for (std::size_t i = 0; i < size; i++)
	{
		file << arr[i] << "\n";
	}
	file.close();
}

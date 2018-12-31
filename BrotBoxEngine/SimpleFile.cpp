#include "stdafx.h"
#include "BBE/SimpleFile.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>

bbe::List<char> bbe::simpleFile::readBinaryFile(const bbe::String & filepath)
{
	//UNTESTED
	char path[1024];
	int length = wcstombs(path, filepath.getRaw(), sizeof(path));
	if(length >= sizeof(path) - 1)
	{
		throw std::runtime_error("Path too long!");
	}
	std::ifstream file(path, std::ios::binary | std::ios::ate);

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
	char path[1024];
	int length = wcstombs(path, filePath.getRaw(), sizeof(path));
	if(length >= sizeof(path) - 1)
	{
		throw std::runtime_error("Path too long!");
	}
	std::ofstream file(path);
	for (int i = 0; i < size; i++)
	{
		file << arr[i] << "\n";
	}
	file.close();
}

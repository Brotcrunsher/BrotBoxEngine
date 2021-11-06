#include "BBE/SimpleFile.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>

bbe::List<unsigned char> bbe::simpleFile::readBinaryFile(const bbe::String & filepath)
{
	//UNTESTED
	std::ifstream file(filepath.getRaw(), std::ios::binary | std::ios::ate);

	if (file)
	{
		size_t fileSize = (size_t)file.tellg();
		bbe::List<unsigned char> fileBuffer;
		fileBuffer.resizeCapacityAndLength(fileSize);
		file.seekg(0);
		file.read((char*)fileBuffer.getRaw(), fileSize);
		file.close();
		return fileBuffer;
	}
	else
	{
		throw std::runtime_error("Failed to open file!");
	}
}

bbe::List<float> bbe::simpleFile::readFloatArrFromFile(const bbe::String& filePath)
{
	std::ifstream file(filePath.getRaw());
	std::string line;
	bbe::List<float> retVal;
	while(std::getline(file, line))
	{
		std::istringstream s(line);
		float f;
		if (!(s >> f)) throw IllegalArgumentException();
		retVal.add(f);
	}

	return retVal;
}

void bbe::simpleFile::writeFloatArrToFile(const bbe::String & filePath, float * arr, size_t size)
{	
	std::ofstream file(filePath.getRaw());
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file!");
	}
	for (std::size_t i = 0; i < size; i++)
	{
		file << arr[i] << "\n";
	}
	file.close();
}

void bbe::simpleFile::writeStringToFile(const bbe::String& filePath, const bbe::String& stringToWrite)
{
	std::ofstream file(filePath.getRaw());
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file!");
	}
	file << stringToWrite.getRaw();
	file.close();
}

bool bbe::simpleFile::doesFileExist(const bbe::String& filePath)
{
	std::ifstream f(filePath.getRaw());
	return (bool)f;
}

bbe::String bbe::simpleFile::readFile(const bbe::String& filePath)
{
	std::ifstream f(filePath.getRaw());
	if (!f.is_open()) {
		throw std::runtime_error("Could not open file!");
	}
	std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	return bbe::String(str.data());
}

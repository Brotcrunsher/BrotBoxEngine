#include "stdafx.h"
#include "BBE/SimpleFile.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>

bbe::List<char> bbe::simpleFile::readBinaryFile(const bbe::String & filepath)
{
	//UNTESTED
	std::cout << filepath.getLength() << std::endl;
	std::cout << filepath.getRaw() << std::endl;
	for(int i = 0; i<filepath.getLength(); i++){
		std::cout << filepath.getRaw()[i] << std::endl;
	}

	wchar_t path_w[1024];
	char path[1024];	
	memset(path_w, 0, 1024*sizeof(wchar_t));

	std::cout << "OLA!" << std::endl;
	if(filepath.getLength() >= sizeof(path) - 1)
	{
		throw std::runtime_error("Path too long!");
	}
	for(int i = 0; i<filepath.getLength(); i++)
	{
		std::cout << "OLA!" << std::endl;
		path_w[i] = filepath.getRaw()[i];
	}

	std::cout << "OLA!" << std::endl;
	int length = wcstombs(path, path_w, sizeof(path));
	if(length >= sizeof(path) - 1)
	{
		throw std::runtime_error("Path too long!");
	}
	std::cout << "Reading File: " << path << std::endl;
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

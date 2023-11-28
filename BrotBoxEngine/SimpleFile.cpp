#include "BBE/SimpleFile.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <filesystem>

bbe::ByteBuffer bbe::simpleFile::readBinaryFile(const bbe::String & filepath)
{
	if (std::filesystem::is_directory(filepath.getRaw()))
	{
		throw bbe::IllegalArgumentException();
	}

	std::ifstream file(filepath.getRaw(), std::ios::binary | std::ios::ate);
	
	if (file)
	{
		size_t fileSize = (size_t)file.tellg();
		if (fileSize == (size_t)-1)
		{
			throw std::runtime_error("Couldn't determin file size.");
		}
		bbe::List<unsigned char> fileBuffer;
		fileBuffer.resizeCapacityAndLength(fileSize);
		file.seekg(0);
		file.read((char*)fileBuffer.getRaw(), fileSize);
		file.close();
		return bbe::ByteBuffer(std::move(fileBuffer));
	}
	else
	{
		throw std::runtime_error("Failed to open file!");
	}
}

bool bbe::simpleFile::readBinaryFileIfChanged(const bbe::String& filepath, bbe::ByteBuffer& outContents, std::filesystem::file_time_type& inOutPreviousModify)
{
	if (std::filesystem::is_directory(filepath.getRaw()))
	{
		throw bbe::IllegalArgumentException();
	}

	std::filesystem::file_time_type currentModifyTime;
	try
	{
		currentModifyTime = std::filesystem::last_write_time(filepath.getRaw());
	}
	catch (std::filesystem::filesystem_error&)
	{
		// Probably the file is still in the write process, so we just report that nothing changed.
		return false;
	}
	if (currentModifyTime <= inOutPreviousModify && inOutPreviousModify != std::filesystem::file_time_type()) return false;

	try {
		outContents = readBinaryFile(filepath);
	}
	catch (std::runtime_error&)
	{
		// Probably the file is still in the write process, so we just report that nothing changed.
		return false;
	}
	inOutPreviousModify = currentModifyTime;

	return true;
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

void bbe::simpleFile::writeBinaryToFile(const bbe::String& filePath, bbe::ByteBuffer& buffer)
{
	std::ofstream file(filePath.getRaw(), std::ios_base::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file!");
	}
	std::copy(buffer.getRaw(), buffer.getRaw() + buffer.getLength(), std::ostreambuf_iterator<char>(file));
	file.close();
}

void bbe::simpleFile::appendStringToFile(const bbe::String& filePath, const bbe::String& stringToAppend)
{
	std::ofstream file(filePath.getRaw(), std::ofstream::app);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file!");
	}
	file << stringToAppend.getRaw();
	file.close();
}

void bbe::simpleFile::appendBinaryToFile(const bbe::String& filePath, bbe::ByteBuffer& buffer)
{
	std::ofstream file(filePath.getRaw(), std::ios::binary | std::ofstream::app);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file!");
	}
	std::copy(buffer.getRaw(), buffer.getRaw() + buffer.getLength(), std::ostreambuf_iterator<char>(file));
	file.close();
}

bool bbe::simpleFile::doesFileExist(const bbe::String& filePath)
{
	std::ifstream f(filePath.getRaw());
	return (bool)f;
}

void bbe::simpleFile::createDirectory(const bbe::String& path)
{
	if (!std::filesystem::is_directory(path.getRaw()) || !std::filesystem::exists(path.getRaw()))
	{
		std::filesystem::create_directories(path.getRaw());
	}
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

bbe::List<bbe::String> bbe::simpleFile::readLines(const bbe::String& filePath)
{
	std::ifstream file(filePath.getRaw());
	std::string line;
	bbe::List<bbe::String> retVal;
	while (std::getline(file, line))
	{
		retVal.add(line.data());
	}

	return retVal;
}

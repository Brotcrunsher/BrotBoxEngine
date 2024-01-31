#pragma once

#include "../BBE/List.h"
#include "../BBE/String.h"
#include "../BBE/ByteBuffer.h"
#include <fstream>
#include <filesystem>

namespace bbe
{
	namespace simpleFile
	{
		bbe::ByteBuffer readBinaryFile(const bbe::String &filepath);
		bool readBinaryFileIfChanged(const bbe::String& filepath, bbe::ByteBuffer& outContents, std::filesystem::file_time_type& inOutPreviousModify);

		bbe::List<float> readFloatArrFromFile(const bbe::String& filePath);
		void writeFloatArrToFile(const bbe::String &filePath, float *arr, size_t size);
		void writeStringToFile(const bbe::String& filePath, const bbe::String& stringToWrite);
		void writeBinaryToFile(const bbe::String& filePath, bbe::ByteBuffer& buffer);
		void appendStringToFile(const bbe::String& filePath, const bbe::String& stringToAppend);
		void appendBinaryToFile(const bbe::String& filePath, bbe::ByteBuffer& buffer);
		bool doesFileExist(const bbe::String& filePath);
		void createDirectory(const bbe::String& path);
		bbe::String readFile(const bbe::String& filePath);
		bbe::List<bbe::String> readLines(const bbe::String& filePath);

#ifdef WIN32
		bbe::String getUserName();
		bbe::String getAutoStartDirectory();
		bbe::String getExecutablePath();
		bbe::String getWorkingDirectory();
		void createLink(const bbe::String& from, const bbe::String& to, const bbe::String& workDir = "");
#endif
	}
}

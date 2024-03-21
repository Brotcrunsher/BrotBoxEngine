#pragma once

#include "../BBE/List.h"
#include "../BBE/String.h"
#include "../BBE/ByteBuffer.h"
#include <fstream>
#include <filesystem>

namespace bbe
{
	namespace backup
	{
		void setBackupPath(const bbe::String& path);
		bool isBackupPathSet();
		bbe::String backupFullPath(const bbe::String& path);
		void writeBinaryToFile(const bbe::String& filePath, const bbe::ByteBuffer& buffer);
		void createDirectory(const bbe::String& path);
		void appendBinaryToFile(const bbe::String& filePath, const bbe::ByteBuffer& buffer);
	}

	namespace simpleFile
	{
		bbe::ByteBuffer readBinaryFile(const bbe::String &filepath);
		bool readBinaryFileIfChanged(const bbe::String& filepath, bbe::ByteBuffer& outContents, std::filesystem::file_time_type& inOutPreviousModify);

		bbe::List<float> readFloatArrFromFile(const bbe::String& filePath);
		void writeFloatArrToFile(const bbe::String &filePath, const float *arr, size_t size);
		void writeFloatArrToFile(const bbe::String& filePath, const bbe::List<float>& data);
		void writeStringToFile(const bbe::String& filePath, const bbe::String& stringToWrite);
		void writeBinaryToFile(const bbe::String& filePath, const bbe::ByteBuffer& buffer);
		void appendStringToFile(const bbe::String& filePath, const bbe::String& stringToAppend);
		void appendBinaryToFile(const bbe::String& filePath, const bbe::ByteBuffer& buffer);
		bool doesFileExist(const bbe::String& filePath);
		void createDirectory(const bbe::String& path);
		bool deleteFile(const bbe::String& path);
		bbe::String readFile(const bbe::String& filePath);
		bbe::List<bbe::String> readLines(const bbe::String& filePath);

#ifndef __EMSCRIPTEN__
		void forEachFile(const bbe::String& filePath, const std::function<void(const bbe::String&)>& func);
#endif

#ifdef WIN32
		bbe::String getUserName();
		bbe::String getAutoStartDirectory();
		bbe::String getExecutablePath();
		bbe::String getWorkingDirectory();
		void createLink(const bbe::String& from, const bbe::String& to, const bbe::String& workDir = "");
		void executeBatchFile(const bbe::String& path);
		bool showOpenDialog(bbe::String& outPath);
		bool showSaveDialog(bbe::String& outPath, const bbe::String& defaultExtension);
#endif
	}
}

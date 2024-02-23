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
	bbe::ByteBuffer bb = readBinaryFile(filePath);
	auto bbs = bb.getSpan();
	bbe::List<float> retVal;
	while (bbs.hasMore())
	{
		float val = 0.f;
		bbs.read(val);
		retVal.add(val);
	}
	return retVal;
}

void bbe::simpleFile::writeFloatArrToFile(const bbe::String & filePath, const float * arr, size_t size)
{	
	std::ofstream file(filePath.getRaw(), std::ios_base::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file!");
	}
	const char* carr = (const char*)arr;
	std::copy(carr, carr + (sizeof(float) * size), std::ostreambuf_iterator<char>(file));
	file.close();
}

void bbe::simpleFile::writeFloatArrToFile(const bbe::String& filePath, const bbe::List<float>& data)
{
	writeFloatArrToFile(filePath, data.getRaw(), data.getLength());
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

void bbe::simpleFile::writeBinaryToFile(const bbe::String& filePath, const bbe::ByteBuffer& buffer)
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

void bbe::simpleFile::appendBinaryToFile(const bbe::String& filePath, const bbe::ByteBuffer& buffer)
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

bool bbe::simpleFile::deleteFile(const bbe::String& path)
{
	return std::filesystem::remove(path.getRaw());
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

void bbe::simpleFile::forEachFile(const bbe::String& filePath, const std::function<void(const bbe::String&)>& func)
{
	for (const auto& f : std::filesystem::directory_iterator(filePath.getRaw()))
	{
		func(f.path().c_str());
	}
}

#ifdef WIN32
#include "windows.h"
#include "winnls.h"
#include "shobjidl.h"
#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"

bbe::String bbe::simpleFile::getUserName()
{
	TCHAR buffer[256];
	DWORD bufferSize = sizeof(buffer) / sizeof(TCHAR);
	if (GetUserName(buffer, &bufferSize))
	{
		return bbe::String(buffer);
	}
	else
	{
		throw std::runtime_error("Error getting user name.");
	}
}

bbe::String bbe::simpleFile::getAutoStartDirectory()
{
	return "C:/Users/" + getUserName() + "/AppData/Roaming/Microsoft/Windows/Start Menu/Programs/Startup/";
}

bbe::String bbe::simpleFile::getExecutablePath()
{
	TCHAR buffer[1024] = {};
	GetModuleFileName(NULL, buffer, sizeof(buffer) / sizeof(TCHAR));
	return bbe::String(buffer);
}

bbe::String bbe::simpleFile::getWorkingDirectory()
{
	TCHAR buffer[1024] = {};
	GetCurrentDirectory(sizeof(buffer) / sizeof(TCHAR), buffer);
	return bbe::String(buffer);
}

void bbe::simpleFile::createLink(const bbe::String& from, const bbe::String& to, const bbe::String& workDir)
{
	// More or less from the microsoft examples, adjusted to use BBE Types and coding style.
	// See: https://learn.microsoft.com/en-us/windows/win32/shell/links

	IShellLink* psl;
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		psl->SetPath(to.getRaw());
		if(workDir.getLength() > 0) psl->SetWorkingDirectory(workDir.getRaw());

		IPersistFile* ppf;
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
		if (SUCCEEDED(hres))
		{
			WCHAR wsz[1024];
			MultiByteToWideChar(CP_ACP, 0, from.getRaw(), -1, wsz, sizeof(wsz) / sizeof(WCHAR));
			hres = ppf->Save(wsz, TRUE);
			ppf->Release();
		}
		psl->Release();
	}
	if (FAILED(hres))
	{
		throw std::runtime_error("Failed to create link.");
	}
}
void bbe::simpleFile::executeBatchFile(const bbe::String& path)
{
	// TODO: This is dumb.
	system(path.getRaw());

	// TODO: Why doesn't this work?
	//PROCESS_INFORMATION pi = {};
	//STARTUPINFOA si = {};
	//si.cb = sizeof(si);
	//CreateProcess(
	//	"cmd.exe",
	//	("/c " + path).getRaw(),
	//	NULL,
	//	NULL,
	//	FALSE,
	//	0,
	//	NULL,
	//	NULL,
	//	&si,
	//	&pi
	//);
	//CloseHandle(pi.hProcess);
	//CloseHandle(pi.hThread);
}
#endif

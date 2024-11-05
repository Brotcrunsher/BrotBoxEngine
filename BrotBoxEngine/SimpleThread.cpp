#include "BBE/SimpleThread.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

void bbe::simpleThread::setName(std::thread& thread, const char* name)
{
#ifdef WIN32
	size_t length = std::mbstowcs(nullptr, name, 0) + 1;
	wchar_t* wName = new wchar_t[length];
	std::mbstowcs(wName, name, length);
	::SetThreadDescription(thread.native_handle(), wName);
	delete[] wName;
#else
#pragma message("bbe::simpleThread::setName not supported on this platform!")
#endif
}

#include "stdafx.h"
#include "BBE/UtilDebug.h"
#include <iostream>

#ifdef __linux__
	#include <execinfo.h>
	#include <unistd.h>
#endif

void bbe::debugBreak()
{
	std::cout << "debugBreak() was triggered!" << std::endl;
	#if defined(_MSC_VER) && defined(_WIN32)
		__debugbreak();
	#elif defined(__linux__)
		/*void *stackTrace[1024];
		std::size_t size = backtrace(stackTrace, sizeof(stackTrace));
		backtrace_symbols_fd(stackTrace, size, STDERR_FILENO); 
		__builtin_trap();*/
	#endif
}

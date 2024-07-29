#pragma once

#include <cstdint>

namespace bbe
{
	enum class Error
	{
		AlreadyCreated = 1000,
		AlreadyStarted = 2000,
		AlreadyUploaded = 3000,
		NotInitialized = 4000,

		NoSuchKeycode = 5000,
		NoSuchMouseButton = 6000,

		IllegalState = 7000,
		OutOfMemory = 8000,

		KeyAlreadyUsed = 9000,

		IllegalArgument = 10000,
		ContainerEmpty = 11000,

		IllegalIndex = 12000,

		NotImplemented = 13000,

		Decode = 14000,
		Unknown = 15000,
		FormatNotSupported = 16000,

		NotStartOfUtf8 = 17000,
		NullPointer = 18000,
		UnexpectedEndOfString = 19000,
		NotAUtf8Char = 20000,

		FatalError = 21000,
		Segfault = 22000,
		UnhandledException = 23000,
	};

	[[noreturn]] void CrashImpl(const char* file, int32_t line, const char* function, Error error);
	[[noreturn]] void CrashImpl(const char* file, int32_t line, const char* function, Error error, const char* msg);
#define Crash(...) CrashImpl(__FILE__, __LINE__, __func__, __VA_ARGS__)
}

#ifndef NDEBUG
#define BBE_TRY_RELEASE
#else
#define BBE_TRY_RELEASE try
#endif

#ifndef NDEBUG
#define BBE_CATCH_RELEASE if(0)
#else
#define BBE_CATCH_RELEASE catch(...)
#endif

#pragma once

#include <cstdint>

namespace bbe
{
	enum class Error
	{
		AlreadyCreated,
		AlreadyStarted,
		AlreadyUploaded,
		NotInitialized,

		NoSuchKeycode,
		NoSuchMouseButton,

		IllegalState,
		OutOfMemory,

		KeyAlreadyUsed,

		IllegalArgument,
		ContainerEmpty,

		IllegalIndex,

		NotImplemented,

		Decode,
		Unknown,
		FormatNotSupported,

		NotStartOfUtf8,
		NullPointer,
		UnexpectedEndOfString,
		NotAUtf8Char,

		FatalError,
	};

	[[noreturn]] void CrashImpl(const char* file, int32_t line, Error error);
	[[noreturn]] void CrashImpl(const char* file, int32_t line, Error error, const char* msg);
#define Crash(...) CrashImpl(__FILE__, __LINE__, __VA_ARGS__)
}

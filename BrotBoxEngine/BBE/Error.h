#pragma once

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

	[[noreturn]] void Crash(Error error);
	[[noreturn]] void Crash(Error error, const char* msg);
}

#include "../BBE/Error.h"
#include "../BBE/BrotTime.h"
#include "../BBE/Logging.h"
#include "../BBE/SimpleFile.h"
#include "../BBE/String.h"
#include "../BBE/UtilDebug.h"
#include <cstdlib>
#include <sstream>
// <stacktrace> can exist as a stub without a linkable implementation. Enable only when the library claims support
// (or MSVC, where the feature is normally complete), so Linux/Clang stays link-clean while still using stacktrace
// on toolchains where it is actually usable.
#if __has_include(<stacktrace>) && (defined(_MSC_VER) || (defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011L))
#include <stacktrace>
#define BBE_HAS_STD_STACKTRACE 1
#else
#define BBE_HAS_STD_STACKTRACE 0
#ifdef _MSC_VER
#pragma warning("Stacktrace lib is not present!")
#endif
#endif

const char *bbe::toString(Error err)
{
#define TO_STR(x) \
	if (err == Error::x) return #x;
	TO_STR(AlreadyCreated);
	TO_STR(AlreadyStarted);
	TO_STR(AlreadyUploaded);
	TO_STR(NotInitialized);
	TO_STR(NoSuchKeycode);
	TO_STR(NoSuchMouseButton);
	TO_STR(IllegalState);
	TO_STR(OutOfMemory);
	TO_STR(KeyAlreadyUsed);
	TO_STR(IllegalArgument);
	TO_STR(ContainerEmpty);
	TO_STR(IllegalIndex);
	TO_STR(NotImplemented);
	TO_STR(Decode);
	TO_STR(Unknown);
	TO_STR(FormatNotSupported);
	TO_STR(NotStartOfUtf8);
	TO_STR(NullPointer);
	TO_STR(UnexpectedEndOfString);
	TO_STR(NotAUtf8Char);
	TO_STR(FatalError);
	TO_STR(Segfault);
	TO_STR(UnhandledException);
	TO_STR(VectoredException);
	TO_STR(DebugBreakInRelease);
	TO_STR(Terminate);
#undef TO_STR

	return "Missing translation";
}

[[noreturn]] void bbe::CrashImpl(const char *file, int32_t line, const char *function, Error error, bool callDebugBreak)
{
	CrashImpl(file, line, function, error, "no message", callDebugBreak);
}

[[noreturn]] void bbe::CrashImpl(const char *file, int32_t line, const char *function, Error error, const char *msg, bool callDebugBreak)
{
#ifndef NDEBUG
	if (callDebugBreak) debugBreak();
#endif

	const bbe::String time = bbe::TimePoint().toString();

	bbe::String string;
	string += "###################\n";
	string += "#                 #\n";
	string += "#   !!!CRASH!!!   #\n";
	string += "#                 #\n";
	string += "###################\n";
	string += "\n";
	string += "Time:  " + time;
	string += "Crash: " + bbe::String((int)error) + " : " + toString(error);
	string += "\n";
	string += "Msg:   " + bbe::String(msg);
	string += "\n";
	string += "Where: " + bbe::String(file) + "(" + line + ")\n";
	string += "       @" + bbe::String(function) + "\n\n";
	string += "Stacktrace:\n";
#if BBE_HAS_STD_STACKTRACE
	std::ostringstream stacktraceStream;
	stacktraceStream << std::stacktrace::current();
	string += stacktraceStream.str();
#else
	string += "Stacktrace lib is not present!";
#endif

	string += "\n\nLog:\n";

	const bbe::ConcurrentList<bbe::String> &log = bbe::logging::getLog();
	log.lock(); // No lock_guard to keep stuff around this crash handler as simple as possible. We might be in a very vulnerable and unreliable state here.
	for (size_t i = 0; i < log.getLength(); i++)
	{
		string += log.getUnprotected(i); // Making sure that we don't recursively crash because of a negative lock count within the ConcurrentList.
		string += "\n";
	}
	log.unlock();
#ifndef __EMSCRIPTEN__
	bbe::simpleFile::createDirectory("CrashLogs");
	bbe::simpleFile::writeStringToFile("CrashLogs/" + bbe::String(std::time(nullptr)) + ".txt", string);
#else
	std::cout << string.getRaw() << std::endl;
#endif

	std::abort();
}

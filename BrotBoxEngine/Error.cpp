#include "../BBE/Error.h"
#include "../BBE/UtilDebug.h"
#include "../BBE/BrotTime.h"
#include "../BBE/Logging.h"
#include "../BBE/SimpleFile.h"
#include "../BBE/String.h"
#include <cstdlib>
#if __has_include(<stacktrace>)
#include <stacktrace>
#else
#pragma warning("Stacktrace lib is not present!")
#endif

[[noreturn]] void bbe::CrashImpl(const char* file, int32_t line, Error error)
{
	CrashImpl(file, line, error, "no message");
}

[[noreturn]] void bbe::CrashImpl(const char* file, int32_t line, Error error, const char* msg)
{
	debugBreak();


	const bbe::String time = bbe::TimePoint().toString();

	bbe::String string;
	string += "###################\n";
	string += "#                 #\n";
	string += "#   !!!CRASH!!!   #\n";
	string += "#                 #\n";
	string += "###################\n";
	string += "\n";
	string += "Time:  " + time;
	string += "Crash: " + bbe::String((int)error);
	string += "\n";
	string += "Msg:   " + bbe::String(msg);
	string += "\n";
	string += "Where: " + bbe::String(file) + "(" + line + ")\n\n";
	string += "Stacktrace:\n";
#ifdef WIN32 // TODO: GCC14 does support this! But it's currently hard to find a stable, easy to install version of it on debian and ubuntu...
	string += std::to_string(std::stacktrace::current());
#else
	string += "Stacktrace lib is not present!";
#endif
	BBELOGLN(string);

	bbe::simpleFile::createDirectory("CrashLogs");
	bbe::simpleFile::writeStringToFile("CrashLogs/" + bbe::String(std::time(nullptr)) + ".txt", string);


	std::abort();
}

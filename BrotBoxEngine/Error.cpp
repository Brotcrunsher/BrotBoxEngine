#include "../BBE/Error.h"
#include "../BBE/UtilDebug.h"
#include "../BBE/BrotTime.h"
#include "../BBE/Logging.h"
#include "../BBE/SimpleFile.h"
#include "../BBE/String.h"
#include <cstdlib>
#if __has_include(<stacktrace>)
#include <stacktrace>
#endif

[[noreturn]] void bbe::Crash(Error error)
{
	Crash(error, "no message");
}

[[noreturn]] void bbe::Crash(Error error, const char* msg)
{
	debugBreak();


	const bbe::String time = bbe::TimePoint().toString();

	bbe::String string;
	string += "###################\n";
	string += "#                 #\n";
	string += "#   !!!CRASH!!!   #\n";
	string += "#   bbe::Crash    #\n";
	string += "#                 #\n";
	string += "###################\n";
	string += "\n";
	string += "Time:   " + time;
	string += "Crash: " + bbe::String((int)error);
	string += "\n";
	string += "Msg: " + bbe::String(msg);
	string += "\n";
	string += "Stacktrace:\n";
#if __has_include(<stacktrace>)
	string += std::to_string(std::stacktrace::current());
#else
	string += "Stacktrace lib is not present!";
#endif
	BBELOGLN(string);

	bbe::simpleFile::createDirectory("CrashLogs");
	bbe::simpleFile::writeStringToFile("CrashLogs/" + bbe::String(std::time(nullptr)) + ".txt", string);


	std::abort();
}

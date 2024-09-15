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

const char* bbe::toString(Error err)
{
#define TO_STR(x) if(err == Error::x) return #x;
	TO_STR(AlreadyCreated       );
	TO_STR(AlreadyStarted       );
	TO_STR(AlreadyUploaded      );
	TO_STR(NotInitialized       );
	TO_STR(NoSuchKeycode        );
	TO_STR(NoSuchMouseButton    );
	TO_STR(IllegalState         );
	TO_STR(OutOfMemory          );
	TO_STR(KeyAlreadyUsed       );
	TO_STR(IllegalArgument      );
	TO_STR(ContainerEmpty       );
	TO_STR(IllegalIndex         );
	TO_STR(NotImplemented       );
	TO_STR(Decode               );
	TO_STR(Unknown              );
	TO_STR(FormatNotSupported   );
	TO_STR(NotStartOfUtf8       );
	TO_STR(NullPointer          );
	TO_STR(UnexpectedEndOfString);
	TO_STR(NotAUtf8Char         );
	TO_STR(FatalError           );
	TO_STR(Segfault             );
	TO_STR(UnhandledException   );
	TO_STR(VectoredException    );
#undef TO_STR

	return "Missing translation";
}

[[noreturn]] void bbe::CrashImpl(const char* file, int32_t line, const char* function, Error error, bool callDebugBreak)
{
	CrashImpl(file, line, function, error, "no message", callDebugBreak);
}

[[noreturn]] void bbe::CrashImpl(const char* file, int32_t line, const char* function, Error error, const char* msg, bool callDebugBreak)
{
	if(callDebugBreak) debugBreak();


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
#ifdef WIN32 // TODO: GCC14 does support this! But it's currently hard to find a stable, easy to install version of it on debian and ubuntu...
	string += std::to_string(std::stacktrace::current());
#else
	string += "Stacktrace lib is not present!";
#endif

	string += "\n\nLog:\n";

	const bbe::List<bbe::String>& log = bbe::logging::getLog();
	for (size_t i = 0; i < log.getLength(); i++)
	{
		string += log[i];
		string += "\n";
	}
#ifndef __EMSCRIPTEN__
	bbe::simpleFile::createDirectory("CrashLogs");
	bbe::simpleFile::writeStringToFile("CrashLogs/" + bbe::String(std::time(nullptr)) + ".txt", string);
#else
	std::cout << string.getRaw() << std::endl;
#endif


	std::abort();
}

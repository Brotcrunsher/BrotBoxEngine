#include "BBE/Logging.h"

static bbe::List<bbe::String> logLines = { "" };

void bbe::logging::INTERNAL::partialLog(const char* c)
{
	logLines.last() += c;
}

void bbe::logging::INTERNAL::fullLog(const char* c)
{
	partialLog(c);
	logLines.add("");
}

const bbe::List<bbe::String>& bbe::logging::getLog()
{
	return logLines;
}

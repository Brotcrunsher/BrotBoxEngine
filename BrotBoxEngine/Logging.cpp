#include "BBE/Logging.h"

static bbe::ConcurrentList<bbe::String> logLines = { "" };

void bbe::logging::INTERNAL::partialLog(const char* c)
{
	std::lock_guard _(logLines);
	logLines.getUnderlying().last() += c;
}

void bbe::logging::INTERNAL::fullLog(const char* c)
{
	partialLog(c);
	logLines.add("");
}

const bbe::ConcurrentList<bbe::String>& bbe::logging::getLog()
{
	return logLines;
}

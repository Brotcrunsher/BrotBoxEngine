#include "BBE/Logging.h"

static bbe::List<bbe::String> logLines;
static bbe::String currentLine;

void bbe::logging::INTERNAL::partialLog(const char* c)
{
	currentLine += c;
}

void bbe::logging::INTERNAL::fullLog(const char* c)
{
	if (currentLine.getLength() == 0)
	{
		logLines.add(c);
	}
	else
	{
		currentLine += c;
		logLines.add(currentLine);
		currentLine = "";
	}
}

const bbe::List<bbe::String>& bbe::logging::getLog()
{
	return logLines;
}

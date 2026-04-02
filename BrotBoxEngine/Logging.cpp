#include "BBE/Logging.h"
#include <chrono>
#include <ctime>
#include <cstdio>

static bbe::ConcurrentList<bbe::String> logLines = { "" };

void bbe::logging::INTERNAL::partialLog(const char *c)
{
	std::lock_guard _(logLines);
	logLines.getUnderlying().last() += c;
}

void bbe::logging::INTERNAL::fullLog(const char *c)
{
	partialLog(c);
	logLines.add("");
}

std::string bbe::logging::INTERNAL::currentTimestamp()
{
	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	std::tm *tm = std::localtime(&t);
	char buf[16];
	std::snprintf(buf, sizeof(buf), "[%02d:%02d:%02d:%03d] ",
		tm->tm_hour, tm->tm_min, tm->tm_sec, static_cast<int>(ms.count()));
	return buf;
}

const bbe::ConcurrentList<bbe::String> &bbe::logging::getLog()
{
	return logLines;
}

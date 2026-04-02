#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include "../BBE/List.h"
#include "../BBE/String.h"

namespace bbe
{
	namespace logging
	{
		namespace INTERNAL
		{
			void partialLog(const char *c);
			void fullLog(const char *c);
			std::string currentTimestamp();
		}

		const bbe::ConcurrentList<bbe::String> &getLog();
	}
}

#define BBELOG(out)                                                          \
	do                                                                      \
	{                                                                       \
		std::string bbelog_ts_ = bbe::logging::INTERNAL::currentTimestamp(); \
		std::stringstream stream;                                           \
		stream << bbelog_ts_ << out;                                        \
		std::cout << stream.str();                                          \
		bbe::logging::INTERNAL::partialLog(stream.str().c_str());           \
	} while (false)

#define BBELOGLN(out)                                                       \
	do                                                                      \
	{                                                                       \
		std::string bbelog_ts_ = bbe::logging::INTERNAL::currentTimestamp(); \
		std::stringstream stream;                                           \
		stream << bbelog_ts_ << out;                                        \
		std::cout << stream.str() << std::endl;                             \
		bbe::logging::INTERNAL::fullLog(stream.str().c_str());              \
	} while (false)

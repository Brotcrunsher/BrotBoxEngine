#pragma once

#include <iostream>
#include <sstream>
#include "../BBE/List.h"
#include "../BBE/String.h"

namespace bbe
{
	namespace logging
	{
		namespace INTERNAL
		{
			void partialLog(const char* c);
			void fullLog(const char* c);
		}

		const bbe::List<bbe::String>& getLog();
	}
}

#define BBELOG(out) \
	do { \
		std::stringstream stream; \
		stream << out; \
		std::cout << stream.str(); \
		bbe::logging::INTERNAL::partialLog(stream.str().c_str()); \
	} while(false)

#define BBELOGLN(out) \
	do { \
		std::stringstream stream; \
		stream << out; \
		std::cout << stream.str() << std::endl; \
		bbe::logging::INTERNAL::fullLog(stream.str().c_str()); \
	} while(false)

#pragma once

#include <cstdint>

namespace bbe
{
	void debugBreakImpl(const char* file, int32_t line);

#define debugBreak() bbe::debugBreakImpl(__FILE__, __LINE__)
}

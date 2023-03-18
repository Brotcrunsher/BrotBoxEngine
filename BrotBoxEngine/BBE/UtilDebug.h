#pragma once

#include <cstdint>

namespace bbe
{
	void debugBreakImpl(const char* file, int32_t line);

	// Note: This must NOT contain the bbe namespace! This way, the user has to add the bbe::, which is closer to what we want.
#define debugBreak() debugBreakImpl(__FILE__, __LINE__)
}

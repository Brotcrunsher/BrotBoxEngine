#pragma once

#include "../BBE/List.h"
#include "../BBE/String.h"
#include <fstream>

namespace bbe
{
	namespace simpleFile
	{
		bbe::List<char> readBinaryFile(const bbe::String &filepath);
	}
}

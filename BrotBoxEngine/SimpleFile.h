#pragma once

#include "List.h"
#include "String.h"
#include <fstream>

namespace bbe
{
	namespace simpleFile
	{
		bbe::List<char> readBinaryFile(const bbe::String &filepath);
	}
}
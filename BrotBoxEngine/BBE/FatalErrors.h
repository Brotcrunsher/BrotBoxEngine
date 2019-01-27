#pragma once

#include "../BBE/Exceptions.h"
#include "../BBE/String.h"

namespace bbe
{
	namespace INTERNAL
	{
		void triggerFatalError(const bbe::String &msg);
		void triggerFatalError(const char* msg);
	}
}

#include "BBE/FatalErrors.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

void bbe::INTERNAL::triggerFatalError(const bbe::String &msg)
{
	triggerFatalError(msg.getRaw());
}

void bbe::INTERNAL::triggerFatalError(const char * msg)
{
	std::cout << "FATAL ERROR: " << msg << std::endl;
#ifdef _WIN32
	MessageBox(nullptr, msg, "FATAL ERROR", MB_OK);
#endif
	throw FatalError(msg);
}

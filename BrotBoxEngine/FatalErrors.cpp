#include "BBE/FatalErrors.h"

#ifdef _WIN32
#include <windows.h>
#endif

void bbe::INTERNAL::triggerFatalError(const bbe::String &msg)
{
	triggerFatalError(msg.getRaw());
}

void bbe::INTERNAL::triggerFatalError(const char * msg)
{
#ifdef _WIN32
	MessageBox(nullptr, msg, "FATAL ERROR", MB_OK);
#endif
	throw FatalError(msg);
}

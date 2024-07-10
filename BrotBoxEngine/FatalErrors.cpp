#include "BBE/FatalErrors.h"
#include "BBE/Logging.h"
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
	// TODO: This whole triggerFatalError stuff is kinda obsolete from the introduction of bbe::Crash.
	BBELOGLN("FATAL ERROR: " << msg);
#ifdef _WIN32
	//MessageBox(nullptr, msg, "FATAL ERROR", MB_OK);
#endif
	bbe::Crash(bbe::Error::FatalError);
}

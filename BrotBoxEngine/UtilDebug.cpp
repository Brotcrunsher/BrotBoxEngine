#include "stdafx.h"
#include "BBE/UtilDebug.h"

void bbe::debugBreak()
{
	#ifdef _MSC_VER
		__debugbreak();
	#else
		//Todo find gcc equivalent to __debugbreak()
	#endif
}

#pragma once

#include <future>
#include "../BBE/String.h"

#ifndef BBE_NO_AUDIO
#include "../BBE/Sound.h"
#endif

namespace bbe
{
	namespace simplePython
	{
		void interpret(const bbe::String &code);
#ifndef BBE_NO_AUDIO
		bbe::Sound gtts(const bbe::String &text, const bbe::String &lang);
		std::future<bbe::Sound> gttsAsync(const bbe::String &text, const bbe::String &lang);
#endif
	}
}

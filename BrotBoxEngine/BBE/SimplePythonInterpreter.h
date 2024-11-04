#pragma once

#include <future>
#include "../BBE/String.h"
#include "../BBE/Sound.h"

namespace bbe
{
	namespace simplePython
	{
		void interpret(const bbe::String& code);
		bbe::Sound gtts(const bbe::String& text, const bbe::String& lang);
		std::future<bbe::Sound> gttsAsync(const bbe::String& text, const bbe::String& lang);
	}
}

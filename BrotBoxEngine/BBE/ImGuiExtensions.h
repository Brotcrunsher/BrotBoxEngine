#pragma once

#include "imgui.h"
#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/Game.h"
#include "../BBE/BrotTime.h"

namespace ImGui
{
	namespace bbe
	{
		namespace INTERNAL
		{
			void setActiveGame(::bbe::Game* game);
		}

		void tooltip(const char* text);
		void tooltip(const ::bbe::String& text);
		bool combo(const char* label, const ::bbe::List<::bbe::String>& selections, int32_t& selection);
		bool clickableText(const char* fmt, ...);
		bool securityButton(const char* text);
		bool datePicker(const char* label, ::bbe::TimePoint* time);
	}
}

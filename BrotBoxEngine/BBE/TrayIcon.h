#pragma once

#ifdef _WIN32
#include "Game.h"

#ifndef _WINDEF_
// Forward declare it to avoid including the whole windows header everywhere.
class HICON__;
typedef HICON__* HICON;
#endif

namespace bbe
{
	namespace TrayIcon
	{
		void init(bbe::Game* game, const char* tooltip, HICON icon);
		void addPopupItem(const char* title, std::function<void()> callback);
		bool isVisible();
		void setIcon(HICON icon);
	}
}
#endif

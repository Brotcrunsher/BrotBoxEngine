#pragma once

#include "Game.h"
#include <cstddef>
#include <cstdint>
#include <functional>

#ifdef _WIN32
#ifndef _WINDEF_
// Forward declare it to avoid including the whole windows header everywhere.
class HICON__;
typedef HICON__* HICON;
#endif
#endif

namespace bbe
{
	namespace TrayIcon
	{
#ifdef _WIN32
		using IconHandle = HICON;
#elif defined(__linux__)
		struct LinuxIconHandle;
		using IconHandle = LinuxIconHandle*;

		IconHandle createIcon(int width, int height, const uint32_t* argbPixels, size_t pixelCount);
#else
		using IconHandle = void*;
#endif

		void init(bbe::Game* game, const char* tooltip, IconHandle icon);
		void addPopupItem(const char* title, std::function<void()> callback);
		bool isVisible();
		void setIcon(IconHandle icon);
		void update();
	}
}

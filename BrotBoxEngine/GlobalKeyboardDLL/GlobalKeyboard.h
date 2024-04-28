#pragma once

#include <array>

#include "../BBE/Keyboard.h"
#include "../BBE/KeyboardKeys.h"

#ifndef _WINDEF_
// Forward declare it to avoid including the whole windows header everywhere.
class HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;
typedef HINSTANCE HMODULE;
class HHOOK__;
typedef HHOOK__* HHOOK;
#endif

namespace bbe
{
	class GlobalKeyboard : public bbe::Keyboard
	{
	private:
		HMODULE hmod = nullptr;
		HHOOK hook = nullptr;
		void* getNextEvent = nullptr;
		bool hooked = false;

	public:
		GlobalKeyboard() = default;
		~GlobalKeyboard();

		void init();
		void uninit();
		bool isInit() const;

		GlobalKeyboard(const GlobalKeyboard&) = delete;
		GlobalKeyboard(GlobalKeyboard&&) = delete;
		GlobalKeyboard& operator=(const GlobalKeyboard&) = delete;
		GlobalKeyboard& operator=(GlobalKeyboard&&) = delete;

		void update();
	};
}

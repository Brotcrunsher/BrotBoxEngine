#pragma once

#include <array>
#include "../BBE/KeyboardKeys.h"

namespace bbe
{
	class Keyboard
	{
		friend class Window;
		friend class Game;
	protected:
		struct KeyState
		{
			bool down    = false;
			bool pressed = false;
		};
		std::array<KeyState, (int)Key::LAST + 1> m_pkeysNextFrame = {};
		std::array<KeyState, (int)Key::LAST + 1> m_pkeysThisFrame = {};

		void update();

	public:
		Keyboard() = default;

		bool isKeyDown(bbe::Key key, bool checkValid = true);
		bool isKeyUp(bbe::Key key, bool checkValid = true);
		bool isKeyPressed(bbe::Key key, bool checkValid = true);

		void INTERNAL_press(bbe::Key key);
		void INTERNAL_release(bbe::Key key);
	};
}

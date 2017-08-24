#pragma once

#include "../BBE/KeyboardKeys.h"

namespace bbe
{
	class Keyboard
	{
		friend class Window;
		friend class Game;
	private:
		bool m_pkeysNextFrame[Key::LAST + 1];
		bool m_pkeysThisFrame[Key::LAST + 1];
		bool m_pkeysLastFrame[Key::LAST + 1];

		

		void update();

	public:
		Keyboard();

		bool isKeyDown(bbe::Key key);
		bool isKeyUp(bbe::Key key);

		bool wasKeyDownLastFrame(bbe::Key key);
		bool wasKeyUpLastFrame(bbe::Key key);

		bool isKeyPressed(bbe::Key key);

		void INTERNAL_press(bbe::Key key);
		void INTERNAL_release(bbe::Key key);
	};
}

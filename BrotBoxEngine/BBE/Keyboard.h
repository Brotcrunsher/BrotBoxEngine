#pragma once

#include "../BBE/KeyboardKeys.h"

namespace bbe
{
	class Keyboard
	{
		friend class Window;
		friend class Game;
	private:
		bool m_keysNextFrame[KEY_LAST + 1];
		bool m_keysThisFrame[KEY_LAST + 1];
		bool m_keysLastFrame[KEY_LAST + 1];

		

		void update();

	public:
		Keyboard();

		bool isKeyDown(int keyCode);
		bool isKeyUp(int keyCode);

		bool wasKeyDownLastFrame(int keyCode);
		bool wasKeyUpLastFrame(int keyCode);

		bool isKeyPressed(int keyCode);

		void INTERNAL_press(int keyCode);
		void INTERNAL_release(int keyCode);
	};
}

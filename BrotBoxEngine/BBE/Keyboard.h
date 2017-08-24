#pragma once

#include "../BBE/KeyboardKeys.h"

namespace bbe
{
	class Keyboard
	{
		friend class Window;
		friend class Game;
	private:
		bool m_pkeysNextFrame[Keys::LAST + 1];
		bool m_pkeysThisFrame[Keys::LAST + 1];
		bool m_pkeysLastFrame[Keys::LAST + 1];

		

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

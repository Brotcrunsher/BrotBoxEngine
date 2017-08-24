#pragma once

#include "../BBE/MouseButtons.h"


namespace bbe
{
	class Vector2;

	class Mouse
	{
		friend class Window;
		friend class Game;

	private:
		float m_mouseNextFrameX    = 0;
		float m_mouseNextFrameY    = 0;
		float m_mouseCurrentFrameX = 0;
		float m_mouseCurrentFrameY = 0;
		float m_mouseLastFrameX    = 0;
		float m_mouseLastFrameY    = 0;
		bool m_pButtonsNextFrame[(int)MouseButton::LAST + 1];
		bool m_pButtonsThisFrame[(int)MouseButton::LAST + 1];
		bool m_pButtonsLastFrame[(int)MouseButton::LAST + 1];

		void update();

	public:
		Mouse();


		float getMouseX();
		float getMouseY();
		Vector2 getMouse();

		float getMouseXDelta();
		float getMouseYDelta();
		Vector2 getMouseDelta();

		bool isButtonDown(bbe::MouseButton button);
		bool isButtonUp(bbe::MouseButton button);

		bool wasButtonDownLastFrame(bbe::MouseButton button);
		bool wasButtonUpLastFrame(bbe::MouseButton button);

		bool isButtonPressed(bbe::MouseButton button);

		void INTERNAL_moveMouse(float x, float y);
		void INTERNAL_press(MouseButton button);
		void INTERNAL_release(MouseButton button);
	};
}
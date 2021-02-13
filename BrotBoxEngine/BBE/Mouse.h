#pragma once

#include "../BBE/MouseButtons.h"


namespace bbe
{
	template<typename T> class Vector2_t;
	using Vector2 = Vector2_t<float>;

	class Mouse
	{
		friend class Window;
		friend class Game;

	private:
		float m_mouseNextFrameX          = 0;
		float m_mouseNextFrameY          = 0;
		float m_mouseCurrentFrameX       = 0;
		float m_mouseCurrentFrameY       = 0;
		float m_mouseLastFrameX          = 0;
		float m_mouseLastFrameY          = 0;
		float m_mouseCurrentFrameXGlobal = 0;
		float m_mouseCurrentFrameYGlobal = 0;
		float m_mouseLastFrameXGlobal    = 0;
		float m_mouseLastFrameYGlobal    = 0;
		float m_mouseScrollX             = 0;
		float m_mouseScrollY             = 0;
		float m_mouseScrollXNext         = 0;
		float m_mouseScrollYNext         = 0;
		bool m_pButtonsNextFrame[(int)MouseButton::LAST + 1];
		bool m_pButtonsThisFrame[(int)MouseButton::LAST + 1];
		bool m_pButtonsLastFrame[(int)MouseButton::LAST + 1];

		void update(float globalMousePosX, float globalMousePosY);

	public:
		Mouse();


		float getMouseX() const;
		float getMouseY() const;
		Vector2 getMouse() const;

		float getMouseXGlobal() const;
		float getMouseYGlobal() const;
		Vector2 getMouseGlobal() const;

		float getScrollX();
		float getScrollY();
		Vector2 getScroll();

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
		void INTERNAL_scroll(float x, float y);
	};
}
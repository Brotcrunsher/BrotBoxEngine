#pragma once



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

		void update();

	public:

		float getMouseX();
		float getMouseY();
		Vector2 getMouse();

		float getMouseXDelta();
		float getMouseYDelta();
		Vector2 getMouseDelta();

		void INTERNAL_moveMouse(float x, float y);
	};
}
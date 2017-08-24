#pragma once

#include "../BBE/GameTime.h"
#include "../BBE/CursorMode.h"
#include "../BBE/KeyboardKeys.h"
#include "../BBE/MouseButtons.h"

namespace bbe
{
	class Window;
	class PrimitiveBrush2D;
	class PrimitiveBrush3D;

	class Game
	{
	private:
		Window*  m_pwindow  = nullptr;
		bool     m_started  = false;
		GameTime m_gameTime;

	public:
		Game();
		~Game();

		Game(const Game&)            = delete;
		Game(Game&&)                 = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&)      = delete;

		void start(int windowWidth, int windowHeight, const char* title);

		virtual void onStart()                            = 0;
		virtual void update(float timeSinceLastFrame)     = 0;
		virtual void draw3D(bbe::PrimitiveBrush3D &brush) = 0;
		virtual void draw2D(bbe::PrimitiveBrush2D &brush) = 0;
		virtual void onEnd()                              = 0;

		bool isKeyDown(bbe::Key key);
		bool isKeyUp(bbe::Key key);
		bool wasKeyDownLastFrame(bbe::Key key);
		bool wasKeyUpLastFrame(bbe::Key key);
		bool isKeyPressed(bbe::Key key);

		bool isMouseDown(bbe::MouseButton button);
		bool isMouseUp(bbe::MouseButton button);
		bool wasMouseDownLastFrame(bbe::MouseButton button);
		bool wasMouseUpLastFrame(bbe::MouseButton button);
		bool isMousePressed(bbe::MouseButton button);
		float getMouseX();
		float getMouseY();
		float getMouseXDelta();
		float getMouseYDelta();

		void setCursorMode(bbe::CursorMode cm);
	};
}

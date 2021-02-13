#pragma once

#include <cstdint>

#include "../BBE/GameTime.h"
#include "../BBE/CursorMode.h"
#include "../BBE/KeyboardKeys.h"
#include "../BBE/MouseButtons.h"
#include "../BBE/Vector2.h"
#include "../BBE/PhysWorld.h"
#include "../BBE/SoundManager.h"

namespace bbe
{
	class Window;
	class PrimitiveBrush2D;
	class PrimitiveBrush3D;

	class Game
	{
	private:
		Window*   m_pwindow           = nullptr;
		bool      m_started           = false;
		bool      m_externallyManaged = false;
		GameTime  m_gameTime;
		PhysWorld m_physWorld = PhysWorld({ 0, -20 });
#ifndef BBE_NO_AUDIO
		bbe::INTERNAL::SoundManager m_soundManager;
#endif

	public:
		Game();
		virtual ~Game();

		Game(const Game&)            = delete;
		Game(Game&&)                 = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&)      = delete;

		void start(int windowWidth, int windowHeight, const char* title);
		bool keepAlive();
		void frame();
		void shutdown();

		virtual void onStart()                            = 0;
		virtual void update(float timeSinceLastFrame)     = 0;
		virtual void draw3D(bbe::PrimitiveBrush3D &brush) = 0;
		virtual void draw2D(bbe::PrimitiveBrush2D &brush) = 0;
		virtual void onEnd()                              = 0;

		void setExternallyManaged(bool managed);
		bool isExternallyManaged() const;

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
		float getMouseX() const;
		float getMouseY() const;
		Vector2 getMouse() const;
		float getMouseXGlobal() const;
		float getMouseYGlobal() const;
		Vector2 getMouseGlobal() const;
		float getMouseXDelta();
		float getMouseYDelta();
		Vector2 getMouseDelta();
		float getMouseScrollX();
		float getMouseScrollY();
		Vector2 getMouseScroll();

		float getTimeSinceStartSeconds();
		float getTimeSinceStartMilliseconds(); 

		int getWindowWidth();
		int getScaledWindowWidth();
		int getWindowHeight();
		int getScaledWindowHeight();

		uint64_t getAmountOfFrames();

		void setCursorMode(bbe::CursorMode cm);

		PhysWorld* getPhysWorld();
	};
}

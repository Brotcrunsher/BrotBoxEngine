#pragma once

#include "../BBE/GameTime.h"

namespace bbe
{
	class Window;
	class PrimitiveBrush2D;
	class PrimitiveBrush3D;

	class Game
	{
	private:
		Window* m_window = nullptr;
		bool m_started = false;
		GameTime m_gameTime;

	public:
		Game();
		~Game();

		Game(const Game&)            = delete;
		Game(Game&&)                 = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&)      = delete;

		void start(int windowWidth, int windowHeight, const char* title);

		virtual void onStart() = 0;
		virtual void update(float timeSinceLastFrame) = 0;
		virtual void draw3D(PrimitiveBrush3D &brush) = 0;
		virtual void draw2D(PrimitiveBrush2D &brush) = 0;
		virtual void onEnd() = 0;

		bool isKeyDown(int keyCode);
		bool isKeyUp(int keyCode);
		bool wasKeyDownLastFrame(int keyCode);
		bool wasKeyUpLastFrame(int keyCode);
		bool isKeyPressed(int keyCode);
	};
}

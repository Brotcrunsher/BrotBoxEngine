#pragma once

#include <memory>
#include <functional>

#include "../BBE/glfwWrapper.h"
#include "../BBE/RenderManager.h"
#include "../BBE/WindowCloseMode.h"
#include "../BBE/Keyboard.h"
#include "../BBE/Mouse.h"
#include "../BBE/Hash.h"
#include "../BBE/CursorMode.h"
#include "../BBE/Model.h"


namespace bbe
{
	class PrimitiveBrush2D;
	class PrimitiveBrush3D;

	class Window
	{
		friend class Game;
	private:
		static size_t windowsAliveCounter;
		
		GLFWwindow                         *m_pwindow;
		std::unique_ptr<bbe::RenderManager> m_renderManager;
		bbe::List<std::function<void()>>    m_closeListeners;
		bbe::List<std::function<void()>>    m_frameStartListeners;

		int                                 m_width;
		int                                 m_height;

		bbe::WindowCloseMode                m_windowCloseMode = bbe::WindowCloseMode::CLOSE;
		
	public:
		Window(int width, int height, const char* title, uint32_t major = 0, uint32_t minor = 0, uint32_t patch = 0);

		Window(const Window& other) = delete;
		Window(Window&& other) = delete;
		Window& operator=(const Window& other) = delete;
		Window& operator=(Window&& other) = delete;

		void preDraw2D();
		void preDraw3D();
		void preDraw();
		bool keepAlive();
		void postDraw();
		void waitEndDraw();
		void waitTillIdle();

		bool isReadyToDraw();

		void setCursorMode(bbe::CursorMode cursorMode);

		GLFWwindow *getRaw();

		~Window();

		int getWidth() const;
		int getScaledWidth() const;

		int getHeight() const;
		int getScaledHeight() const;

		Vector2 getGlobalMousePos() const;

		void setWindowCloseMode(bbe::WindowCloseMode wcm);
		bbe::WindowCloseMode getWindowCloseMode() const;

		void showWindow();
		void hideWindow();
		bool isShown() const;

		PrimitiveBrush2D& getBrush2D();
		PrimitiveBrush3D& getBrush3D();

		static Window* INTERNAL_firstInstance;
		Keyboard INTERNAL_keyboard;
		Mouse INTERNAL_mouse;
		void INTERNAL_resize(int width, int height);

		void screenshot(const bbe::String& path);
		void setVideoRenderingMode(const char* path);

		void registerCloseListener(const std::function<void()>& listener);
		void executeCloseListeners();
		void registerFrameStartListener(const std::function<void()>& listener);
		void executeFrameStartListeneres();

#ifdef BBE_RENDERER_OPENGL
		uint32_t getAmountOfDrawcalls() const;
#endif
	};



	void INTERNAL_keyCallback(GLFWwindow *window, int keyCode, int scanCode, int action, int mods);
	void INTERNAL_charCallback(GLFWwindow* window, unsigned int c);
	void INTERNAL_cursorPosCallback(GLFWwindow *window, double xpos, double ypos);
	void INTERNAL_windowResizeCallback(GLFWwindow *window, int width, int height);
	void INTERNAL_mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	void INTERNAL_mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
	void INTERNAL_windowCloseCallback(GLFWwindow* window);

	template<>
	uint32_t hash(const Window &t);

}

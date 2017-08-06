#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/VulkanManager.h"
#include "../BBE/Keyboard.h"
#include "../BBE/Hash.h"


namespace bbe
{
	class PrimitiveBrush2D;

	class Window
	{
		friend class Game;
	private:
		static size_t windowsAliveCounter;

		GLFWwindow *m_pwindow;
		INTERNAL::vulkan::VulkanManager m_vulkanManager;
		int m_width;
		int m_height;
		
	public:
		Window(int width, int height, const char* title, uint32_t major = 0, uint32_t minor = 0, uint32_t patch = 0);

		Window(const Window& other) = delete;
		Window(Window&& other) = delete;
		Window& operator=(const Window& other) = delete;
		Window& operator=(Window&& other) = delete;

		void preDraw();
		bool keepAlive();
		void postDraw();

		GLFWwindow *getRaw();

		~Window();

		int getWidth() const;

		int getHeight() const;

		PrimitiveBrush2D* getBrush2D();

		static Window* INTERNAL_firstInstance;
		Keyboard INTERNAL_keyboard;
	};



	void INTERNAL_keyCallback(GLFWwindow *window, int keyCode, int scanCode, int action, int mods);

	template<>
	uint32_t hash(const Window &t);

}

#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "VulkanManager.h"


namespace bbe
{
	class Window
	{
	private:
		static size_t windowsAliveCounter;

		GLFWwindow *window;
		INTERNAL::vulkan::VulkanManager m_vulkanManager;

	public:
		Window(int width, int height, const char* title, uint32_t major = 0, uint32_t minor = 0, uint32_t patch = 0)
		{
			if (windowsAliveCounter == 0)
			{
				glfwInit();
			}
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			window = glfwCreateWindow(width, height, title, nullptr, nullptr);
			
			m_vulkanManager.init(title, major, minor, patch, window);
			
			windowsAliveCounter++;
		}

		Window(const Window& other) = delete;
		Window(Window&& other) = delete;
		Window& operator=(const Window& other) = delete;
		Window& operator=(Window&& other) = delete;

		bool keepAlive()
		{
			if (glfwWindowShouldClose(window))
			{
				return false;
			}

			glfwPollEvents();
			return true;
		}

		GLFWwindow *getRaw()
		{
			return window;
		}

		~Window()
		{
			glfwDestroyWindow(window);
			if (windowsAliveCounter == 1)
			{
				glfwTerminate();
			}

			windowsAliveCounter--;
		}

	};

	size_t Window::windowsAliveCounter = 0;
}
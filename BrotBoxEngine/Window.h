#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"


namespace bbe
{
	class Window
	{
	private:
		static size_t windowsAliveCounter;

		GLFWwindow *window;
	public:
		Window(int width, int height, const char* title)
		{
			if (windowsAliveCounter == 0)
			{
				glfwInit();
			}
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			window = glfwCreateWindow(width, height, title, nullptr, nullptr);
			

			windowsAliveCounter++;
		}

		bool keepAlive()
		{
			if (glfwWindowShouldClose(window))
			{
				return false;
			}

			glfwPollEvents();
			return true;
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
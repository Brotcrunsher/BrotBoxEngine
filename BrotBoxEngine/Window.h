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

		GLFWwindow *m_pwindow;
		INTERNAL::vulkan::VulkanManager m_vulkanManager;
		int m_width;
		int m_height;

	public:
		Window(int width, int height, const char* title, uint32_t major = 0, uint32_t minor = 0, uint32_t patch = 0)
			: m_width(width), m_height(height)
		{
			if (windowsAliveCounter == 0)
			{
				glfwInit();
			}
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			m_pwindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
			
			m_vulkanManager.init(title, major, minor, patch, m_pwindow, width, height);
			
			windowsAliveCounter++;
		}

		Window(const Window& other) = delete;
		Window(Window&& other) = delete;
		Window& operator=(const Window& other) = delete;
		Window& operator=(Window&& other) = delete;

		bool keepAlive()
		{
			if (glfwWindowShouldClose(m_pwindow))
			{
				return false;
			}

			glfwPollEvents();
			return true;
		}

		GLFWwindow *getRaw()
		{
			return m_pwindow;
		}

		~Window()
		{
			glfwDestroyWindow(m_pwindow);
			if (windowsAliveCounter == 1)
			{
				glfwTerminate();
			}

			windowsAliveCounter--;
		}

		int getWidth() const
		{
			return m_width;
		}

		int getHeight() const
		{
			return m_height;
		}
	};

	

	size_t Window::windowsAliveCounter = 0;
}
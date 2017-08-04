#include "stdafx.h"
#include "Window.h"
#include "PrimitiveBrush2D.h"
#include <iostream>


size_t bbe::Window::windowsAliveCounter = 0;
bbe::Window* bbe::Window::INTERNAL_firstInstance = nullptr;


bbe::Window::Window(int width, int height, const char * title, uint32_t major, uint32_t minor, uint32_t patch)
	: m_width(width), m_height(height)
{
	if(bbe::Window::INTERNAL_firstInstance == nullptr)
	{
		bbe::Window::INTERNAL_firstInstance = this;
	}
	if (windowsAliveCounter == 0)
	{
		glfwInit();
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_pwindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

	m_vulkanManager.init(title, major, minor, patch, m_pwindow, width, height);

	glfwSetKeyCallback(m_pwindow, INTERNAL_keyCallback);

	windowsAliveCounter++;
}

void bbe::Window::preDraw()
{
	m_vulkanManager.preDraw();
}

bool bbe::Window::keepAlive()
{
	if (glfwWindowShouldClose(m_pwindow))
	{
		return false;
	}

	glfwPollEvents();
	return true;
}

void bbe::Window::postDraw()
{
	m_vulkanManager.postDraw();
}

GLFWwindow * bbe::Window::getRaw()
{
	return m_pwindow;
}

bbe::Window::~Window()
{
	m_vulkanManager.destroy();
	glfwDestroyWindow(m_pwindow);
	if (windowsAliveCounter == 1)
	{
		glfwTerminate();
	}

	windowsAliveCounter--;

	if(this == bbe::Window::INTERNAL_firstInstance)
	{
		bbe::Window::INTERNAL_firstInstance = nullptr;
	}
}

int bbe::Window::getWidth() const
{
	return m_width;
}

int bbe::Window::getHeight() const
{
	return m_height;
}

bbe::PrimitiveBrush2D * bbe::Window::getBrush2D()
{
	return m_vulkanManager.getBrush2D();
}

void bbe::INTERNAL_keyCallback(GLFWwindow * window, int keyCode, int scanCode, int action, int mods)
{
	std::cout << "hai";
	if (action == GLFW_PRESS)
	{
		bbe::Window::INTERNAL_firstInstance->INTERNAL_keyboard.INTERNAL_press(keyCode);
	}
	else if (action == GLFW_RELEASE)
	{
		bbe::Window::INTERNAL_firstInstance->INTERNAL_keyboard.INTERNAL_release(keyCode);
	}
}

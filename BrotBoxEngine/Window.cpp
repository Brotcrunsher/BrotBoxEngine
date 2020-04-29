#include "BBE/Window.h"
#include "BBE/PrimitiveBrush2D.h"
#include "BBE/PrimitiveBrush3D.h"
#include <iostream>
#include "BBE/MouseButtons.h"
#include "BBE/FatalErrors.h"


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
		if (glfwInit() == GLFW_FALSE)
		{
			bbe::INTERNAL::triggerFatalError("An error occurred while initializing GLFW.");
		}
		if (glfwVulkanSupported() == GLFW_FALSE)
		{
			bbe::INTERNAL::triggerFatalError("Your GPU and/or driver does not support vulkan!");
		}
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_pwindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (m_pwindow == nullptr)
	{
		bbe::INTERNAL::triggerFatalError("Could not create window!");
	}

	std::cout << "Init vulkan manager" << std::endl;
	m_vulkanManager.init(title, major, minor, patch, m_pwindow, width, height);


	std::cout << "Setting glwf callbacks" << std::endl;
	glfwSetKeyCallback(m_pwindow, INTERNAL_keyCallback);
	glfwSetCursorPosCallback(m_pwindow, INTERNAL_cursorPosCallback);
	glfwSetMouseButtonCallback(m_pwindow, INTERNAL_mouseButtonCallback);
	glfwSetWindowSizeCallback(m_pwindow, INTERNAL_windowResizeCallback);
	glfwSetScrollCallback(m_pwindow, INTERNAL_mouseScrollCallback);
	double mX = 0;
	double mY = 0;
	glfwGetCursorPos(m_pwindow, &mX, &mY);
	
	std::cout << "Init mouse" << std::endl;
	INTERNAL_mouse.INTERNAL_moveMouse((float)mX, (float)mY);
	windowsAliveCounter++;
}

void bbe::Window::preDraw2D()
{
	m_vulkanManager.preDraw2D();
}

void bbe::Window::preDraw3D()
{
	m_vulkanManager.preDraw3D();
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

void bbe::Window::waitEndDraw()
{
	m_vulkanManager.waitEndDraw();
}

void bbe::Window::setCursorMode(bbe::CursorMode cursorMode)
{
	switch (cursorMode)
	{
	case bbe::CursorMode::DISABLED:
		glfwSetInputMode(m_pwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		break;
	case bbe::CursorMode::NORMAL:
		glfwSetInputMode(m_pwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	case bbe::CursorMode::HIDDEN:
		glfwSetInputMode(m_pwindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		break;
	default:
		throw IllegalArgumentException();
	}
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

bbe::PrimitiveBrush2D& bbe::Window::getBrush2D()
{
	return m_vulkanManager.getBrush2D();
}

bbe::PrimitiveBrush3D& bbe::Window::getBrush3D()
{
	return m_vulkanManager.getBrush3D();
}

void bbe::Window::INTERNAL_resize(int width, int height)
{
	m_width = width;
	m_height = height;

	m_vulkanManager.resize(width, height);
}

void bbe::INTERNAL_keyCallback(GLFWwindow * window, int keyCode, int scanCode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		bbe::Window::INTERNAL_firstInstance->INTERNAL_keyboard.INTERNAL_press((bbe::Key)keyCode);
	}
	else if (action == GLFW_RELEASE)
	{
		bbe::Window::INTERNAL_firstInstance->INTERNAL_keyboard.INTERNAL_release((bbe::Key)keyCode);
	}
}

void bbe::INTERNAL_cursorPosCallback(GLFWwindow * window, double xpos, double ypos)
{
	bbe::Window::INTERNAL_firstInstance->INTERNAL_mouse.INTERNAL_moveMouse((float)xpos, (float)ypos);
}

void bbe::INTERNAL_windowResizeCallback(GLFWwindow * window, int width, int height)
{
	bbe::Window::INTERNAL_firstInstance->INTERNAL_resize(width, height);
}

void bbe::INTERNAL_mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		bbe::Window::INTERNAL_firstInstance->INTERNAL_mouse.INTERNAL_press((bbe::MouseButton)button);
	}
	else if (action == GLFW_RELEASE)
	{
		bbe::Window::INTERNAL_firstInstance->INTERNAL_mouse.INTERNAL_release((bbe::MouseButton)button);
	}
}

void bbe::INTERNAL_mouseScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
	bbe::Window::INTERNAL_firstInstance->INTERNAL_mouse.INTERNAL_scroll(xoffset, yoffset);
}

template<>
uint32_t bbe::hash(const bbe::Window & t)
{
	//UNTESTED
	return t.getWidth() * 7 + t.getHeight() * 13;
}
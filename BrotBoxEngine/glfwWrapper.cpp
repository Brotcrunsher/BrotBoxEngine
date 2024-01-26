#include "BBE/glfwWrapper.h"

int bbe::glfwWrapper::glfwVulkanSupported(void)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwVulkanSupported();
#else
	return 0;
#endif
}

int bbe::glfwWrapper::glfwInit()
{
#ifndef BBE_RENDERER_NULL
	return ::glfwInit();
#else
	return 1;
#endif
}

void bbe::glfwWrapper::glfwWindowHint(int hint, int value)
{
#ifndef BBE_RENDERER_NULL
	::glfwWindowHint(hint, value);
#endif
}

GLFWwindow* bbe::glfwWrapper::glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwCreateWindow(width, height, title, monitor, share);
#else
	return (GLFWwindow*)new int;
#endif
}

void bbe::glfwWrapper::glfwGetWindowContentScale(GLFWwindow* window, float* xscale, float* yscale)
{
#if defined(__EMSCRIPTEN__) || defined(BBE_RENDERER_NULL)
	if (xscale) *xscale = 1;
	if (yscale) *yscale = 1;
#else
	::glfwGetWindowContentScale(window, xscale, yscale);
#endif
}

GLFWkeyfun bbe::glfwWrapper::glfwSetKeyCallback(GLFWwindow* window, GLFWkeyfun callback)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwSetKeyCallback(window, callback);
#else
	return callback;
#endif
}

GLFWcharfun bbe::glfwWrapper::glfwSetCharCallback(GLFWwindow* handle, GLFWcharfun cbfun)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwSetCharCallback(handle, cbfun);
#else
	return cbfun;
#endif
}

GLFWcursorposfun bbe::glfwWrapper::glfwSetCursorPosCallback(GLFWwindow* handle, GLFWcursorposfun cbfun)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwSetCursorPosCallback(handle, cbfun);
#else
	return cbfun;
#endif
}

GLFWmousebuttonfun bbe::glfwWrapper::glfwSetMouseButtonCallback(GLFWwindow* handle, GLFWmousebuttonfun cbfun)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwSetMouseButtonCallback(handle, cbfun);
#else
	return cbfun;
#endif
}

GLFWwindowsizefun bbe::glfwWrapper::glfwSetWindowSizeCallback(GLFWwindow* handle, GLFWwindowsizefun cbfun)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwSetWindowSizeCallback(handle, cbfun);
#else
	return cbfun;
#endif
}

GLFWscrollfun bbe::glfwWrapper::glfwSetScrollCallback(GLFWwindow* handle, GLFWscrollfun cbfun)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwSetScrollCallback(handle, cbfun);
#else
	return cbfun;
#endif
}

GLFWwindowclosefun bbe::glfwWrapper::glfwSetWindowCloseCallback(GLFWwindow* handle, GLFWwindowclosefun cbfun)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwSetWindowCloseCallback(handle, cbfun);
#else
	return cbfun;
#endif
}

GLFWwindowrefreshfun bbe::glfwWrapper::glfwSetWindowRefreshCallback(GLFWwindow* handle, GLFWwindowrefreshfun cbfun)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwSetWindowRefreshCallback(handle, cbfun);
#else
	return cbfun;
#endif
}

GLFWwindowposfun bbe::glfwWrapper::glfwSetWindowPosCallback(GLFWwindow* handle, GLFWwindowposfun cbfun)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwSetWindowPosCallback(handle, cbfun);
#else
	return cbfun;
#endif
}

void bbe::glfwWrapper::glfwGetCursorPos(GLFWwindow* handle, double* xpos, double* ypos)
{
#ifndef BBE_RENDERER_NULL
	::glfwGetCursorPos(handle, xpos, ypos);
#endif
}

void bbe::glfwWrapper::glfwSwapInterval(int interval)
{
#ifndef BBE_RENDERER_NULL
	::glfwSwapInterval(interval);
#endif
}

int bbe::glfwWrapper::glfwWindowShouldClose(GLFWwindow* handle)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwWindowShouldClose(handle);
#else
	return 0;
#endif
}

void bbe::glfwWrapper::glfwPollEvents(void)
{
#ifndef BBE_RENDERER_NULL
	::glfwPollEvents();
#endif
}

void bbe::glfwWrapper::glFinish(void)
{
#ifdef BBE_RENDERER_OPENGL
	::glFinish();
#endif
}

void bbe::glfwWrapper::glfwSetInputMode(GLFWwindow* handle, int mode, int value)
{
#ifndef BBE_RENDERER_NULL
	::glfwSetInputMode(handle, mode, value);
#endif
}

void bbe::glfwWrapper::glfwDestroyWindow(GLFWwindow* handle)
{
#ifndef BBE_RENDERER_NULL
	::glfwDestroyWindow(handle);
#else
	delete handle;
#endif
}

void bbe::glfwWrapper::glfwTerminate(void)
{
#ifndef BBE_RENDERER_NULL
	::glfwTerminate();
#endif
}

void bbe::glfwWrapper::glfwGetWindowPos(GLFWwindow* handle, int* xpos, int* ypos)
{
#ifndef BBE_RENDERER_NULL
	::glfwGetWindowPos(handle, xpos, ypos);
#endif
}

const char* bbe::glfwWrapper::glfwGetClipboardString(GLFWwindow* handle)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwGetClipboardString(handle);
#else
	return "";
#endif
}

void bbe::glfwWrapper::glfwSetClipboardString(GLFWwindow* handle, const char* string)
{
#ifndef BBE_RENDERER_NULL
	::glfwSetClipboardString(handle, string);
#endif
}

void bbe::glfwWrapper::glfwSetWindowShouldClose(GLFWwindow* window, int value)
{
#ifndef BBE_RENDERER_NULL
	::glfwSetWindowShouldClose(window, value);
#endif
}

void bbe::glfwWrapper::glfwShowWindow(GLFWwindow* window)
{
#ifndef BBE_RENDERER_NULL
	::glfwShowWindow(window);
#endif
}

void bbe::glfwWrapper::glfwHideWindow(GLFWwindow* window)
{
#ifndef BBE_RENDERER_NULL
	::glfwHideWindow(window);
#endif
}

void bbe::glfwWrapper::glfwFocusWindow(GLFWwindow* window)
{
#ifndef BBE_RENDERER_NULL
	::glfwFocusWindow(window);
#endif
}

int bbe::glfwWrapper::glfwGetWindowAttrib(GLFWwindow* window, int attrib)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwGetWindowAttrib(window, attrib);
#else
	return 0;
#endif
}

void bbe::glfwWrapper::glfwSetWindowUserPointer(GLFWwindow* window, void* pointer)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwSetWindowUserPointer(window, pointer);
#endif
}

void* bbe::glfwWrapper::glfwGetWindowUserPointer(GLFWwindow* window)
{
#ifndef BBE_RENDERER_NULL
	return ::glfwGetWindowUserPointer(window);
#else
	return nullptr;
#endif
}

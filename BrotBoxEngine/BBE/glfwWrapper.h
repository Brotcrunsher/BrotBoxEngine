#pragma once

#ifdef BBE_RENDERER_OPENGL
#include <GL/glew.h>
#endif
#include "GLFW/glfw3.h"



namespace bbe
{
	namespace glfwWrapper
	{
		int glfwInit();
		int glfwVulkanSupported(void);
		void glfwWindowHint(int hint, int value);
		GLFWwindow* glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
		void glfwGetWindowContentScale(GLFWwindow* window, float* xscale, float* yscale);
		GLFWkeyfun glfwSetKeyCallback(GLFWwindow* window, GLFWkeyfun callback);
		GLFWcharfun glfwSetCharCallback(GLFWwindow* handle, GLFWcharfun cbfun);
		GLFWcursorposfun glfwSetCursorPosCallback(GLFWwindow* handle, GLFWcursorposfun cbfun);
		GLFWmousebuttonfun glfwSetMouseButtonCallback(GLFWwindow* handle, GLFWmousebuttonfun cbfun);
		GLFWwindowsizefun glfwSetWindowSizeCallback(GLFWwindow* handle, GLFWwindowsizefun cbfun);
		GLFWscrollfun glfwSetScrollCallback(GLFWwindow* handle, GLFWscrollfun cbfun);
		GLFWwindowclosefun glfwSetWindowCloseCallback(GLFWwindow* handle, GLFWwindowclosefun cbfun);
		void glfwGetCursorPos(GLFWwindow* handle, double* xpos, double* ypos);
		int glfwWindowShouldClose(GLFWwindow* handle);
		void glfwPollEvents(void);
		void glfwSetInputMode(GLFWwindow* handle, int mode, int value);
		void glfwDestroyWindow(GLFWwindow* handle);
		void glfwTerminate(void);
		void glfwGetWindowPos(GLFWwindow* handle, int* xpos, int* ypos);
		const char* glfwGetClipboardString(GLFWwindow* handle);
		void glfwSetClipboardString(GLFWwindow* handle, const char* string);
		void glfwSetWindowShouldClose(GLFWwindow* window, int value);
		void glfwShowWindow(GLFWwindow* window);
		void glfwHideWindow(GLFWwindow* window);
		int glfwGetWindowAttrib(GLFWwindow* window, int attrib);
		void glfwSetWindowUserPointer(GLFWwindow* window, void* pointer);
		void* glfwGetWindowUserPointer(GLFWwindow* window);
	}
}

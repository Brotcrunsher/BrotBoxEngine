#pragma once

#ifdef BBE_RENDERER_OPENGL
#include <GL/glew.h>
#endif
#include "GLFW/glfw3.h"

namespace bbe
{
	namespace glfwWrapper
	{
		void glfwGetWindowContentScale(GLFWwindow* window, float* xscale, float* yscale);
	}
}

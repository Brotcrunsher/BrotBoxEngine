#include "BBE/glfwWrapper.h"

void bbe::glfwWrapper::glfwGetWindowContentScale(GLFWwindow* window, float* xscale, float* yscale)
{
#ifdef __EMSCRIPTEN__
	if (xscale) *xscale = 1;
	if (yscale) *yscale = 1;
#else
	::glfwGetWindowContentScale(window, xscale, yscale);
#endif
}

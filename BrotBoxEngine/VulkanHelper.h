#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"

namespace bbe
{
	namespace vulkan
	{
#define ASSERT_VULKAN(val)\
		if(val != VK_SUCCESS){\
			__debugbreak();\
		}
	}
}
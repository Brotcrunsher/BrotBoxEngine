#pragma once

#include "../BBE/glfwWrapper.h"
#include "../BBE/String.h"

namespace bbe
{
	enum class MouseButton
	{
		LAST = GLFW_MOUSE_BUTTON_LAST,

		LEFT = GLFW_MOUSE_BUTTON_LEFT,
		RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
		MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE,
		_4 = GLFW_MOUSE_BUTTON_4,
		_5 = GLFW_MOUSE_BUTTON_5,
		_6 = GLFW_MOUSE_BUTTON_6,
		_7 = GLFW_MOUSE_BUTTON_7,
		_8 = GLFW_MOUSE_BUTTON_8,
	};

	bbe::String mouseButtonToString(MouseButton button);
	bool isMouseButtonValid(MouseButton button);
}
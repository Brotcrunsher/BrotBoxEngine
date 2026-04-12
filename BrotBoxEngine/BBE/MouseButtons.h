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
		/// Not a GLFW index: aggregate queries over all physical buttons (\c isButtonPressed, etc.).
		ANY = 32766,
	};

	bbe::String mouseButtonToString(MouseButton button);
	/// True for real GLFW mouse buttons (array indices \c 0 … \c LAST), excluding \c ANY.
	bool isMouseButtonPhysical(MouseButton button);
	/// True for physical buttons and \c ANY (valid for query APIs).
	bool isMouseButtonValid(MouseButton button);
}
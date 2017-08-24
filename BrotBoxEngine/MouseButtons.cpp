#include "stdafx.h"
#include "BBE/MouseButtons.h"
#include "BBE/Exceptions.h"

bbe::String bbe::mouseButtonToString(MouseButton button)
{
	switch (button)
	{
	case MouseButton::LEFT:
		return bbe::String("MB_LEFT");
	case MouseButton::RIGHT:
		return bbe::String("MB_RIGHT");
	case MouseButton::MIDDLE:
		return bbe::String("MB_MIDDLE");
	case MouseButton::_4:
		return bbe::String("MB_4");
	case MouseButton::_5:
		return bbe::String("MB_5");
	case MouseButton::_6:
		return bbe::String("MB_6");
	case MouseButton::_7:
		return bbe::String("MB_7");
	case MouseButton::_8:
		return bbe::String("MB_8");
	}

	throw NoSuchMouseButtonException();
}

bool bbe::isMouseButtonValid(MouseButton button)
{
	switch (button)
	{
		case MouseButton::LEFT  :
		case MouseButton::RIGHT :
		case MouseButton::MIDDLE:
		case MouseButton::_4	   :
		case MouseButton::_5    :
		case MouseButton::_6    :
		case MouseButton::_7    :
		case MouseButton::_8    :
			return true;
	}
	return false;
}

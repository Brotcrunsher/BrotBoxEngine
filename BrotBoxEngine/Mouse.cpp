#include "BBE/Mouse.h"
#include "BBE/Vector2.h"
#include "BBE/Error.h"

bool bbe::Mouse::isButtonDown(bbe::MouseButton button) const
{
	if (!isMouseButtonValid(button))
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}
	return m_pButtonsThisFrame[(int)button];
}

bool bbe::Mouse::isButtonUp(bbe::MouseButton button) const
{
	if (!isMouseButtonValid(button))
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}
	return !m_pButtonsThisFrame[(int)button];
}

bool bbe::Mouse::wasButtonDownLastFrame(bbe::MouseButton button) const
{
	if (!isMouseButtonValid(button))
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}
	return m_pButtonsLastFrame[(int)button];
}

bool bbe::Mouse::wasButtonUpLastFrame(bbe::MouseButton button) const
{
	if (!isMouseButtonValid(button))
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}
	return !m_pButtonsLastFrame[(int)button];
}

bool bbe::Mouse::isButtonPressed(bbe::MouseButton button) const
{
	if (!isMouseButtonValid(button))
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}
	return m_pButtonsThisFrame[(int)button] && !m_pButtonsLastFrame[(int)button];
}

bool bbe::Mouse::isButtonReleased(bbe::MouseButton button) const
{
	if (!isMouseButtonValid(button))
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}
	return !m_pButtonsThisFrame[(int)button] && m_pButtonsLastFrame[(int)button];
}

void bbe::Mouse::INTERNAL_moveMouse(float x, float y)
{
	m_mouseNextFrameX = x;
	m_mouseNextFrameY = y;
}

void bbe::Mouse::INTERNAL_press(MouseButton button)
{
	if (!isMouseButtonValid(button))
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}

	m_pButtonsNextFrame[(int)button] = true;
}

void bbe::Mouse::INTERNAL_release(MouseButton button)
{
	if (!isMouseButtonValid(button))
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}

	m_pButtonsNextFrame[(int)button] = false;
}

void bbe::Mouse::INTERNAL_scroll(float x, float y)
{
	m_mouseScrollXNext = x;
	m_mouseScrollYNext = y;
}


void bbe::Mouse::update(float globalMousePosX, float globalMousePosY)
{
	m_mouseLastFrameX = m_mouseCurrentFrameX;
	m_mouseLastFrameY = m_mouseCurrentFrameY;
	m_mouseCurrentFrameX = m_mouseNextFrameX;
	m_mouseCurrentFrameY = m_mouseNextFrameY;

	m_mouseLastFrameXGlobal = m_mouseCurrentFrameXGlobal;
	m_mouseLastFrameYGlobal = m_mouseCurrentFrameYGlobal;
	m_mouseCurrentFrameXGlobal = globalMousePosX;
	m_mouseCurrentFrameYGlobal = globalMousePosY;

	m_mouseScrollX = m_mouseScrollXNext;
	m_mouseScrollY = m_mouseScrollYNext;
	m_mouseScrollXNext = 0;
	m_mouseScrollYNext = 0;

	memcpy(m_pButtonsLastFrame, m_pButtonsThisFrame, ((int)MouseButton::LAST + 1) * sizeof(bool));
	memcpy(m_pButtonsThisFrame, m_pButtonsNextFrame, ((int)MouseButton::LAST + 1) * sizeof(bool));
}

bbe::Mouse::Mouse()
{
	memset(m_pButtonsLastFrame, 0, ((int)MouseButton::LAST + 1) * sizeof(bool));
	memset(m_pButtonsThisFrame, 0, ((int)MouseButton::LAST + 1) * sizeof(bool));
	memset(m_pButtonsNextFrame, 0, ((int)MouseButton::LAST + 1) * sizeof(bool));
}

float bbe::Mouse::getMouseX() const
{
	return m_mouseCurrentFrameX;
}

float bbe::Mouse::getMouseY() const
{
	return m_mouseCurrentFrameY;
}

bbe::Vector2 bbe::Mouse::getMouse() const
{
	return Vector2(getMouseX(), getMouseY());
}

float bbe::Mouse::getMouseXPrevious() const
{
	return m_mouseLastFrameX;
}

float bbe::Mouse::getMouseYPrevious() const
{
	return m_mouseLastFrameY;
}

float bbe::Mouse::getMouseXGlobal() const
{
	return m_mouseCurrentFrameXGlobal;
}

float bbe::Mouse::getMouseYGlobal() const
{
	return m_mouseCurrentFrameYGlobal;
}

bbe::Vector2 bbe::Mouse::getMouseGlobal() const
{
	return Vector2(getMouseXGlobal(), getMouseYGlobal());
}

float bbe::Mouse::getScrollX()
{
	return m_mouseScrollX;
}

float bbe::Mouse::getScrollY()
{
	return m_mouseScrollY;
}

bbe::Vector2 bbe::Mouse::getScroll()
{
	return Vector2(
		getScrollX(),
		getScrollY()
	);
}

float bbe::Mouse::getMouseXDelta()
{
	return m_mouseCurrentFrameX - m_mouseLastFrameX;
}

float bbe::Mouse::getMouseYDelta()
{
	return m_mouseCurrentFrameY - m_mouseLastFrameY;
}

bbe::Vector2 bbe::Mouse::getMouseDelta()
{
	return Vector2(getMouseXDelta(), getMouseYDelta());
}

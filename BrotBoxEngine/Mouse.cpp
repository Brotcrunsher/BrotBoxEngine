#include "BBE/Mouse.h"
#include "BBE/Vector2.h"
#include "BBE/Exceptions.h"

bool bbe::Mouse::isButtonDown(bbe::MouseButton button)
{
	if (!isMouseButtonValid(button))
	{
		throw NoSuchMouseButtonException();
	}
	return m_pButtonsThisFrame[(int)button];
}

bool bbe::Mouse::isButtonUp(bbe::MouseButton button)
{
	if (!isMouseButtonValid(button))
	{
		throw NoSuchMouseButtonException();
	}
	return !m_pButtonsThisFrame[(int)button];
}

bool bbe::Mouse::wasButtonDownLastFrame(bbe::MouseButton button)
{
	if (!isMouseButtonValid(button))
	{
		throw NoSuchMouseButtonException();
	}
	return m_pButtonsLastFrame[(int)button];
}

bool bbe::Mouse::wasButtonUpLastFrame(bbe::MouseButton button)
{
	if (!isMouseButtonValid(button))
	{
		throw NoSuchMouseButtonException();
	}
	return !m_pButtonsLastFrame[(int)button];
}

bool bbe::Mouse::isButtonPressed(bbe::MouseButton button)
{
	if (!isMouseButtonValid(button))
	{
		throw NoSuchMouseButtonException();
	}
	return m_pButtonsThisFrame[(int)button] && !m_pButtonsLastFrame[(int)button];
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
		throw NoSuchMouseButtonException();
	}

	m_pButtonsNextFrame[(int)button] = true;
}

void bbe::Mouse::INTERNAL_release(MouseButton button)
{
	if (!isMouseButtonValid(button))
	{
		throw NoSuchMouseButtonException();
	}

	m_pButtonsNextFrame[(int)button] = false;
}


void bbe::Mouse::update()
{
	m_mouseLastFrameX = m_mouseCurrentFrameX;
	m_mouseLastFrameY = m_mouseCurrentFrameY;
	m_mouseCurrentFrameX = m_mouseNextFrameX;
	m_mouseCurrentFrameY = m_mouseNextFrameY;

	memcpy(m_pButtonsLastFrame, m_pButtonsThisFrame, ((int)MouseButton::LAST + 1) * sizeof(bool));
	memcpy(m_pButtonsThisFrame, m_pButtonsNextFrame, ((int)MouseButton::LAST + 1) * sizeof(bool));
}

bbe::Mouse::Mouse()
{
	memset(m_pButtonsLastFrame, 0, ((int)MouseButton::LAST + 1) * sizeof(bool));
	memset(m_pButtonsThisFrame, 0, ((int)MouseButton::LAST + 1) * sizeof(bool));
	memset(m_pButtonsNextFrame, 0, ((int)MouseButton::LAST + 1) * sizeof(bool));
}

float bbe::Mouse::getMouseX()
{
	return m_mouseCurrentFrameX;
}

float bbe::Mouse::getMouseY()
{
	return m_mouseCurrentFrameY;
}

bbe::Vector2 bbe::Mouse::getMouse()
{
	return Vector2(getMouseX(), getMouseY());
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

#include "stdafx.h"
#include "BBE/Mouse.h"
#include "BBE/Vector2.h"

void bbe::Mouse::INTERNAL_moveMouse(float x, float y)
{
	m_mouseNextFrameX = x;
	m_mouseNextFrameY = y;
}

void bbe::Mouse::update()
{
	m_mouseLastFrameX = m_mouseCurrentFrameX;
	m_mouseLastFrameY = m_mouseCurrentFrameY;
	m_mouseCurrentFrameX = m_mouseNextFrameX;
	m_mouseCurrentFrameY = m_mouseNextFrameY;
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

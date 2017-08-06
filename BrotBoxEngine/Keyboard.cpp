#include "stdafx.h"
#include "BBE/Keyboard.h"
#include "BBE/Exceptions.h"

void bbe::Keyboard::INTERNAL_press(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	m_keysNextFrame[keyCode] = true;
}

void bbe::Keyboard::INTERNAL_release(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	m_keysNextFrame[keyCode] = false;
}

void bbe::Keyboard::update()
{
	memcpy(m_keysLastFrame, m_keysThisFrame, (KEY_LAST + 1) * sizeof(bool));
	memcpy(m_keysThisFrame, m_keysNextFrame, (KEY_LAST + 1) * sizeof(bool));
}

bbe::Keyboard::Keyboard()
{
	memset(m_keysLastFrame, 0, (KEY_LAST + 1) * sizeof(bool));
	memset(m_keysThisFrame, 0, (KEY_LAST + 1) * sizeof(bool));
	memset(m_keysNextFrame, 0, (KEY_LAST + 1) * sizeof(bool));
}

bool bbe::Keyboard::isKeyDown(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	return m_keysThisFrame[keyCode];
}

bool bbe::Keyboard::isKeyUp(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	return !m_keysThisFrame[keyCode];
}

bool bbe::Keyboard::wasKeyDownLastFrame(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	return m_keysLastFrame[keyCode];
}

bool bbe::Keyboard::wasKeyUpLastFrame(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	return !m_keysLastFrame[keyCode];
}

bool bbe::Keyboard::isKeyPressed(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	return m_keysThisFrame[keyCode] && !m_keysLastFrame[keyCode];
}

#include "stdafx.h"
#include "BBE/Keyboard.h"
#include "BBE/Exceptions.h"

void bbe::Keyboard::INTERNAL_press(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	m_pkeysNextFrame[keyCode] = true;
}

void bbe::Keyboard::INTERNAL_release(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	m_pkeysNextFrame[keyCode] = false;
}

void bbe::Keyboard::update()
{
	memcpy(m_pkeysLastFrame, m_pkeysThisFrame, (Keys::LAST + 1) * sizeof(bool));
	memcpy(m_pkeysThisFrame, m_pkeysNextFrame, (Keys::LAST + 1) * sizeof(bool));
}

bbe::Keyboard::Keyboard()
{
	memset(m_pkeysLastFrame, 0, (Keys::LAST + 1) * sizeof(bool));
	memset(m_pkeysThisFrame, 0, (Keys::LAST + 1) * sizeof(bool));
	memset(m_pkeysNextFrame, 0, (Keys::LAST + 1) * sizeof(bool));
}

bool bbe::Keyboard::isKeyDown(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysThisFrame[keyCode];
}

bool bbe::Keyboard::isKeyUp(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	return !m_pkeysThisFrame[keyCode];
}

bool bbe::Keyboard::wasKeyDownLastFrame(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysLastFrame[keyCode];
}

bool bbe::Keyboard::wasKeyUpLastFrame(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	return !m_pkeysLastFrame[keyCode];
}

bool bbe::Keyboard::isKeyPressed(int keyCode)
{
	if (!isKeyCodeValid(keyCode))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysThisFrame[keyCode] && !m_pkeysLastFrame[keyCode];
}

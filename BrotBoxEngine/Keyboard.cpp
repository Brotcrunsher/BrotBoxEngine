#include "stdafx.h"
#include "BBE/Keyboard.h"
#include "BBE/Exceptions.h"

void bbe::Keyboard::INTERNAL_press(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	m_pkeysNextFrame[key] = true;
}

void bbe::Keyboard::INTERNAL_release(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	m_pkeysNextFrame[key] = false;
}

void bbe::Keyboard::update()
{
	memcpy(m_pkeysLastFrame, m_pkeysThisFrame, (Key::LAST + 1) * sizeof(bool));
	memcpy(m_pkeysThisFrame, m_pkeysNextFrame, (Key::LAST + 1) * sizeof(bool));
}

bbe::Keyboard::Keyboard()
{
	memset(m_pkeysLastFrame, 0, (Key::LAST + 1) * sizeof(bool));
	memset(m_pkeysThisFrame, 0, (Key::LAST + 1) * sizeof(bool));
	memset(m_pkeysNextFrame, 0, (Key::LAST + 1) * sizeof(bool));
}

bool bbe::Keyboard::isKeyDown(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysThisFrame[key];
}

bool bbe::Keyboard::isKeyUp(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return !m_pkeysThisFrame[key];
}

bool bbe::Keyboard::wasKeyDownLastFrame(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysLastFrame[key];
}

bool bbe::Keyboard::wasKeyUpLastFrame(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return !m_pkeysLastFrame[key];
}

bool bbe::Keyboard::isKeyPressed(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysThisFrame[key] && !m_pkeysLastFrame[key];
}

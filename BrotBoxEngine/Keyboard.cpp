#include "BBE/Keyboard.h"
#include "BBE/Exceptions.h"

void bbe::Keyboard::INTERNAL_press(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	m_pkeysNextFrame[(int)key].down = true;
	m_pkeysNextFrame[(int)key].pressed = true;
}

void bbe::Keyboard::INTERNAL_release(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	m_pkeysNextFrame[(int)key].down = false;
}

void bbe::Keyboard::update()
{
	m_pkeysThisFrame = m_pkeysNextFrame;
	for (size_t i = 0; i < m_pkeysNextFrame.size(); i++)
	{
		m_pkeysNextFrame[i].pressed = false;
	}
}

bool bbe::Keyboard::isKeyDown(bbe::Key key, bool checkValid)
{
	if (checkValid && !isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysThisFrame[(int)key].down;
}

bool bbe::Keyboard::isKeyUp(bbe::Key key, bool checkValid)
{
	if (checkValid && !isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return !m_pkeysThisFrame[(int)key].down;
}

bool bbe::Keyboard::isKeyPressed(bbe::Key key, bool checkValid)
{
	if (checkValid && !isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysThisFrame[(int)key].pressed;
}

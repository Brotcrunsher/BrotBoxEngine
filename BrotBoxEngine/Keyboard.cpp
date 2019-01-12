#include "BBE/Keyboard.h"
#include "BBE/Exceptions.h"

void bbe::Keyboard::INTERNAL_press(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	m_pkeysNextFrame[(int)key] = true;
}

void bbe::Keyboard::INTERNAL_release(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	m_pkeysNextFrame[(int)key] = false;
}

void bbe::Keyboard::update()
{
	memcpy(m_pkeysLastFrame, m_pkeysThisFrame, ((int)Key::LAST + 1) * sizeof(bool));
	memcpy(m_pkeysThisFrame, m_pkeysNextFrame, ((int)Key::LAST + 1) * sizeof(bool));
}

bbe::Keyboard::Keyboard()
{
	memset(m_pkeysLastFrame, 0, ((int)Key::LAST + 1) * sizeof(bool));
	memset(m_pkeysThisFrame, 0, ((int)Key::LAST + 1) * sizeof(bool));
	memset(m_pkeysNextFrame, 0, ((int)Key::LAST + 1) * sizeof(bool));
}

bool bbe::Keyboard::isKeyDown(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysThisFrame[(int)key];
}

bool bbe::Keyboard::isKeyUp(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return !m_pkeysThisFrame[(int)key];
}

bool bbe::Keyboard::wasKeyDownLastFrame(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysLastFrame[(int)key];
}

bool bbe::Keyboard::wasKeyUpLastFrame(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return !m_pkeysLastFrame[(int)key];
}

bool bbe::Keyboard::isKeyPressed(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		throw NoSuchKeycodeException();
	}

	return m_pkeysThisFrame[(int)key] && !m_pkeysLastFrame[(int)key];
}

#include "BBE/Keyboard.h"
#include "BBE/Error.h"

void bbe::Keyboard::INTERNAL_press(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		bbe::Crash(bbe::Error::NoSuchKeycode);
	}

	m_pkeysNextFrame[(int)key].down = true;
	m_pkeysNextFrame[(int)key].pressed = true;
	m_pkeysNextFrame[(int)key].typed = true;
	m_pkeysNextFrame[(int)key].nextTypedTime = bbe::TimePoint().plusMilliseconds(500);
}

void bbe::Keyboard::INTERNAL_release(bbe::Key key)
{
	if (!isKeyCodeValid(key))
	{
		bbe::Crash(bbe::Error::NoSuchKeycode);
	}

	m_pkeysNextFrame[(int)key].down = false;
}

void bbe::Keyboard::update()
{
	m_pkeysThisFrame = m_pkeysNextFrame;
	for (size_t i = 0; i < m_pkeysNextFrame.size(); i++)
	{
		m_pkeysNextFrame[i].pressed = false;
		m_pkeysNextFrame[i].typed = false;
		if (m_pkeysNextFrame[i].down)
		{
			if (m_pkeysNextFrame[i].nextTypedTime.hasPassed())
			{
				m_pkeysNextFrame[i].nextTypedTime = bbe::TimePoint().plusMilliseconds(100);
				m_pkeysNextFrame[i].typed = true;
			}
		}
	}
}

bool bbe::Keyboard::isKeyDown(bbe::Key key, bool checkValid)
{
	if (checkValid && !isKeyCodeValid(key))
	{
		bbe::Crash(bbe::Error::NoSuchKeycode);
	}

	return m_pkeysThisFrame[(int)key].down;
}

bool bbe::Keyboard::isKeyUp(bbe::Key key, bool checkValid)
{
	if (checkValid && !isKeyCodeValid(key))
	{
		bbe::Crash(bbe::Error::NoSuchKeycode);
	}

	return !m_pkeysThisFrame[(int)key].down;
}

bool bbe::Keyboard::isKeyPressed(bbe::Key key, bool checkValid)
{
	if (checkValid && !isKeyCodeValid(key))
	{
		bbe::Crash(bbe::Error::NoSuchKeycode);
	}

	return m_pkeysThisFrame[(int)key].pressed;
}

bool bbe::Keyboard::isKeyTyped(bbe::Key key, bool checkValid)
{
	if (checkValid && !isKeyCodeValid(key))
	{
		bbe::Crash(bbe::Error::NoSuchKeycode);
	}

	return m_pkeysThisFrame[(int)key].typed;
}
